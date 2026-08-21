/*
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <zmk/split/transport/types.h>

/* Called by the split peripheral core after an authenticated status write. */
int zmk_split_peripheral_handle_beckon_status(
    const struct zmk_split_transport_beckon_status *status);
