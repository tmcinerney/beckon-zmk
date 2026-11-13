#!/usr/bin/env python3
"""Release-boundary regression tests; no GitHub or device writes."""
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location("candidate", ROOT / "beckon/scripts/candidate.py")
candidate = importlib.util.module_from_spec(spec)
spec.loader.exec_module(candidate)


class CandidateTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.directory = Path(self.temp.name)
        self.commit = "a" * 40
        (self.directory / "source.json").write_text(json.dumps({"commit": self.commit}))
        firmware = bytes.fromhex("5546320a57515d9e") + bytes(504)
        (self.directory / "glove80.uf2").write_bytes(firmware)
        (self.directory / "build.log").write_text("checks passed\n")
        (self.directory / "west-frozen.yml").write_text("frozen dependencies\n")

    def prepare(self):
        return candidate.prepare(self.directory, "v0.2.0", "v0.2.0-rc.6")

    def test_records_built_source_not_publish_checkout(self):
        metadata = self.prepare()
        self.assertEqual(metadata["commit"], self.commit)
        self.assertEqual((self.directory / "beckon.uf2").read_bytes(), (self.directory / "glove80.uf2").read_bytes())
        result = subprocess.run(["shasum", "-a", "256", "-c", "SHA256SUMS"], cwd=self.directory, capture_output=True)
        self.assertEqual(result.returncode, 0)

    def test_rejects_mismatched_or_injected_tags(self):
        for release, tag in [("v0.2.0", "v0.3.0-rc.1"), ("v0.2.0", "v0.2.0-rc.1 extra"), ("v0.2.0-x", "v0.2.0-x-rc.1")]:
            with self.subTest(tag=tag), self.assertRaises(ValueError):
                candidate.prepare(self.directory, release, tag)
        self.assertFalse((self.directory / "candidate.json").exists())

    def test_rejects_missing_build_or_invalid_identity(self):
        (self.directory / "source.json").write_text('{"commit":"beckon/main"}')
        with self.assertRaises(ValueError):
            self.prepare()
        (self.directory / "source.json").write_text(json.dumps({"commit": self.commit}))
        (self.directory / "build.log").write_text("")
        with self.assertRaises(ValueError):
            self.prepare()

    def test_rejects_invalid_firmware(self):
        (self.directory / "glove80.uf2").write_bytes(b"failed build")
        with self.assertRaises(ValueError):
            self.prepare()

    def run_publish(self, mode):
        self.prepare()
        mock = self.directory / "bin"
        mock.mkdir()
        (mock / "gh").write_text(r"""#!/usr/bin/env python3
import json, os, sys
from pathlib import Path
args = sys.argv[1:]
with open(os.environ['GH_TRACE'], 'a') as trace:
    trace.write(json.dumps(args) + '\n')
mode = os.environ['GH_MODE']
if args[:2] == ['release', 'view']:
    sys.exit(0 if mode == 'wrong-source' else 1)
if args[:2] == ['release', 'create']:
    sys.exit(1 if mode == 'create-fails' else 0)
if args[:1] == ['api'] and '/commits/' in args[1] and not args[1].endswith('/comments'):
    print('b' * 40)
""")
        (mock / "gh").chmod(0o755)
        env = dict(os.environ, PATH=str(mock) + os.pathsep + os.environ["PATH"], GITHUB_REPOSITORY="test/firmware", GH_MODE=mode, GH_TRACE=str(self.directory / "trace"))
        result = subprocess.run(["bash", str(ROOT / "beckon/scripts/publish-candidate.sh"), str(self.directory)], env=env, capture_output=True, text=True)
        self.assertTrue((self.directory / "trace").exists(), result.stderr)
        calls = [json.loads(line) for line in (self.directory / "trace").read_text().splitlines()]
        return result, calls

    def test_publish_failure_does_not_notify(self):
        result, calls = self.run_publish("create-fails")
        self.assertNotEqual(result.returncode, 0)
        self.assertTrue(any(call[:2] == ["release", "create"] for call in calls))
        self.assertFalse(any("POST" in call for call in calls))

    def test_existing_tag_with_wrong_source_cannot_succeed(self):
        result, calls = self.run_publish("wrong-source")
        self.assertNotEqual(result.returncode, 0)
        self.assertFalse(any(call[:2] == ["release", "create"] or "POST" in call for call in calls))

    def test_successful_publish_notifies_after_release(self):
        result, calls = self.run_publish("success")
        self.assertEqual(result.returncode, 0, result.stderr)
        create = next(i for i, call in enumerate(calls) if call[:2] == ["release", "create"])
        notify = next(i for i, call in enumerate(calls) if "POST" in call)
        self.assertLess(create, notify)


if __name__ == "__main__":
    unittest.main()
