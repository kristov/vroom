#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hid_device.h"

#define DEBUG 0
#define debug_print(fmt, ...) do { if (DEBUG) fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)

char* hid_get_usage_text(uint32_t usagePage, uint32_t usage);

#define HID_RD_USAGE_PAGE 0x04
#define HID_RD_USAGE 0x08
#define HID_RD_LOGICAL_MINIMUM 0x14
#define HID_RD_USAGE_MINIMUM 0x18
#define HID_RD_LOGICAL_MAXIMUM 0x24
#define HID_RD_USAGE_MAXIMUM 0x28
#define HID_RD_REPORT_SIZE 0x74
#define HID_RD_INPUT 0x80
#define HID_RD_REPORT_ID 0x84
#define HID_RD_OUTPUT 0x90
#define HID_RD_REPORT_COUNT 0x94
#define HID_RD_COLLECTION_START 0xa0
#define HID_RD_FEATURE 0xb0
#define HID_RD_COLLECTION_END 0xc0

#define HID_RD_PHYSICAL_MINIMUM 0x00;
#define HID_RD_PHYSICAL_MAXIMUM 0x00;

typedef struct hid_context_array hid_context_array_t;
struct hid_context_array {
    void* data;
    hid_context_array_t* next;
};

typedef struct hid_report_context hid_report_context_t;
struct hid_report_context {
    uint32_t usage_page;
    uint32_t usage;
    uint32_t usage_minimum;
    uint32_t usage_maximum;
    uint32_t report_id;
    uint32_t report_count;
    uint32_t report_size;
    uint32_t logical_maximum;
    uint32_t logical_minimum;
    uint32_t input_bit_offset;
    uint32_t input_bit_size;
    uint32_t output_bit_offset;
    uint32_t output_bit_size;
    uint32_t feature_bit_offset;
    uint32_t feature_bit_size;
    uint32_t nr_usages;
    hid_context_array_t* usages;
    uint32_t nr_report_items;
    hid_context_array_t* report_items;
    uint32_t nr_reports;
    hid_context_array_t* reports;
    hid_device_t* device;
};

void* safe_malloc(size_t n, char* file, unsigned long line) {
    void* p = malloc(n);
    if (!p) {
        fprintf(stderr, "[%s:%lu]Out of memory(%zu bytes)\n", file, line, n);
        exit(EXIT_FAILURE);
    }
    return p;
}
#define SAFEMALLOC(n) safe_malloc(n, __FILE__, __LINE__)

hid_context_array_t* hid_context_array_new(void* data) {
    hid_context_array_t* array = SAFEMALLOC(sizeof(hid_context_array_t));
    array->data = data;
    array->next = NULL;
    return array;
}

void hid_context_array_push(hid_context_array_t* root, hid_context_array_t* item) {
    while (NULL != root->next) {
        root = root->next;
    }
    root->next = item;
}

uint32_t hid_context_array_count(hid_context_array_t* root) {
    uint32_t count = 0;

    while (NULL != root) {
        count++;
        root = root->next;
    }

    return count;
}

void hid_context_array_destroy(hid_context_array_t* root) {
    hid_context_array_t* tmp;
    while (NULL != root) {
        tmp = root;
        root = root->next;
        free(tmp);
    }
}

hid_input_report_item_usage_t* hid_input_new_report_item_usage() {
    hid_input_report_item_usage_t* report_item_usage = NULL;

    report_item_usage = SAFEMALLOC(sizeof(hid_input_report_item_usage_t));
    memset(report_item_usage, 0, sizeof(hid_input_report_item_usage_t));

    return report_item_usage;
}

hid_input_report_item_t* hid_input_new_report_item() {
    hid_input_report_item_t* report_item = NULL;

    report_item = SAFEMALLOC(sizeof(hid_input_report_item_t));
    memset(report_item, 0, sizeof(hid_input_report_item_t));

    return report_item;
}

void hid_input_destroy_report_item(hid_input_report_item_t* report_item) {
    free(report_item->usages);
}

hid_input_report_t* hid_input_new_report(uint32_t report_id) {
    hid_input_report_t* report = NULL;

    report = SAFEMALLOC(sizeof(hid_input_report_t));
    memset(report, 0, sizeof(hid_input_report_t));
    report->report_id = report_id;

    return report;
}

void hid_input_destroy_report(hid_input_report_t* report) {
    uint32_t idx = 0;

    for (idx = 0; idx < report->nr_report_items; idx++) {
        hid_input_destroy_report_item(&report->report_items[idx]);
    }
    free(report->report_items);
    if (NULL != report->description) {
        free(report->description);
    }
}

hid_device_t* hid_input_new_device() {
    hid_device_t* device = NULL;

    device = SAFEMALLOC(sizeof(hid_device_t));
    memset(device, 0, sizeof(hid_device_t));

    return device;
}

void hid_device_destroy(hid_device_t* device) {
    uint32_t idx = 0;

    for (idx = 0; idx < device->nr_reports; idx++) {
        hid_input_destroy_report(&device->reports[idx]);
    }
    free(device->reports);
    if (NULL != device->description) {
        free(device->description);
    }
    free(device);
}

void hid_context_new_report_item_usage(hid_report_context_t* context) {
    hid_input_report_item_usage_t* report_item_usage = NULL;
    hid_context_array_t* array_item;

    report_item_usage = hid_input_new_report_item_usage();
    report_item_usage->usage_page = context->usage_page;
    report_item_usage->usage = context->usage;

    array_item = hid_context_array_new((void*)report_item_usage);
    if (NULL == context->usages) {
        context->usages = array_item;
    }
    else {
        hid_context_array_push(context->usages, array_item);
    }
}

void hid_context_destroy_report_item_usages(hid_report_context_t* context) {
    hid_context_array_t* head;
    hid_input_report_item_usage_t* usage;

    head = context->usages;
    while (NULL != head) {
        usage = (hid_input_report_item_usage_t*)head->data;
        free(usage);
        head = head->next;
    }

    hid_context_array_destroy(context->usages);
    context->usages = NULL;
}

void hid_context_copy_report_item_usages(hid_report_context_t* context, hid_input_report_item_t* report_item) {
    hid_context_array_t* head;
    hid_input_report_item_usage_t* usage;
    uint32_t count = 0;

    report_item->nr_usages = hid_context_array_count(context->usages);
    report_item->usages = SAFEMALLOC(sizeof(hid_input_report_item_usage_t) * report_item->nr_usages);

    head = context->usages;
    while (NULL != head) {
        usage = (hid_input_report_item_usage_t*)head->data;
        memcpy(&report_item->usages[count], usage, sizeof(hid_input_report_item_usage_t));
        free(usage);
        count++;
        head = head->next;
    }

    hid_context_array_destroy(context->usages);
    context->usages = NULL;
}

void hid_context_new_report_item(hid_report_context_t* context, uint8_t input_flags) {
    hid_input_report_item_t* report_item = NULL;
    hid_context_array_t* array_item;

    //if (NULL == context->usages) {
    //    // Do not add a report item that has no usages
    //    return;
    //}

    report_item = hid_input_new_report_item();
    report_item->logical_maximum = context->logical_maximum;
    report_item->logical_minimum = context->logical_minimum;
    report_item->report_count = context->report_count;
    report_item->report_size = context->report_size;
    report_item->bit_offset = context->input_bit_offset;
    report_item->bit_size = context->input_bit_size;
    report_item->is_data = (~input_flags & 0x01);
    report_item->is_array = (~(input_flags >> 1) & 0x01);
    report_item->is_absolute = (~(input_flags >> 2) & 0x01);
    report_item->is_nowrap = (~(input_flags >> 3) & 0x01);
    report_item->is_linear = (~(input_flags >> 4) & 0x01);
    report_item->is_preferredstate = (~(input_flags >> 5) & 0x01);
    report_item->is_nonullstate = (~(input_flags >> 6) & 0x01);
    report_item->is_nonvolatile = (~(input_flags >> 7) & 0x01);
    hid_context_copy_report_item_usages(context, report_item);

    array_item = hid_context_array_new((void*)report_item);
    if (NULL == context->report_items) {
        context->report_items = array_item;
    }
    else {
        hid_context_array_push(context->report_items, array_item);
    }
}

