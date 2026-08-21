if(CONFIG_BECKON_STATUS_TRANSPORT OR CONFIG_BECKON_STATUS_LED_PROOF)
  zephyr_library()
  zephyr_library_sources(src/events.c)
  zephyr_library_sources_ifdef(CONFIG_BECKON_STATUS_TRANSPORT src/protocol.c src/status_transport.c)
  zephyr_library_sources_ifdef(CONFIG_BECKON_STATUS_LED_PROOF src/status_led.c src/status_led_color.c src/status_split_sync.c)
  zephyr_library_include_directories(include)
  # The peripheral build does not include Raw HID, so it does not inherit
  # ZMK's app headers through that module as the central build does.
  zephyr_library_include_directories(${APPLICATION_SOURCE_DIR}/include)
endif()
