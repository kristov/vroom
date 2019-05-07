#include "stdint.h"

struct screen_manager {
    struct card cards[256];
    uint8_t nr_cards;
};

uint8_t screen_manager_set_screen_mode(struct screen_manager* manager, uint8_t screen_id, uint8_t mode) {
}

uint8_t screen_manager_queue_add_data_load(struct screen_manager* manager, uint8_t screen_id) {
}

uint8_t screen_manager_queue_add_texture_load(struct screen_manager* manager, uint8_t screen_id) {
}

uint8_t screen_manager_queue_update_matrix(struct screen_manager* manager, uint8_t screen_id) {
}

uint8_t screen_manager_run(struct screen_manager* manager) {
    // 
}

uint8_t screen_manager_init(struct screen_manager* manager) {
}