void hid_context_copy_report_items(hid_report_context_t* context, hid_input_report_t* report) {
    hid_context_array_t* head;
    hid_input_report_item_t* report_item;
    uint32_t count = 0;
    uint32_t bit_count = 0;

    report->nr_report_items = hid_context_array_count(context->report_items);
    report->report_items = SAFEMALLOC(sizeof(hid_input_report_item_t) * report->nr_report_items);

    head = context->report_items;
    while (NULL != head) {
        report_item = (hid_input_report_item_t*)head->data;
        bit_count += report_item->bit_size;
        memcpy(&report->report_items[count], report_item, sizeof(hid_input_report_item_t));
        free(report_item);
        count++;
        head = head->next;
    }
    report->byte_size = bit_count / 8;

    hid_context_array_destroy(context->report_items);
    context->report_items = NULL;
}

void hid_context_new_report(hid_report_context_t* context, uint32_t report_id) {
    hid_input_report_t* report = NULL;
    hid_context_array_t* array_item;

    if (NULL == context->report_items) {
        // Do not add an empty report with no report items
        return;
    }

    report = hid_input_new_report(report_id);
    hid_context_copy_report_items(context, report);

    array_item = hid_context_array_new((void*)report);
    if (NULL == context->reports) {
        context->reports = array_item;
    }
    else {
        hid_context_array_push(context->reports, array_item);
    }
}

void hid_context_copy_reports(hid_report_context_t* context, hid_device_t* device) {
    hid_context_array_t* head;
    hid_input_report_t* report;
    uint8_t populate_lookup;
    uint32_t count = 0;

    device->nr_reports = hid_context_array_count(context->reports);
    device->reports = SAFEMALLOC(sizeof(hid_input_report_t) * device->nr_reports);

    populate_lookup = 0;
    if (device->nr_reports > 1) {
        populate_lookup = 1;
    }

    head = context->reports;
    while (NULL != head) {
        report = (hid_input_report_t*)head->data;
        memcpy(&device->reports[count], report, sizeof(hid_input_report_t));
        if (populate_lookup) {
            device->report_id_lookup[report->report_id] = &device->reports[count];
        }
        free(report);
        count++;
        head = head->next;
    }

    hid_context_array_destroy(context->reports);
}

void hid_context_clear(hid_report_context_t* context) {
    context->usage_page = 0;
    context->usage = 0;
    context->usage_minimum = 0;
    context->usage_maximum = 0;
    context->report_id = 0;
    context->report_count = 0;
    context->report_size = 0;
    context->logical_maximum = 0;
    context->logical_minimum = 0;
    context->input_bit_offset = 0;
    context->input_bit_size = 0;
    context->output_bit_offset = 0;
    context->output_bit_size = 0;
    context->feature_bit_offset = 0;
    context->feature_bit_size = 0;
}

uint32_t hid_input_extract_data_from_buffer(uint8_t* buffer, uint32_t index, uint8_t bSize) {
    uint32_t value;
    switch (bSize) {
        case 0x01:
            value = buffer[index];
            break;
        case 0x02:
            value = ((buffer[index + 1] << 8) | (buffer[index] & 0xff));
            break;
        case 0x03:
            value = ((buffer[index + 2] << 16) | (buffer[index + 1] << 8) | (buffer[index] & 0xff));
            break;
        case 0x04:
            value = ((buffer[index + 3] << 24) | (buffer[index + 2] << 16) | (buffer[index + 1] << 8) | (buffer[index] & 0xff));
            break;
        default:
            debug_print("unknown size: %d\n", bSize);
            break;
    }
    return value;
}

uint32_t hid_device_read_report_item(uint8_t* buffer, uint32_t index, hid_report_context_t* context) {
    uint32_t idx = 0;
    uint8_t byte;
    uint8_t code;
    uint8_t bSize;
    char* description;

    uint32_t value;

    byte = buffer[index];
    bSize = (byte & 0x03);
    code = (byte & 0xfc);

    // Advance over the first byte code
    index++;

    switch (code) {
        case HID_RD_USAGE_PAGE:
            context->usage_page = hid_input_extract_data_from_buffer(buffer, index, bSize);
            debug_print("HID_RD_USAGE_PAGE:\n");
            index += bSize;
            break;
        case HID_RD_USAGE:
            context->usage = hid_input_extract_data_from_buffer(buffer, index, bSize);
            description = hid_get_usage_text(context->usage_page, context->usage);
            debug_print("HID_RD_USAGE: %04x [%s]\n", context->usage, description);
            free(description);
            hid_context_new_report_item_usage(context);
            index += bSize;
            break;
        case HID_RD_LOGICAL_MINIMUM:
            debug_print("HID_RD_LOGICAL_MINIMUM:\n");
            context->logical_minimum = hid_input_extract_data_from_buffer(buffer, index, bSize);
            index += bSize;
            break;
        case HID_RD_USAGE_MINIMUM:
            debug_print("HID_RD_USAGE_MINIMUM:\n");
            context->usage_minimum = hid_input_extract_data_from_buffer(buffer, index, bSize);
            index += bSize;
            break;
        case HID_RD_LOGICAL_MAXIMUM:
            debug_print("HID_RD_LOGICAL_MAXIMUM:\n");
            context->logical_maximum = hid_input_extract_data_from_buffer(buffer, index, bSize);
            index += bSize;
            break;
        case HID_RD_USAGE_MAXIMUM:
            debug_print("HID_RD_USAGE_MAXIMUM:\n");
            context->usage_maximum = hid_input_extract_data_from_buffer(buffer, index, bSize);
            index += bSize;
            break;
        case HID_RD_REPORT_SIZE:
            debug_print("HID_RD_REPORT_SIZE:\n");
            context->report_size = hid_input_extract_data_from_buffer(buffer, index, bSize);
            index += bSize;
            break;
        case HID_RD_INPUT:
            context->input_bit_size = (context->report_size * context->report_count);
            debug_print("HID_RD_INPUT (%02x)\n", buffer[index]);
            debug_print("  input_bit_offset: %d\n", context->input_bit_offset);
            debug_print("    input_bit_size: %d\n", context->input_bit_size);

            if (context->usage_maximum) {
                // If there is a min and max, ignore any previous usages
                hid_context_destroy_report_item_usages(context);

                for (idx = context->usage_minimum; idx <= context->usage_maximum; idx++) {
                    context->usage = idx;
                    hid_context_new_report_item_usage(context);
                }
            }

            value = hid_input_extract_data_from_buffer(buffer, index, bSize);
            hid_context_new_report_item(context, value);

            context->usage_minimum = 0;
            context->usage_maximum = 0;

            context->input_bit_offset += context->input_bit_size;
            index += bSize;
            break;
        case HID_RD_REPORT_ID:
            debug_print("HID_RD_REPORT_ID:\n");
            context->report_id = hid_input_extract_data_from_buffer(buffer, index, bSize);
            index += bSize;
            break;
        case HID_RD_OUTPUT:
/*
            context->output_bit_size = (context->report_size * context->report_count);
            debug_print("HID_RD_OUTPUT (%02x)\n", buffer[index]);
            debug_print("  output_bit_offset: %d\n", context->output_bit_offset);
            debug_print("    output_bit_size: %d\n", context->output_bit_size);

            if (context->usage_maximum) {
                // If there is a min and max, ignore any previous usages
                hid_context_destroy_report_item_usages(context);

                for (idx = context->usage_minimum; idx <= context->usage_maximum; idx++) {
                    context->usage = idx;
                    hid_context_new_report_item_usage(context);
                }
            }

            value = hid_input_extract_data_from_buffer(buffer, index, bSize);
            hid_context_new_output_item(context, value);

            context->usage_minimum = 0;
            context->usage_maximum = 0;

            context->output_bit_offset += context->output_bit_size;
*/
            index += bSize;
            break;
        case HID_RD_REPORT_COUNT:
            debug_print("HID_RD_REPORT_COUNT:\n");
            context->report_count = hid_input_extract_data_from_buffer(buffer, index, bSize);
            index += bSize;
            break;
        case HID_RD_COLLECTION_START:
            debug_print("HID_RD_COLLECTION_START: (add usages to reports/devices)\n");
            index += bSize;
            break;
        case HID_RD_FEATURE:
/*
            context->feature_bit_size = (context->report_size * context->report_count);
            debug_print("HID_RD_FEATURE (%02x)\n", buffer[index]);
            debug_print("  feature_bit_offset: %d\n", context->feature_bit_offset);
            debug_print("    feature_bit_size: %d\n", context->feature_bit_size);

            if (context->usage_maximum) {
                // If there is a min and max, ignore any previous usages
                hid_context_destroy_report_item_usages(context);

                for (idx = context->usage_minimum; idx <= context->usage_maximum; idx++) {
                    context->usage = idx;
                    hid_context_new_report_item_usage(context);
                }
            }

            value = hid_input_extract_data_from_buffer(buffer, index, bSize);
            hid_context_new_feature_item(context, value);

            context->usage_minimum = 0;
            context->usage_maximum = 0;

            context->feature_bit_offset += context->feature_bit_size;
*/
            index += bSize;
            break;
        case HID_RD_COLLECTION_END:
            debug_print("HID_RD_COLLECTION_END:\n");
            hid_context_new_report(context, context->report_id);
            hid_context_clear(context);
            index += bSize;
            break;
        default:
            if (bSize > 0) {
                value = hid_input_extract_data_from_buffer(buffer, index, bSize);
                debug_print("UNKNOWN CODE: %02x --> %032x\n", byte, value);
            }
            else {
                debug_print("UNKNOWN CODE: %02x --> [zero size]\n", byte);
            }
            index += bSize;
            break;
    }
    return index;
}

