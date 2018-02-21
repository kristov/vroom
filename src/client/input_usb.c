#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

// sudo apt-get install libsqlite3-dev

typedef struct input_usb input_usb_t;
struct input_usb {
    sqlite3 *db;
};

input_usb_t* input_usb_create() {
    input_usb_t* input;
    input = malloc(sizeof(input_usb_t));
    if (NULL == input) {
        return NULL;
    }
    memset(input, 0, sizeof(input_usb_t));
    return input;
}

void input_usb_initialize_database(input_usb_t* input) {
    char filename[200];
    int written = snprintf(filename, 200, "/home/%s/.vroominputdb", getenv("USER"));
    if (written == 200) {
        fprintf(stderr, "path to .vroominputdb is too long!\n");
        return;
    }

    int rc = sqlite3_open(filename, &input->db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(input->db));
        sqlite3_close(input->db);
        return;
    }

    char create_t_device[] = "CREATE TABLE IF NOT EXISTS Device (vendor_id INTEGER, product_id INTEGER);";

    create table if not exists TableName (col1 typ1, ..., colN typN)
}

int main (void) {
    input_usb_t* input;

    input = input_usb_create();
    input_usb_initialize_database(input);

    return 0;       
}
