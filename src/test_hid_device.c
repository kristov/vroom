#include "hid_device.h"
#include "test_harness.h"

void test_oculus_dk1_report_descriptor(test_harness_t* test) {
    hid_device_t* device;

/*
device:
  report [id: 10]:
    report item: [size:8, count:1]
      [DAT|ARR|ABS|NOW|LIN|PRE|NUL|VOL]
      [ 0 | 0 | 1 | 1 | 1 | 1 | 1 | 1 ]
      report item usage: Head Tracker
      report item usage: Vendor-defined (ff00:0001)
      report item usage: Byte Count
    report item: [size:16, count:1]
      [DAT|ARR|ABS|NOW|LIN|PRE|NUL|VOL]
      [ 0 | 0 | 1 | 1 | 1 | 1 | 1 | 1 ]
      report item usage: Byte Count
    report item: [size:16, count:1]
      [DAT|ARR|ABS|NOW|LIN|PRE|NUL|VOL]
      [ 0 | 0 | 1 | 1 | 1 | 1 | 1 | 1 ]
      report item usage: Byte Count
    report item: [size:16, count:1]
      [DAT|ARR|ABS|NOW|LIN|PRE|NUL|VOL]
      [ 0 | 0 | 1 | 1 | 1 | 1 | 1 | 1 ]
      report item usage: Byte Count
    report item: [size:8, count:16]
      [DAT|ARR|ABS|NOW|LIN|PRE|NUL|VOL]
      [ 0 | 0 | 1 | 1 | 1 | 1 | 1 | 1 ]
      report item usage: Byte Count
    report item: [size:8, count:16]
      [DAT|ARR|ABS|NOW|LIN|PRE|NUL|VOL]
      [ 0 | 0 | 1 | 1 | 1 | 1 | 1 | 1 ]
      report item usage: Byte Count
    report item: [size:8, count:16]
      [DAT|ARR|ABS|NOW|LIN|PRE|NUL|VOL]
      [ 0 | 0 | 1 | 1 | 1 | 1 | 1 | 1 ]
      report item usage: Byte Count
    report item: [size:16, count:3]
      [DAT|ARR|ABS|NOW|LIN|PRE|NUL|VOL]
      [ 0 | 0 | 1 | 1 | 1 | 1 | 1 | 1 ]
      report item usage: Byte Count
*/

    uint8_t descriptor[] = {
		0x05, 0x03,
		0x09, 0x05,
		0xa1, 0x01,
		0x06, 0x00,
		0xff, 0x09,
		0x01, 0xa1,
		0x02, 0x05,
		0x01, 0x85,
		0x01, 0x09,
		0x3b, 0x15,
		0x00, 0x26,
		0xff, 0x00,
		0x75, 0x08,
		0x95, 0x01,
		0x81, 0x03,
		0x09, 0x3b,
		0x15, 0x00,
		0x27, 0xff,
		0xff, 0x00,
		0x00, 0x75,
		0x10, 0x95,
		0x01, 0x81,
		0x03, 0x09,
		0x3b, 0x15,
		0x00, 0x27,
		0xff, 0xff,
		0x00, 0x00,
		0x75, 0x10,
		0x95, 0x01,
		0x81, 0x03,
		0x09, 0x3b,
		0x15, 0x00,
		0x27, 0xff,
		0xff, 0x00,
		0x00, 0x75,
		0x10, 0x95,
		0x01, 0x81,
		0x03, 0x09,
		0x3b, 0x15,
		0x00, 0x26,
		0xff, 0x00,
		0x75, 0x08,
		0x95, 0x10,
		0x81, 0x03,
		0x09, 0x3b,
		0x15, 0x00,
		0x26, 0xff,
		0x00, 0x75,
		0x08, 0x95,
		0x10, 0x81,
		0x03, 0x09,
		0x3b, 0x15,
		0x00, 0x26,
		0xff, 0x00,
		0x75, 0x08,
		0x95, 0x10,
		0x81, 0x03,
		0x09, 0x3b,
		0x15, 0x00,
		0x27, 0xff,
		0xff, 0x00,
		0x00, 0x75,
		0x10, 0x95,
		0x03, 0x81,
		0x03, 0x85,
		0x02, 0x09,
		0x3b, 0x15,
		0x00, 0x27,
		0xff, 0xff,
		0x00, 0x00,
		0x75, 0x10,
		0x95, 0x01,
		0xb1, 0x03,
		0x09, 0x3b,
		0x15, 0x00,
		0x26, 0xff,
		0x00, 0x75,
		0x08, 0x95,
		0x04, 0xb1,
		0x03, 0x85,
		0x03, 0x09,
		0x3b, 0x15,
		0x00, 0x27,
		0xff, 0xff,
		0x00, 0x00,
		0x75, 0x10,
		0x95, 0x01,
		0xb1, 0x03,
		0x09, 0x3b,
		0x15, 0x00,
		0x26, 0xff,
		0x00, 0x75,
		0x08, 0x95,
		0x42, 0xb1,
		0x03, 0x85,
		0x04, 0x09,
		0x3b, 0x15,
		0x00, 0x27,
		0xff, 0xff,
		0x00, 0x00,
		0x75, 0x10,
		0x95, 0x01,
		0xb1, 0x03,
		0x09, 0x3b,
		0x15, 0x00,
		0x26, 0xff,
		0x00, 0x75,
		0x08, 0x95,
		0x05, 0xb1,
		0x03, 0x85,
		0x05, 0x09,
		0x3b, 0x15,
		0x00, 0x27,
		0xff, 0xff,
		0x00, 0x00,
		0x75, 0x10,
		0x95, 0x01,
		0xb1, 0x03,
		0x09, 0x3b,
		0x15, 0x00,
		0x26, 0xff,
		0x00, 0x75,
		0x08, 0x95,
		0x03, 0xb1,
		0x03, 0x85,
		0x06, 0x09,
		0x3b, 0x15,
		0x00, 0x27,
		0xff, 0xff,
		0x00, 0x00,
		0x75, 0x10,
		0x95, 0x01,
		0xb1, 0x03,
		0x09, 0x3b,
		0x15, 0x00,
		0x26, 0xff,
		0x00, 0x75,
		0x08, 0x95,
		0x01, 0xb1,
		0x03, 0x85,
		0x07, 0x09,
		0x3b, 0x15,
		0x00, 0x27,
		0xff, 0xff,
		0x00, 0x00,
		0x75, 0x10,
		0x95, 0x01,
		0xb1, 0x03,
		0x09, 0x3b,
		0x15, 0x00,
		0x26, 0xff,
		0x00, 0x75,
		0x08, 0x95,
		0x02, 0xb1,
		0x03, 0x85,
		0x08, 0x09,
		0x3b, 0x15,
		0x00, 0x27,
		0xff, 0xff,
		0x00, 0x00,
		0x75, 0x10,
		0x95, 0x01,
		0xb1, 0x03,
		0x09, 0x3b,
		0x15, 0x00,
		0x26, 0xff,
		0x00, 0x75,
		0x08, 0x95,
		0x02, 0xb1,
		0x03, 0x85,
		0x09, 0x09,
		0x3b, 0x15,
		0x00, 0x27,
		0xff, 0xff,
		0x00, 0x00,
		0x75, 0x10,
		0x95, 0x01,
		0xb1, 0x03,
		0x09, 0x3b,
		0x15, 0x00,
		0x26, 0xff,
		0x00, 0x75,
		0x08, 0x95,
		0x35, 0xb1,
		0x03, 0x85,
		0x0a, 0x09,
		0x3b, 0x15,
		0x00, 0x27,
		0xff, 0xff,
		0x00, 0x00,
		0x75, 0x10,
		0x95, 0x01,
		0xb1, 0x03,
		0x09, 0x3b,
		0x15, 0x00,
		0x26, 0xff,
		0x00, 0x75,
		0x08, 0x95,
		0x0c, 0xb1,
		0x03, 0xc0,
		0xc0
    };

    test_harness_make_note(test, "Testing Oculus DK1");
    device = hid_device_report_descriptor(descriptor, 401);
    hid_device_dump(device);
}

