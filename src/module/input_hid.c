#include <stdio.h>
#include <stdint.h>
#include "esm.h"
#include "vrms_runtime.h"
#include "hid_monitor.h"

int device_add_callback(hid_monitor_device_t* device) {
    if (device->vendor_id == 0x413c && device->product_id == 0x2003) {
        fprintf(stderr, "add callback for device: %s\n", device->devnode);
        return 1;
    }
    fprintf(stderr, "%04x %04x\n", device->vendor_id, device->product_id);
    return 1;
}

int device_rem_callback(hid_monitor_device_t* device) {
    fprintf(stderr, "remove callback for device: %s\n", device->devnode);
    return 1;
}

void report_callback(hid_monitor_device_t* device, void* data, uint32_t data_length) {
    fprintf(stderr, "report callback for %s\n", device->devnode);

/*

void hid_monitor_bitfield8_callback(uint8_t bitfield8) {
    //(((bitfield8) >> (bit_no)) & 1)
}

    if (hid_device_nr_reports(device->hid_device) > 1) {
        report_id = (uint32_t)buf[0];
        report = hid_device_get_report_by_id(device->hid_device, report_id);
        memmove(buf, buf + 1, res);
        res--;
    }
    else {
        report_id = 0;
        report = &device->hid_device->reports[0];
    }

    if (NULL == report) {
        fprintf(stderr, "Unable to find report id: %d\n", report_id);
        return;
    }

    res_expected = report->byte_size;
    if (res_expected != res) {
        fprintf(stderr, "Read %d bytes but expected %d bytes!!\n", res, res_expected);
    }
*/
/*
device:
  report [id: 0]:
    report item: [size:1, count:3]
      [DAT|ARR|ABS|NOW|LIN|PRE|NUL|VOL]
      [ 1 | 0 | 1 | 1 | 1 | 1 | 1 | 1 ]
      report item usage: Button 1
      report item usage: Button 2
      report item usage: Button 3
    report item: [size:5, count:1]
      [DAT|ARR|ABS|NOW|LIN|PRE|NUL|VOL]
      [ 0 | 0 | 1 | 1 | 1 | 1 | 1 | 1 ]
    report item: [size:8, count:3]
      [DAT|ARR|ABS|NOW|LIN|PRE|NUL|VOL]
      [ 1 | 0 | 0 | 1 | 1 | 1 | 1 | 1 ]
      report item usage: X
      report item usage: Y
      report item usage: Wheel
*/
/*
    fprintf(stderr, "BEGIN REPORT: %d\n", report_id);
    for (idx = 0; idx < report->nr_report_items; idx++) {
        report_item = report->report_items[idx];
        if (!report_item.is_data) {
            continue;
        }

        bit_offset = report_item.bit_offset;
        byte_offset = bit_offset / 8;
        byte_bit_offset = bit_offset % 8;

        if (report_item.is_array) {
            if (report_item.is_absolute) {
                fprintf(stderr, "    report item: %d ARRAY BYTE\n", idx);
            }
        }
        else {
            if (report_item.is_absolute) {
                if (1 == report_item.report_size) {
                    bitfield8 = 0;
                    bitfield8 = buf[byte_offset] << byte_bit_offset;
                    hid_monitor_bitfield8_callback(bitfield8);
                }
                else {
                    fprintf(stderr, "unable to handle bitfield larger than 8 bits\n");
                }
            }
            else {
                fprintf(stderr, "logical_minimum: %d, logical_maximum: %d\n", (int8_t)report_item.logical_minimum, (int8_t)report_item.logical_maximum);
                if ((int8_t)report_item.logical_minimum < 0) {
                    for (f = 0; f < report_item.report_count; f++) {
                        signed8 = 0;
                        signed8 = buf[byte_offset] << byte_bit_offset;
                        fprintf(stderr, "signed value at index %d: %d\n", f, signed8);
                    }
                }
                else {
                    for (f = 0; f < report_item.report_count; f++) {
                        unsigned8 = 0;
                        unsigned8 = buf[byte_offset] << byte_bit_offset;
                        fprintf(stderr, "unsigned value at index %d: %d\n", f, unsigned8);
                    }
                }
            }
        }
    }
    fprintf(stderr, "END\n");
*/
}

void* run_module(void* data) {
    hid_monitor_t* monitor;

    monitor = hid_monitor_create();

    hid_monitor_set_device_add_callback(monitor, device_add_callback);
    hid_monitor_set_device_rem_callback(monitor, device_rem_callback);
    hid_monitor_set_report_callback(monitor, report_callback);

    hid_monitor_init(monitor);
    hid_monitor_run(monitor);

    return NULL;       
}