hid_input_report_t* hid_device_get_report_by_id(hid_device_t* device, uint32_t report_id) {
    if (report_id <= HID_DEVICE_MAX_REPORT_ID) {
        if (NULL != device->report_id_lookup[report_id]) {
            return device->report_id_lookup[report_id];
        }
    }
    return NULL;
}

uint32_t hid_device_nr_reports(hid_device_t* device) {
    if (NULL == device) {
        return 0;
    }
    return device->nr_reports;
}

void hid_device_dump(hid_device_t* device) {
    hid_input_report_t report;
    hid_input_report_item_t report_item;
    hid_input_report_item_usage_t report_item_usage;
    uint32_t r = 0;
    uint32_t ri = 0;
    uint32_t riu = 0;
    char* description;

    fprintf(stderr, "device:\n");
    for (r = 0; r < device->nr_reports; r++) {
        report = device->reports[r];
        fprintf(stderr, "  report [id: %d]:\n", report.report_id);
        for (ri = 0; ri < report.nr_report_items; ri++) {
            report_item = report.report_items[ri];
            fprintf(stderr, "    report item: [size:%d, count:%d]\n", report_item.report_size, report_item.report_count);
            fprintf(stderr, "      [DAT|ARR|ABS|NOW|LIN|PRE|NUL|VOL]\n");
            fprintf(stderr, "      [ %d | %d | %d | %d | %d | %d | %d | %d ]\n", report_item.is_data, report_item.is_array, report_item.is_absolute, report_item.is_nowrap, report_item.is_linear, report_item.is_preferredstate, report_item.is_nonullstate, report_item.is_nonvolatile);
            for (riu = 0; riu < report_item.nr_usages; riu++) {
                report_item_usage = report_item.usages[riu];
                description = hid_get_usage_text(report_item_usage.usage_page, report_item_usage.usage);
                fprintf(stderr, "      report item usage: %s\n", description);
                free(description);
            }
        }
    }
    fprintf(stderr, "\n");
}

hid_device_t* hid_device_report_descriptor(uint8_t* buffer, uint32_t length) {
    uint32_t index;
    hid_report_context_t context;
    hid_device_t* device;

    memset(&context, 0, sizeof(hid_report_context_t));

    device = hid_input_new_device();

    index = 0;
    while (index < length) {
        index = hid_device_read_report_item(buffer, index, &context);
    }

    hid_context_copy_reports(&context, device);

    //hid_dump_device(device);
    //hid_device_destroy(device);

    return device;
}

/*
 * Copied from https://github.com/smokris/usb-hid-usage with minor modifications:
 *
 * - Changed function getHidUsageText() to hid_get_usage_text()
 * - Removed main() function as it will be included in library
*/

/**
 * Returns a string containing a verbal description of the specified USB HID usage.
 *
 * The caller is responsible for freeing the returned string.
 *
 * Based on "HID Usage Tables 10/28/2004 Version 1.12" (with a few spelling errors corrected),
 * plus changes from Review Requests 28 through 52.  Exceptions:
 *
 *    - Implementation of RR39 is incomplete.
 *    - The conflicting assignments between RR33 (status "Review") and RR47 (status "Approved") are noted below; I went with the latter.
 *    - Includes usage 0x07:0xffffffff (the 8-bit USB HID keycodes of up to 8 simultanouely-pressed keys).
 *    - Includes usage pages 0x84 and 0x85 from "Universal Serial Bus Usage Tables for HID Power Devices Release 1.0 November 1, 1997".
 *    - Includes usage page 0xff (Fn key on Apple Keyboards).
 *
 * Copyright (C) 2015 Steve Mokris.  Use and distribution permitted under the terms of the Apache License Version 2.0.
 */