void test_my_wireless_mouse_report_descriptor(test_harness_t* test) {
    hid_device_t* device;
    uint32_t failed;

    uint8_t descriptor[] = {
        0x05, 0x01,
        0x09, 0x02,
        0xa1, 0x01,
        0x85, 0x01,
        0x09, 0x01,
        0xa1, 0x00,
        0x05, 0x09,
        0x19, 0x01,
        0x29, 0x05,
        0x15, 0x00,
        0x25, 0x01,
        0x95, 0x05,
        0x75, 0x01,
        0x81, 0x02,
        0x95, 0x01,
        0x75, 0x03,
        0x81, 0x01,
        0x05, 0x01,
        0x09, 0x30,
        0x09, 0x31,
        0x15, 0x81,
        0x25, 0x7f,
        0x75, 0x08,
        0x95, 0x02,
        0x81, 0x06,
        0x09, 0x38,
        0x15, 0x81,
        0x25, 0x7f,
        0x75, 0x08,
        0x95, 0x01,
        0x81, 0x06,
        0xc0,
        0xc0,
        0x05, 0x0c,
        0x09, 0x01,
        0xa1, 0x01,
        0x85, 0x03,
        0x75, 0x10,
        0x95, 0x02,
        0x15, 0x01,
        0x26, 0x8c,
        0x02, 0x19,
        0x01, 0x2a,
        0x8c, 0x02,
        0x81, 0x00,
        0xc0,
        0x05, 0x01,
        0x09, 0x80,
        0xa1, 0x01,
        0x85, 0x04,
        0x75, 0x02,
        0x95, 0x01,
        0x15, 0x01,
        0x25, 0x03,
        0x09, 0x82,
        0x09, 0x81,
        0x09, 0x83,
        0x81, 0x60,
        0x75, 0x06,
        0x81, 0x03,
        0xc0,
        0x05, 0x01,
        0x09, 0x00,
        0xa1, 0x01,
        0x85, 0x05,
        0x06, 0x00,
        0xff, 0x09,
        0x01, 0x15,
        0x81, 0x25,
        0x7f, 0x75,
        0x08, 0x95,
        0x07, 0xb1,
        0x02,
        0xc0
    };

    failed = test_harness_nr_failed_tests(test);

    test_harness_make_note(test, "Testing weird wireless mouse");
    device = hid_device_report_descriptor(descriptor, 142);

    is_equal_uint32(test, device->nr_reports, 3, "device has 3 reports");
	is_equal_uint32(test, device->reports[0].report_id, 1, "first report id is 1");
	is_equal_uint32(test, device->reports[1].report_id, 3, "second report id is 3");
	is_equal_uint32(test, device->reports[2].report_id, 4, "third report id is 4");
	
    hid_input_report_t report = device->reports[0];
    is_equal_uint32(test, report.nr_report_items, 4, "first report has 4 report items");
    is_equal_uint32(test, report.report_items[0].report_size, 1, "report1 report_size");
    is_equal_uint32(test, report.report_items[0].report_count, 5, "report1 report_count");

    is_equal_uint32(test, report.report_items[1].report_size, 3, "report2 report_size");
    is_equal_uint32(test, report.report_items[1].report_count, 1, "report2 report_count");
    is_equal_uint32(test, report.report_items[1].nr_usages, 0, "report2 nr usages is zero");

    is_equal_uint32(test, report.report_items[2].report_size, 8, "report3 report_size");
    is_equal_uint32(test, report.report_items[2].report_count, 2, "report3 report_count");

    is_equal_uint32(test, report.report_items[3].report_size, 8, "report4 report_size");
    is_equal_uint32(test, report.report_items[3].report_count, 1, "report4 report_count");

    if (test_harness_nr_failed_tests(test) > failed) {
        hid_device_dump(device);
    }

    hid_device_destroy(device);
}

