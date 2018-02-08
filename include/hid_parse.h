#include <stdint.h>

typedef struct hid_input_report_item_usage hid_input_report_item_usage_t;
struct hid_input_report_item_usage {
    uint32_t usage_page;
    uint32_t usage;
};

typedef struct hid_input_report_item hid_input_report_item_t;
struct hid_input_report_item {
    uint32_t logical_maximum;
    uint32_t logical_minimum;
    uint32_t report_count;
    uint32_t report_size;
    uint32_t bit_offset;
    uint32_t bit_size;
    uint8_t is_data;
    uint8_t is_array;
    uint8_t is_absolute;
    uint8_t is_nowrap;
    uint8_t is_linear;
    uint8_t is_preferredstate;
    uint8_t is_nonullstate;
    uint8_t is_nonvolatile;
    uint32_t nr_usages;
    hid_input_report_item_usage_t* usages;
};

typedef struct hid_input_report hid_input_report_t;
struct hid_input_report {
    uint32_t report_id;
    unsigned char* description;
    uint32_t nr_report_items;
    hid_input_report_item_t* report_items;
};

typedef struct hid_input_device hid_input_device_t;
struct hid_input_device {
    unsigned char* description;
    uint32_t nr_reports;
    hid_input_report_t* reports;
};

hid_input_device_t* hid_parse_report_descriptor(uint8_t* buffer, uint32_t length);
void hid_dump_device(hid_input_device_t* device);
void hid_input_destroy_device(hid_input_device_t* device);
