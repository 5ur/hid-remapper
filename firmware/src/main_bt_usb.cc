// USB device glue for the remapper_bt target.
//
// This is kept in its own translation unit because BTstack (included by
// main_bt.cc) and TinyUSB both define hid_report_type_t / HID_REPORT_TYPE_*
// and cannot be included together.

#include <tusb.h>

#include <bsp/board_api.h>

#include <pico/platform.h>

#include "globals.h"
#include "our_descriptor.h"
#include "remapper.h"
#include "tick.h"

static void __no_inline_not_in_flash_func(sof_handler)(uint32_t frame_count) {
    sof_callback();
}

void bt_usb_init() {
    board_init();
    tusb_init();
    tud_sof_isr_set(sof_handler);
}

void bt_usb_task() {
    tud_task();
}

bool bt_usb_hid_ready(uint8_t itf) {
    return tud_hid_n_ready(itf);
}

bool bt_usb_suspended() {
    return tud_suspended();
}

bool bt_usb_do_send_report(uint8_t interface, const uint8_t* report_with_id, uint8_t len) {
    if (tud_suspended() &&
        (our_descriptor->should_cause_wakeup != nullptr) &&
        our_descriptor->should_cause_wakeup(report_with_id[0], report_with_id + 1, len - 1)) {
        tud_remote_wakeup();
    } else {
        tud_hid_n_report(interface, report_with_id[0], report_with_id + 1, len - 1);
    }
    return true;
}