void test_my_dell_keyboard_report_descriptor(test_harness_t* test) {
    hid_device_t* device;
    uint32_t failed;

    uint8_t descriptor[] = {
        0x05, 0x01,
        0x09, 0x06,
        0xa1, 0x01,
        0x05, 0x07,
        0x19, 0xe0,
        0x29, 0xe7,
        0x15, 0x00,
        0x25, 0x01,
        0x75, 0x01,
        0x95, 0x08,
        0x81, 0x02,
        0x95, 0x01,
        0x75, 0x08,
        0x81, 0x01,
        0x95, 0x03,
        0x75, 0x01,
        0x05, 0x08,
        0x19, 0x01,
        0x29, 0x03,
        0x91, 0x02,
        0x95, 0x01,
        0x75, 0x05,
        0x91, 0x01,
        0x95, 0x06,
        0x75, 0x08,
        0x15, 0x00,
        0x26, 0xff,
        0x00, 0x05,
        0x07, 0x19,
        0x00, 0x2a,
        0xff, 0x00,
        0x81, 0x00,
        0xc0
    };

    failed = test_harness_nr_failed_tests(test);

    test_harness_make_note(test, "Testing descriptor from Dell Keyboard");
    device = hid_device_report_descriptor(descriptor, 65);

    is_equal_uint32(test, device->nr_reports, 1, "device has 1 report");
	hid_input_report_t report = device->reports[0];
	is_equal_uint32(test, report.report_id, 0, "report id is 0");
    is_equal_uint32(test, report.nr_report_items, 3, "report has 3 items");

    is_equal_uint32(test, report.report_items[0].report_size, 1, "report1 report_size");
    is_equal_uint32(test, report.report_items[0].report_count, 8, "report1 report_count");
    is_equal_uint32(test, report.report_items[0].nr_usages, 8, "report1 nr_usages");
    is_equal_uint32(test, report.report_items[0].usages[0].usage_page, 7, "report1 usage1 usage_page");
    is_equal_uint32(test, report.report_items[0].usages[0].usage, 224, "report1 usage1 usage");

    is_equal_uint32(test, report.report_items[1].report_size, 8, "report2 report_size");
    is_equal_uint32(test, report.report_items[1].report_count, 1, "report2 report_count");
	is_equal_uint32(test, report.report_items[1].nr_usages, 0, "report2 has zero usages");

    is_equal_uint32(test, report.report_items[2].report_size, 8, "report3 report_size");
    is_equal_uint32(test, report.report_items[2].report_count, 6, "report3 report_count");
    is_equal_uint32(test, report.report_items[2].usages[0].usage_page, 7, "report3 usage1 usage_page");
    is_equal_uint32(test, report.report_items[2].usages[0].usage, 0, "report3 usage1 usage");

    if (test_harness_nr_failed_tests(test) > failed) {
        hid_device_dump(device);
    }

    hid_device_destroy(device);
}