char* hid_get_usage_text(uint32_t usagePage, uint32_t usage) {
	char *text = 0;

	if (usagePage == 0x01)	// Generic Desktop
	{
		char *lookup[] = {
			0, "Pointer", "Mouse", 0, "Joystick", "Game Pad", "Keyboard", "Keypad",
			"Multi-axis Controller", "Tablet PC System Controls", "Water Cooling Device", "Computer Chassis Device", "Wireless Radio Controls", "Portable Device Control", 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"X", "Y", "Z", "Rx", "Ry", "Rz", "Slider", "Dial",
			"Wheel", "Hat switch", "Counted Buffer", "Byte Count", "Motion Wakeup", "Start", "Select", 0,
			"Vx", "Vy", "Vz", "Vbrx", "Vbry", "Vbrz", "Vno", "Feature Notification",
			"Resolution Multiplier", 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"System Control", "System Power Down", "System Sleep", "System Wake Up", "System Context Menu", "System Main Menu", "System App Menu", "System Menu Help",
			"System Menu Exit", "System Menu Select", "System Menu Right", "System Menu Left", "System Menu Up", "System Menu Down", "System Cold Restart", "System Warm Restart",
			"D-pad Up", "D-pad Down", "D-pad Right", "D-pad Left", 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"System Dock", "System Undock", "System Setup", "System Break", "System Debugger Break", "Application Break", "Application Debugger Break", "System Speaker Mute",
			"System Hibernate", 0, 0, 0, 0, 0, 0, 0,
			"System Display Invert", "System Display Internal", "System Display External", "System Display Both", "System Display Dual", "System Display Toggle Int/Ext", "System Display Swap Primary/Secondary", "System Display LCD Autoscale",
			0, 0, 0, 0, 0, 0, 0, 0,
			"Sensor Zone", "RPM", "Coolant Level", "Coolant Critical Level", "Coolant Pump", "Chassis Enclosure", "Wireless Radio Button", "Wireless Radio LED",
			"Wireless Radio Slider Switch", "System Display Rotation Lock Button", "System Display Rotation Lock Slider Switch", "Control Enable", 0, 0, 0, 0,
		};
		text = usage < sizeof(lookup)/sizeof(unsigned char *) ? (lookup[usage] ? strdup(lookup[usage]) : 0) : 0;
	}

	else if (usagePage == 0x02)	// Simulation Controls
	{
		char *lookup[] = {
			0, "Flight Simulation Device", "Automobile Simulation Device", "Tank Simulation Device", "Spaceship Simulation Device", "Submarine Simulation Device", "Sailing Simulation Device", "Motorcycle Simulation Device",
			"Sports Simulation Device", "Airplane Simulation Device", "Helicopter Simulation Device", "Magic Carpet Simulation Device", "Bicycle Simulation Device", 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"Flight Control Stick", "Flight Stick", "Cyclic Control", "Cyclic Trim", "Flight Yoke", "Track Control", 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"Aileron", "Aileron Trim", "Anti-Torque Control", "Autopilot Enable", "Chaff Release", "Collective Control", "Dive Brake", "Electronic Countermeasures",
			"Elevator", "Elevator Trim", "Rudder", "Throttle", "Flight Communications", "Flare Release", "Landing Gear", "Toe Brake",
			"Trigger", "Weapons Arm", "Weapons Select", "Wing Flaps", "Accelerator", "Brake", "Clutch", "Shifter",
			"Steering", "Turret Direction", "Barrel Elevation", "Dive Plane", "Ballast", "Bicycle Crank", "Handle Bars", "Front Brake",
			"Rear Brake", 0, 0, 0, 0, 0, 0, 0,
		};
		text = usage < sizeof(lookup)/sizeof(unsigned char *) ? (lookup[usage] ? strdup(lookup[usage]) : 0) : 0;
	}

	else if (usagePage == 0x03)	// VR Controls
	{
		char *lookup[] = {
			0, "Belt", "Body Suit", "Flexor", "Glove", "Head Tracker", "Head Mounted Display", "Hand Tracker",
			"Oculometer", "Vest", "Animatronic Device", 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"Stereo Enable", "Display Enable", 0, 0, 0, 0, 0, 0,
		};
		text = usage < sizeof(lookup)/sizeof(unsigned char *) ? (lookup[usage] ? strdup(lookup[usage]) : 0) : 0;
	}

	else if (usagePage == 0x04)	// Sport Controls
	{
		char *lookup[] = {
			0, "Baseball Bat", "Golf Club", "Rowing Machine", "Treadmill", 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"Oar", "Slope", "Rate", "Stick Speed", "Stick Face Angle", "Stick Heel/Toe", "Stick Follow Through", "Stick Tempo",
			"Stick Type", "Stick Height", 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"Putter", "1 Iron", "2 Iron", "3 Iron", "4 Iron", "5 Iron", "6 Iron", "7 Iron",
			"8 Iron", "9 Iron", "10 Iron", "11 Iron", "Sand Wedge", "Loft Wedge", "Power Wedge", "1 Wood",
			"3 Wood", "5 Wood", "7 Wood", "9 Wood", 0, 0, 0, 0,
		};
		text = usage < sizeof(lookup)/sizeof(unsigned char *) ? (lookup[usage] ? strdup(lookup[usage]) : 0) : 0;
	}

	else if (usagePage == 0x05)	// Game Controls
	{
		char *lookup[] = {
			0, "3D Game Controller", "Pinball Device", "Gun Device", 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"Point of View", "Turn Right/Left", "Pitch Forward/Backward", "Roll Right/Left", "Move Right/Left", "Move Forward/Backward", "Move Up/Down", "Lean Right/Left",
			"Lean Forward/Backward", "Height of POV", "Flipper", "Secondary Flipper", "Bump", "New Game", "Shoot Ball", "Player",
			"Gun Bolt", "Gun Clip", "Gun Selector", "Gun Single Shot", "Gun Burst", "Gun Automatic", "Gun Safety", "Gamepad Fire/Jump",
			"Gamepad Trigger", 0, 0, 0, 0, 0, 0, 0,
		};
		text = usage < sizeof(lookup)/sizeof(unsigned char *) ? (lookup[usage] ? strdup(lookup[usage]) : 0) : 0;
	}

	else if (usagePage == 0x06)	// Generic Device Controls
	{
		char *lookup[] = {
			0, "Background Controls", 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"Battery Strength", "Wireless Channel", "Wireless ID", "Discover Wireless Control", "Security Code Character Entered", "Security Code Character Erased", "Security Code Cleared", "Sequence ID",
			"Sequence ID Reset", "RF Signal Strength", 0, 0, 0, 0, 0, 0,
		};
		text = usage < sizeof(lookup)/sizeof(unsigned char *) ? (lookup[usage] ? strdup(lookup[usage]) : 0) : 0;
	}

	else if (usagePage == 0x07)	// Keyboard/Keypad
	{
		char *lookup[] = {
			0, "Keyboard ErrorRollOver", "Keyboard POSTFail", "Keyboard ErrorUndefined", "Keyboard a and A", "Keyboard b and B", "Keyboard c and C", "Keyboard d and D",
			"Keyboard e and E", "Keyboard f and F", "Keyboard g and G", "Keyboard h and H", "Keyboard i and I", "Keyboard j and J", "Keyboard k and K", "Keyboard l and L",
			"Keyboard m and M", "Keyboard n and N", "Keyboard o and O", "Keyboard p and P", "Keyboard q and Q", "Keyboard r and R", "Keyboard s and S", "Keyboard t and T",
			"Keyboard u and U", "Keyboard v and V", "Keyboard w and W", "Keyboard x and X", "Keyboard y and Y", "Keyboard z and Z", "Keyboard 1 and !", "Keyboard 2 and @",
			"Keyboard 3 and #", "Keyboard 4 and $", "Keyboard 5 and %", "Keyboard 6 and ^", "Keyboard 7 and &", "Keyboard 8 and *", "Keyboard 9 and (", "Keyboard 0 and )",
			"Keyboard Return (ENTER)", "Keyboard ESCAPE", "Keyboard DELETE (Backspace)", "Keyboard Tab", "Keyboard Spacebar", "Keyboard - and _", "Keyboard = and +", "Keyboard [ and {",
			"Keyboard ] and }", "Keyboard \\ and |", "Keyboard Non-US # and ~", "Keyboard ; and :", "Keyboard ‘ and “", "Keyboard Grave Accent and Tilde", "Keyboard , and <", "Keyboard . and >",
			"Keyboard / and ?", "Keyboard Caps Lock", "Keyboard F1", "Keyboard F2", "Keyboard F3", "Keyboard F4", "Keyboard F5", "Keyboard F6",
			"Keyboard F7", "Keyboard F8", "Keyboard F9", "Keyboard F10", "Keyboard F11", "Keyboard F12", "Keyboard PrintScreen", "Keyboard Scroll Lock",
			"Keyboard Pause", "Keyboard Insert", "Keyboard Home", "Keyboard PageUp", "Keyboard Delete Forward", "Keyboard End", "Keyboard PageDown", "Keyboard RightArrow",
			"Keyboard LeftArrow", "Keyboard DownArrow", "Keyboard UpArrow", "Keypad Num Lock and Clear", "Keypad /", "Keypad *", "Keypad -", "Keypad +",
			"Keypad ENTER", "Keypad 1 and End", "Keypad 2 and Down Arrow", "Keypad 3 and PageDn", "Keypad 4 and Left Arrow", "Keypad 5", "Keypad 6 and Right Arrow", "Keypad 7 and Home",
			"Keypad 8 and Up Arrow", "Keypad 9 and PageUp", "Keypad 0 and Insert", "Keypad . and Delete", "Keyboard Non-US \\ and |", "Keyboard Application", "Keyboard Power", "Keypad =",
			"Keyboard F13", "Keyboard F14", "Keyboard F15", "Keyboard F16", "Keyboard F17", "Keyboard F18", "Keyboard F19", "Keyboard F20",
			"Keyboard F21", "Keyboard F22", "Keyboard F23", "Keyboard F24", "Keyboard Execute", "Keyboard Help", "Keyboard Menu", "Keyboard Select",
			"Keyboard Stop", "Keyboard Again", "Keyboard Undo", "Keyboard Cut", "Keyboard Copy", "Keyboard Paste", "Keyboard Find", "Keyboard Mute",
			"Keyboard Volume Up", "Keyboard Volume Down", "Keyboard Locking Caps Lock", "Keyboard Locking Num Lock", "Keyboard Locking Scroll Lock", "Keypad Comma", "Keypad Equal Sign", "Keyboard International1",
			"Keyboard International2", "Keyboard International3", "Keyboard International4", "Keyboard International5", "Keyboard International6", "Keyboard International7", "Keyboard International8", "Keyboard International9",
			"Keyboard LANG1", "Keyboard LANG2", "Keyboard LANG3", "Keyboard LANG4", "Keyboard LANG5", "Keyboard LANG6", "Keyboard LANG7", "Keyboard LANG8",
			"Keyboard LANG9", "Keyboard Alternate Erase", "Keyboard SysReq/Attention", "Keyboard Cancel", "Keyboard Clear", "Keyboard Prior", "Keyboard Return", "Keyboard Separator",
			"Keyboard Out", "Keyboard Oper", "Keyboard Clear/Again", "Keyboard CrSel/Props", "Keyboard ExSel", 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"Keypad 00", "Keypad 000", "Thousands Separator", "Decimal Separator", "Currency Unit", "Currency Sub-unit", "Keypad (", "Keypad )",
			"Keypad {", "Keypad }", "Keypad Tab", "Keypad Backspace", "Keypad A", "Keypad B", "Keypad C", "Keypad D",
			"Keypad E", "Keypad F", "Keypad XOR", "Keypad ^", "Keypad %", "Keypad <", "Keypad >", "Keypad &",
			"Keypad &&", "Keypad |", "Keypad ||", "Keypad :", "Keypad #", "Keypad Space", "Keypad @", "Keypad !",
			"Keypad Memory Store", "Keypad Memory Recall", "Keypad Memory Clear", "Keypad Memory Add", "Keypad Memory Subtract", "Keypad Memory Multiply", "Keypad Memory Divide", "Keypad +/-",
			"Keypad Clear", "Keypad Clear Entry", "Keypad Binary", "Keypad Octal", "Keypad Decimal", "Keypad Hexadecimal", 0, 0,
			"Keyboard LeftControl", "Keyboard LeftShift", "Keyboard LeftAlt", "Keyboard Left GUI", "Keyboard RightControl", "Keyboard RightShift", "Keyboard RightAlt", "Keyboard Right GUI",
		};
		text = usage < sizeof(lookup)/sizeof(unsigned char *) ? (lookup[usage] ? strdup(lookup[usage]) : 0) : 0;

		if (usage == 0xffffffff)
			text = strdup("Keycodes");
	}

	else if (usagePage == 0x08)	// LED
	{
		char *lookup[] = {
			0, "Num Lock", "Caps Lock", "Scroll Lock", "Compose", "Kana", "Power", "Shift",
			"Do Not Disturb", "Mute", "Tone Enable", "High Cut Filter", "Low Cut Filter", "Equalizer Enable", "Sound Field On", "Surround On",
			"Repeat", "Stereo", "Sampling Rate Detect", "Spinning", "CAV", "CLV", "Recording Format Detect", "Off-Hook",
			"Ring", "Message Waiting", "Data Mode", "Battery Operation", "Battery OK", "Battery Low", "Speaker", "Head Set",
			"Hold", "Microphone", "Coverage", "Night Mode", "Send Calls", "Call Pickup", "Conference", "Stand-by",
			"Camera On", "Camera Off", "On-Line", "Off-Line", "Busy", "Ready", "Paper-Out", "Paper-Jam",
			"Remote", "Forward", "Reverse", "Stop", "Rewind", "Fast Forward", "Play", "Pause",
			"Record", "Error", "Usage Selected Indicator", "Usage In Use Indicator", "Usage Multi Mode Indicator", "Indicator On", "Indicator Flash", "Indicator Slow Blink",
			"Indicator Fast Blink", "Indicator Off", "Flash On Time", "Slow Blink On Time", "Slow Blink Off Time", "Fast Blink On Time", "Fast Blink Off Time", "￼Usage Indicator Color",
			"Indicator Red", "Indicator Green", "Indicator Amber", "Generic Indicator", "System Suspend", "External Power Connected", /* "Indicator Blue", "Indicator Orange", */ "Player Indicator", "Player 1",	// Conflict between RR33 and RR47
			/* "Good Status", "Warning Status", "RGB LED", "Red LED Channel", "Greed LED Channel", "Blue LED Channel", "LED Intensity", 0, */ "Player 2", "Player 3", "Player 4",  "Player 5",  "Player 6",  "Player 7",  "Player 8", 0,	// Conflict between RR33 and RR47
		};
		text = usage < sizeof(lookup)/sizeof(unsigned char *) ? (lookup[usage] ? strdup(lookup[usage]) : 0) : 0;
	}

	else if (usagePage == 0x09)	// Button
	{
		if (usage > 0 && usage <= 0xffff)
			asprintf(&text, "Button %d", usage);
	}
	
	else if (usagePage == 0x0a)	// Ordinal
	{
		if (usage > 0 && usage <= 0xffff)
			asprintf(&text, "Instance %d", usage);
	}
	
	else if (usagePage == 0x0b)	// Telephony Device
	{
		char *lookup[] = {
			0, "Phone", "Answering Machine", "Message Controls", "Handset", "Headset", "Telephony Key Pad", "Programmable Button",
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"Hook Switch", "Flash", "Feature", "Hold", "Redial", "Transfer", "Drop", "Park",
			"Forward Calls", "Alternate Function", "Line", "Speaker Phone", "Conference", "Ring Enable", "Ring Select", "Phone Mute",
			"Caller ID", "Send", 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"Speed Dial", "Store Number", "Recall Number", "Phone Directory", 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"Voice Mail", "Screen Calls", "Do Not Disturb", "Message", "Answer On/Off", 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"Inside Dial Tone", "Outside Dial Tone", "Inside Ring Tone", "Outside Ring Tone", "Priority Ring Tone", "Inside Ringback", "Priority Ringback", "Line Busy Tone",
			"Reorder Tone", "Call Waiting Tone", "Confirmation Tone", "Confirmation Tone", "Tones Off", "Outside Ringback", "Ringer", 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"Phone Key 0", "Phone Key 1", "Phone Key 2", "Phone Key 3", "Phone Key 4", "Phone Key 5", "Phone Key 6", "Phone Key 7",
			"Phone Key 8", "Phone Key 9", "Phone Key Star", "Phone Key Pound", "Phone Key A", "Phone Key B", "Phone Key C", "Phone Key D",
			"Phone Call History Key", "Phone Caller ID Key", "Phone Settings Key", 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"Host Control", "Host Available", "Host Call Active", "Activate Handset Audio", "Ring Type", "Re-dialable Phone Number", 0, 0,
			"Stop Ring Tone", "PSTN Ring Tone", "Host Ring Tone", "Alert Sound Error", "Alert Sound Confirm", "Alert Sound Notification", "Silent Ring", 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"Email Message Waiting", "Voicemail Message Waiting", "Host Hold", 0, 0, 0, 0, 0,
			"Incoming Call History Count", "Outgoing Call History Count", "Incoming Call History", "Outgoing Call History", "Phone Locale", 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"Phone Time Second", "Phone Time Minute", "Phone Time Hour", "Phone Date Day", "Phone Date Month", "Phone Date Year", "Handset Nickname", "Address Book ID",
			"Call Duration", "Dual Mode Phone", 0, 0, 0, 0, 0, 0,
		};
		text = usage < sizeof(lookup)/sizeof(unsigned char *) ? (lookup[usage] ? strdup(lookup[usage]) : 0) : 0;
	}
	
	else if (usagePage == 0x0c)	// Consumer
	{
		char *lookup[] = {
			0, "Consumer Control", "Numeric Key Pad", "Programmable Buttons", "Microphone", "Headphone", "Graphic Equalizer", 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"+10", "+100", "AM/PM", 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"Power", "Reset", "Sleep", "Sleep After", "Sleep Mode", "Illumination", "Function Buttons", 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"Menu", "Menu Pick", "Menu Up", "Menu Down", "Menu Left", "Menu Right", "Menu Escape", "Menu Value Increase",
			"Menu Value Decrease", 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"Data On Screen", "Closed Caption", "Closed Caption Select", "VCR/TV", "Broadcast Mode", "Snapshot", "Still", "Picture-in-Picture Toggle",
			"Picture-in-Picture Swap", "Red Menu Button", "Green Menu Button", "Blue Menu Button", "Yellow Menu Button", "Aspect", "3D Mode Select", "Display Brightness Increment",
			"Display Brightness Decrement", "Display Brightness", "Display Backlight Toggle", "Display Set Brightness to Minimum", "Display Set Brightness to Maximum", "Display Set Auto Brightness", 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, "Assign Selection", "Mode Step", "Recall Last", "Enter Channel", "Order Movie", "Channel", 0,
			"Media Select Computer", "Media Select TV", "Media Select WWW", "Media Select DVD", "Media Select Telephone", "Media Select Program Guide", "Media Select Video Phone", "Media Select Games",
			"Media Select Messages", "Media Select CD", "Media Select VCR", "Media Select Tuner", "Quit", "Help", "Media Select Tape", "Media Select Cable",
			"Media Select Satellite", "Media Select Security", "Media Select Home", "Media Select Call", "Channel Increment", "Channel Decrement", "Media Select SAP", 0,
			"VCR Plus", "Once", "Daily", "Weekly", "Monthly", 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"Play", "Pause", "Record", "Fast Forward", "Rewind", "Scan Next Track", "Scan Previous Track", "Stop",
			"Eject", "Random Play", "Select Disc", "Enter Disc", "Repeat", "Tracking", "Track Normal", "Slow Tracking",
			"Frame Forward", "Frame Back", "Mark", "Clear Mark", "Repeat From Mark", "Return To Mark", "Search Mark Forward", "Search Mark Backwards",
			"Counter Reset", "Show Counter", "Tracking Increment", "Tracking Decrement", "Stop/Eject", "Play/Pause", "Play/Skip", "Voice Command",
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"Volume", "Balance", "Mute", "Bass", "Treble", "Bass Boost", "Surround Mode", "Loudness",
			"MPX", "Volume Increment", "Volume Decrement", 0, 0, 0, 0, 0,
			"Speed Select", "Playback Speed", "Standard Play", "Long Play", "Extended Play", "Slow", 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"Fan Enable", "Fan Speed", "Light Enable", "Light Illumination Level", "Climate Control Enable", "Room Temperature", "Security Enable", "Fire Alarm",
			"Police Alarm", "Proximity", "Motion", "Duress Alarm", "Holdup Alarm", "Medical Alarm", 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"Balance Right", "Balance Left", "Bass Increment", "Bass Decrement", "Treble Increment", "Treble Decrement", 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"Speaker System", "Channel Left", "Channel Right", "Channel Center", "Channel Front", "Channel Center Front", "Channel Side", "Channel Surround",
			"Channel Low Frequency Enhancement", "Channel Top", "Channel Unknown", 0, 0, 0, 0, 0,
			"Sub-channel", "Sub-channel Increment", "Sub-channel Decrement", "Alternate Audio Increment", "Alternate Audio Decrement", 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"Application Launch Buttons", "AL Launch Button Configuration Tool", "AL Programmable Button Configuration", "AL Consumer Control Configuration", "AL Word Processor", "AL Text Editor", "AL Spreadsheet", "AL Graphics Editor",
			"AL Presentation App", "AL Database App", "AL Email Reader", "AL Newsreader", "AL Voicemail", "AL Contacts/Address Book", "AL Calendar/Schedule", "AL Task/Project Manager",
			"AL Log/Journal/Timecard", "AL Checkbook/Finance", "AL Calculator", "AL A/V Capture/Playback", "AL Local Machine Browser", "AL LAN/WAN Browser", "AL Internet Browser", "AL Remote Networking/ISP Connect",
			"AL Network Conference", "AL Network Chat", "AL Telephony/Dialer", "AL Logon", "AL Logoff", "AL Logon/Logoff", "AL Terminal Lock/Screensaver", "AL Control Panel",
			"AL Command Line Processor/Run", "AL Process/Task Manager", "AL Select Task/Application", "AL Next Task/Application", "AL Previous Task/Application", "AL Preemptive Halt Task/Application", "AL Integrated Help Center", "AL Documents",
			"AL Thesaurus", "AL Dictionary", "AL Desktop", "AL Spell Check", "AL Grammar Check", "AL Wireless Status", "AL Keyboard Layout", "AL Virus Protection",
			"AL Encryption", "AL Screen Saver", "AL Alarms", "AL Clock", "AL File Browser", "AL Power Status", "AL Image Browser", "AL Audio Browser",
			"AL Movie Browser", "AL Digital Rights Manager", "AL Digital Wallet", 0, "AL Instant Messaging", "AL OEM Features/Tips/Tutorial Browser", "AL OEM Help", "AL Online Community",
			"AL Entertainment Content Browser", "AL Online Shopping Browser", "AL SmartCard Information/Help", "AL Market Monitor/Finance Browser", "AL Customized Corporate News Browser", "AL Online Activity Browser", "AL Research/Search Browser", "AL Audio Player",
			"AL Message Status", "AL Contact Sync", 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"Generic GUI Application Controls", "AC New", "AC Open", "AC Close", "AC Exit", "AC Maximize", "AC Minimize", "AC Save",
			"AC Print", "AC Properties", "AC Undo", "AC Copy", "AC Cut", "AC Paste", "AC Select All", "AC Find",
			"AC Find and Replace", "AC Search", "AC Go To", "AC Home", "AC Back", "AC Forward", "AC Stop", "AC Refresh",
			"AC Previous Link", "AC Next Link", "AC Bookmarks", "AC History", "AC Subscriptions", "AC Zoom In", "AC Zoom Out", "AC Zoom",
			"AC Full Screen View", "AC Normal View", "AC View Toggle", "AC Scroll Up", "AC Scroll Down", "AC Scroll", "AC Pan Left", "AC Pan Right",
			"AC Pan", "AC New Window", "AC Tile Horizontally", "AC Tile Vertically", "AC Format", "AC Edit", "AC Bold", "AC Italics",
			"AC Underline", "AC Strikethrough", "AC Subscript", "AC Superscript", "AC All Caps", "AC Rotate", "AC Resize", "AC Flip horizontal",
			"AC Flip Vertical", "AC Mirror Horizontal", "AC Mirror Vertical", "AC Font Select", "AC Font Color", "AC Font Size", "AC Justify Left", "AC Justify Center H",
			"AC Justify Right", "AC Justify Block H", "AC Justify Top", "AC Justify Center V", "AC Justify Bottom", "AC Justify Block V", "AC Indent Decrease", "AC Indent Increase",
			"AC Numbered List", "AC Restart Numbering", "AC Bulleted List", "AC Promote", "AC Demote", "AC Yes", "AC No", "AC Cancel",
			"AC Catalog", "AC Buy/Checkout", "AC Add to Cart", "AC Expand", "AC Expand All", "AC Collapse", "AC Collapse All", "AC Print Preview",
			"AC Paste Special", "AC Insert Mode", "AC Delete", "AC Lock", "AC Unlock", "AC Protect", "AC Unprotect", "AC Attach Comment",
			"AC Delete Comment", "AC View Comment", "AC Select Word", "AC Select Sentence", "AC Select Paragraph", "AC Select Column", "AC Select Row", "AC Select Table",
			"AC Select Object", "AC Redo/Repeat", "AC Sort", "AC Sort Ascending", "AC Sort Descending", "AC Filter", "AC Set Clock", "AC View Clock",
			"AC Select Time Zone", "AC Edit Time Zones", "AC Set Alarm", "AC Clear Alarm", "AC Snooze Alarm", "AC Reset Alarm", "AC Synchronize", "AC Send/Receive",
			"AC Send To", "AC Reply", "AC Reply All", "AC Forward Msg", "AC Send", "AC Attach File", "AC Upload", "AC Download (Save Target As)",
			"AC Set Borders", "AC Insert Row", "AC Insert Column", "AC Insert File", "AC Insert Picture", "AC Insert Object", "AC Insert Symbol", "AC Save and Close",
			"AC Rename", "AC Merge", "AC Split", "AC Distribute Horizontally", "AC Distribute Vertically", 0, 0, 0,
			"AC Soft Key Left", "AC Soft Key Right", 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"AC Idle Keep Alive", 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"Extended Keyboard Attributes Collection", "Keyboard Form Factor", "Keyboard Key Type", "Keyboard Physical Layout", "Vendor-Specific Keyboard Physical Layout", "Keyboard IETF Language Tag Index", "Implemented Keyboard Input Assist Controls", "Keyboard Input Assist Previous",
			"Keyboard Input Assist Next", "Keyboard Input Assist Previous Group", "Keyboard Input Assist Next Group", "Keyboard Input Assist Accept", "Keyboard Input Assist Cancel", 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"Contact Edited", "Contact Added", "Contact Record Active", "Contact Index", "Contact Nickname", "Contact First Name", "Contact Last Name", "Contact Full Name",
			"Contact Phone Number Personal", "Contact Phone Number Business", "Contact Phone Number Mobile", "Contact Phone Number Pager", "Contact Phone Number Fax", "Contact Phone Number Other", "Contact Email Personal", "Contact Email Business",
			"Contact Email Other", "Contact Email Main", "Contact Speed Dial Number", "Contact Status Flag", "Contact Misc.", 0, 0, 0,
		};
		text = usage < sizeof(lookup)/sizeof(unsigned char *) ? (lookup[usage] ? strdup(lookup[usage]) : 0) : 0;
	}
	
	else if (usagePage == 0x0d)	// Digitizers
	{
		char *lookup[] = {
			0, "Digitizer", "Pen", "Light Pen", "Touch Screen", "Touch Pad", "White Board", "Coordinate Measuring",
			"3D Digitizer", "Stereo Plotter", "Articulated Arm", "Armature", "Multiple Point Digitizer", "Free Space Wand", "Device configuration", 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"Stylus", "Puck", "Finger", "Device settings", 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"Tip Pressure", "Barrel Pressure", "In Range", "Touch", "Untouch", "Tap", "Quality", "Data Valid",
			"Transducer Index", "Tablet Function Keys", "Program Change Keys", "Battery Strength", "Invert", "X Tilt", "Y Tilt", "Azimuth",
			"Altitude", "Twist", "Tip Switch", "Secondary Tip Switch", "Barrel Switch", "Eraser", "Tablet Pick", "Touch Valid",
			"Width", "Height", 0, 0, 0, 0, 0, 0,
			0, "Contact identifier", "Device mode", "Device identifier", "Contact count", "Contact count maximum", 0, 0,
			0, 0, "Secondary Barrel Switch", "Transducer Serial Number", 0, 0, 0, 0,
		};
		text = usage < sizeof(lookup)/sizeof(unsigned char *) ? (lookup[usage] ? strdup(lookup[usage]) : 0) : 0;
	}

	else if (usagePage == 0x14)	// Alphanumeric Display / Auxiliary Display
	{
		char *lookup[] = {
			0, "Alphanumeric Display", "Bitmapped/Auxiliary Display", 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"Display Attributes Report", "ASCII Character Set", "Data Read Back", "Font Read Back", "Display Control Report", "Clear Display", "Display Enable", "Screen Saver Delay",
			"Screen Saver Enable", "Vertical Scroll", "Horizontal Scroll", "Character Report", "Display Data", "Display Status", "Stat Not Ready", "Stat Ready",
			"Err Not a loadable character", "Err Font data cannot be read", "Cursor Position Report", "Row", "Column", "Rows", "Columns", "Cursor Pixel Positioning",
			"Cursor Mode", "Cursor Enable", "Cursor Blink", "Font Report", "Font Data", "Character Width", "Character Height", "Character Spacing Horizontal",
			"Character Spacing Vertical", "Unicode Character Set", "Font 7-Segment", "7-Segment Direct Map", "Font 14-Segment", "14-Segment Direct Map", "Display Brightness", "Display Contrast",
			"Character Attribute", "Attribute Readback", "Attribute Data", "Char Attr Enhance", "Char Attr Underline", "Char Attr Blink", 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"Bitmap Size X", "Bitmap Size Y", "Max Blit Size", "Bit Depth Format", "Display Orientation", "Palette Report", "Palette Data Size", "Palette Data Offset",
			"Palette Data", "Blit Report", "Blit Rectangle X1", "Blit Rectangle Y1", "Blit Rectangle X2", "Blit Rectangle Y2", "Blit Data", "Soft Button",
			"Soft Button ID", "Soft Button Side", "Soft Button Offset 1", "Soft Button Offset 2", "Soft Button Report", 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, "Soft Keys", 0, 0, 0, 0, 0,
			0, 0, 0, 0, "Display Data Extensions", 0, 0, "Character Mapping",
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, "Unicode Equivalent", 0, "Character Page Mapping",
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, "Request Report",
		};
		text = usage < sizeof(lookup)/sizeof(unsigned char *) ? (lookup[usage] ? strdup(lookup[usage]) : 0) : 0;
	}
	
	else if (usagePage == 0x20)	// Sensor
	{
		char *lookup[] = {
			0, "Sensor", 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"Biometric", "Biometric: Human Presence", "Biometric: Human Proximity", "Biometric: Human Touch", 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"Electrical", "Electrical: Capacitance", "Electrical: Current", "Electrical: Power", "Electrical: Inductance", "Electrical: Resistance", "Electrical: Voltage", "Electrical: Potentiometer",
			"Electrical: Frequency", "Electrical: Period", 0, 0, 0, 0, 0, 0,
			"Environmental", "Environmental: Atmospheric Pressure", "Environmental: Humidity", "Environmental: Temperature", "Environmental: Wind Direction", "Environmental: Wind Speed", 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"Light", "Light: Ambient Light", "Light: Consumer Infrared", 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"Location", "Location: Broadcast", "Location: Dead Reckoning", "Location: GPS (Global Positioning System)", "Location: Lookup", "Location: Other", "Location: Static", "Location: Triangulation",
			0, 0, 0, 0, 0, 0, 0, 0,
			"Mechanical", "Mechanical: Boolean Switch", "Mechanical: Boolean Switch Array", "Mechanical: Multivalue Switch", "Mechanical: Force", "Mechanical: Pressure", "Mechanical: Strain", "Mechanical: Weight",
			"Mechanical: Haptic Vibrator", "Mechanical: Hall Effect Switch", 0, 0, 0, 0, 0, 0,
			"Motion", "Motion: Accelerometer 1D", "Motion: Accelerometer 2D", "Motion: Accelerometer 3D", "Motion: Gyrometer 1D", "Motion: Gyrometer 2D", "Motion: Gyrometer 3D", "Motion: Motion Detector",
			"Motion: Speedometer", "Motion: Accelerometer", "Motion: Gyrometer", 0, 0, 0, 0, 0,
			"Orientation", "Orientation: Compass 1D", "Orientation: Compass 2D", "Orientation: Compass 3D", "Orientation: Inclinometer 1D", "Orientation: Inclinometer 2D", "Orientation: Inclinometer 3D", "Orientation: Distance 1D",
			"Orientation: Distance 2D", "Orientation: Distance 3D", "Orientation: Device Orientation", "Orientation: Compass", "Orientation: Inclinometer", "Orientation: Distance", 0, 0,
			"Scanner", "Scanner: Barcode", "Scanner: RFID", "Scanner: NFC", 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"Time", "Time: Alarm Timer", "Time: Real Time Clock", 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"Other", "Other: Custom", "Other: Generic", "Other: Generic Enumerator", 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
		};
		uint32_t usageWithoutModifier = usage & 0xff;
		text = usageWithoutModifier < sizeof(lookup)/sizeof(unsigned char *) ? (lookup[usageWithoutModifier] ? strdup(lookup[usageWithoutModifier]) : 0) : 0;
	}
	
	else if (usagePage == 0x40)	// Medical Instrument
	{
		char *lookup[] = {
			0, "Medical Ultrasound", 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"VCR/Acquisition", "Freeze/Thaw", "Clip Store", "Update", "Next", "Save", "Print", "Microphone Enable",
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"Cine", "Transmit Power", "Volume", "Focus", "Depth", 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"Soft Step - Primary", "Soft Step - Secondary", 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"Depth Gain Compensation", 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"Zoom Select", "Zoom Adjust", "Spectral Doppler Mode Select", "Spectral Doppler Adjust", "Color Doppler Mode Select", "Color Doppler Adjust", "Motion Mode Select", "Motion Mode Adjust",
			"2-D Mode Select", "2-D Mode Adjust", 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"Soft Control Select", "Soft Control Adjust", 0, 0, 0, 0, 0, 0,
		};
		text = usage < sizeof(lookup)/sizeof(unsigned char *) ? (lookup[usage] ? strdup(lookup[usage]) : 0) : 0;
	}

	else if (usagePage == 0x84)	// Power Device
	{
		char *lookup[] = {
			0, "iName", "PresentStatus", "ChangedStatus", "UPS", "PowerSupply", 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"BatterySystem", "BatterySystemID", "Battery", "BatteryID", "Charger", "ChargerID", "PowerConverter", "PowerConverterID",
			"OutletSystem", "OutletSystemID", "Input", "InputID", "Output", "OutputID", "Flow", "FlowID",
			"Outlet", "OutletID", "Gang", "GangID", "Sink", "SinkID", 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"Voltage", "Current", "Frequency", "ApparentPower", "ActivePower", "PercentLoad", "Temperature", "Humidity",
			"BadCount", 0, 0, 0, 0, 0, 0, 0,
			"ConfigVoltage", "ConfigCurrent", "ConfigFrequency", "ConfigApparentPower", "ConfigActivePower", "ConfigPercentLoad", "ConfigTemperature", "ConfigHumidity",
			0, 0, 0, 0, 0, 0, 0, 0,
			"SwitchOnControl", "SwitchOffControl", "ToggleControl", "LowVoltageTransfer", "HighVoltageTransfer", "DelayBeforeReboot", "DelayBeforeStartup", "DelayBeforeShutdown",
			"Test", "ModuleReset", "AudibleAlarmControl", 0, 0, 0, 0, 0,
			"Present", "Good", "InternalFailure", "VoltageOutOfRange", "FrequencyOutOfRange", "Overload", "OverCharged", "OverTemperature",
			"ShutdownRequested", "ShutdownImminent", 0, "SwitchOn/Off", "Switchable", "Used", "Boost", "Buck",
			"Initialized", "Tested", "AwaitingPower", "CommunicationLost", 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, "iManufacturer", "iProduct", "iSerialNumber",
		};
		text = usage < sizeof(lookup)/sizeof(unsigned char *) ? (lookup[usage] ? strdup(lookup[usage]) : 0) : 0;
	}

	else if (usagePage == 0x85)	// Battery System
	{
		char *lookup[] = {
			0, "SMBBatteryMode", "SMBBatteryStatus", "SMBAlarmWarning", "SMBChargerMode", "SMBChargerStatus", "SMBChargerSpecInfo", "SMBSelectorState",
			"SMBSelectorPresets", "SMBSelectorInfo", 0, 0, 0, 0, 0, 0,
			"OptionalMfgFunction1", "OptionalMfgFunction2", "OptionalMfgFunction3", "OptionalMfgFunction4", "OptionalMfgFunction5", "ConnectionToSMBus", "OutputConnection", "ChargerConnection",
			"BatteryInsertion", "Usenext", "OKToUse", "BatterySupported", "SelectorRevision", "ChargingIndicator", 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"ManufacturerAccess", "RemainingCapacityLimit", "RemainingTimeLimit", "AtRate", "CapacityMode", "BroadcastToCharger", "PrimaryBattery", "ChargeController",
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"TerminateCharge", "TerminateDischarge", "BelowRemainingCapacityLimit", "RemainingTimeLimitExpired", "Charging", "Discharging", "FullyCharged", "FullyDischarged",
			"ConditioningFlag", "AtRateOK", "SMBErrorCode", "NeedReplacement", 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"AtRateTimeToFull", "AtRateTimeToEmpty", "AverageCurrent", "Maxerror", "RelativeStateOfCharge", "AbsoluteStateOfCharge", "RemainingCapacity", "FullChargeCapacity",
			"RunTimeToEmpty", "AverageTimeToEmpty", "AverageTimeToFull", "CycleCount", 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"BattPackModelLevel", "InternalChargeController", "PrimaryBatterySupport", "DesignCapacity", "SpecificationInfo", "ManufacturerDate", "SerialNumber", "iManufacturerName",
			"iDevicename", "iDeviceChemistry", "ManufacturerData", "Rechargeable", "WarningCapacityLimit", "CapacityGranularity1", "CapacityGranularity2", "iOEMInformation",
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"InhibitCharge", "EnablePolling", "ResetToZero", 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"ACPresent", "BatteryPresent", "PowerFail", "AlarmInhibited", "ThermistorUnderRange", "ThermistorHot", "ThermistorCold", "ThermistorOverRange",
			"VoltageOutOfRange", "CurrentOutOfRange", "CurrentNotRegulated", "VoltageNotRegulated", "MasterMode", 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"ChargerSelectorSupport", "ChargerSpec", "Level2", "Level3", 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
		};
		text = usage < sizeof(lookup)/sizeof(unsigned char *) ? (lookup[usage] ? strdup(lookup[usage]) : 0) : 0;
	}

	else if (usagePage == 0x90)	// Camera Control
	{
		char *lookup[] = {
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"Camera Auto-focus", "Camera Shutter", 0, 0, 0, 0, 0, 0,
		};
		text = usage < sizeof(lookup)/sizeof(unsigned char *) ? (lookup[usage] ? strdup(lookup[usage]) : 0) : 0;
	}

	else if (usagePage == 0xff)	// Fn key on Apple Keyboards
	{
		char *lookup[] = {
			0, 0, 0, "Fn key", 0, 0, 0, 0,
		};
		text = usage < sizeof(lookup)/sizeof(unsigned char *) ? (lookup[usage] ? strdup(lookup[usage]) : 0) : 0;
	}

	else if (usagePage == 0xf1d0)	// Fast IDentity Online Alliance
	{
		char *lookup[] = {
			0, "U2F Authenticator Device", 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			"Input Report Data", "Output Report Data", 0, 0, 0, 0, 0, 0,
		};
		text = usage < sizeof(lookup)/sizeof(unsigned char *) ? (lookup[usage] ? strdup(lookup[usage]) : 0) : 0;
	}

	else if (usagePage >= 0xff00 && usagePage <= 0xffff)
		asprintf(&text, "Vendor-defined (%04x:%04x)", usagePage, usage);

	if (!text)
		asprintf(&text, "unknown (%04x:%04x)", usagePage, usage);
	
	return text;
}