void test_sample_mouse_report_descriptor(test_harness_t* test) {
    hid_device_t* device;

    uint8_t descriptor[] = {
        0x05, 0x01, // USAGE_PAGE (Generic Desktop)
        0x09, 0x02, // USAGE (Mouse)
        0xa1, 0x01, // COLLECTION (Application)
        0x09, 0x01, //   USAGE (Pointer)
        0xa1, 0x00, //   COLLECTION (Physical)
        0x05, 0x09, //     USAGE_PAGE (Button)
        0x19, 0x01, //     USAGE_MINIMUM (Button 1)
        0x29, 0x03, //     USAGE_MAXIMUM (Button 3)
        0x15, 0x00, //     LOGICAL_MINIMUM (0)
        0x25, 0x01, //     LOGICAL_MAXIMUM (1)
        0x95, 0x03, //     REPORT_COUNT (3)
        0x75, 0x01, //     REPORT_SIZE (1)
        0x81, 0x02, //     INPUT (Data,Var,Abs)
        0x95, 0x01, //     REPORT_COUNT (1)
        0x75, 0x05, //     REPORT_SIZE (5)
        0x81, 0x03, //     INPUT (Cnst,Var,Abs)
        0x05, 0x01, //     USAGE_PAGE (Generic Desktop)
        0x09, 0x30, //     USAGE (X)
        0x09, 0x31, //     USAGE (Y)
        0x15, 0x81, //     LOGICAL_MINIMUM (-127)
        0x25, 0x7f, //     LOGICAL_MAXIMUM (127)
        0x75, 0x08, //     REPORT_SIZE (8)
        0x95, 0x02, //     REPORT_COUNT (2)
        0x81, 0x06, //     INPUT (Data,Var,Rel)
        0xc0,       //   END_COLLECTION
        0xc0        // END_COLLECTION
    };

    test_harness_make_note(test, "Testing example descriptor for mouse");
    device = hid_device_report_descriptor(descriptor, 50);

    is_equal_uint32(test, device->nr_reports, 1, "device has 1 report");
	hid_input_report_t report = device->reports[0];
	is_equal_uint32(test, report.report_id, 0, "report id is 0");
    is_equal_uint32(test, report.nr_report_items, 3, "report has 3 items");

    is_equal_uint32(test, report.report_items[0].report_size, 1, "report1 report_size");
    is_equal_uint32(test, report.report_items[0].report_count, 3, "report1 report_count");
    is_equal_uint32(test, report.report_items[0].nr_usages, 3, "report1 nr_usages");
    is_equal_uint32(test, report.report_items[0].usages[0].usage_page, 9, "report1 usage1 usage_page");
    is_equal_uint32(test, report.report_items[0].usages[0].usage, 1, "report1 usage1 usage");
    is_equal_uint32(test, report.report_items[0].usages[1].usage_page, 9, "report1 usage2 usage");
    is_equal_uint32(test, report.report_items[0].usages[1].usage, 2, "report1 usage2 usage");
    is_equal_uint32(test, report.report_items[0].usages[2].usage_page, 9, "report1 usage3 usage_page");
    is_equal_uint32(test, report.report_items[0].usages[2].usage, 3, "report1 usage3 usage");

    is_equal_uint32(test, report.report_items[1].report_size, 5, "report2 report_size");
    is_equal_uint32(test, report.report_items[1].report_count, 1, "report2 report_count");
	is_equal_uint32(test, report.report_items[1].nr_usages, 0, "report2 has zero usages");

    is_equal_uint32(test, report.report_items[2].report_size, 8, "report3 report_size");
    is_equal_uint32(test, report.report_items[2].report_count, 2, "report3 report_count");
    is_equal_uint32(test, report.report_items[2].usages[0].usage_page, 1, "report3 usage1 usage_page");
    is_equal_uint32(test, report.report_items[2].usages[0].usage, 48, "report3 usage1 usage");
    is_equal_uint32(test, report.report_items[2].usages[1].usage_page, 1, "report3 usage2 usage_page");
    is_equal_uint32(test, report.report_items[2].usages[1].usage, 49, "report3 usage2 usage");

    hid_device_destroy(device);
}

int main(void) {
    test_harness_t* test = test_harness_create();
    //test->verbose = 1;

    test_sample_mouse_report_descriptor(test);
    test_my_dell_keyboard_report_descriptor(test);
    test_my_wireless_mouse_report_descriptor(test);
    //test_oculus_dk1_report_descriptor(test); /* hid_device is BROKEN for this */

    test_harness_exit_with_status(test);
}
