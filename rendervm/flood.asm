; Memory locations of the variables. Map memory ends
; at 0x64.
;
var pl_x  0x19
var pl_y  0x1a
var tmp_x 0x1b
var tmp_y 0x1c
var tile  0x1d

; Call the main function. Then release control back
; to the server.
main:
    call flood
    yield

; Generates an index on the uint16 stack from two y
; and x values from the uint8 stack. This index is
; used to get the map value from uint8 memory.
;
; This map value is then pushed onto the uint8 stack
; for testing.
;
load_tile:
    ; Copy the coords as we will move them
    uint8_dup 2

    ; Move y onto the uint16 stack and multiply by 64.
    uint16_move_uint8
    uint16_push 5
    uint16_multiply

    ; Move the x value onto the uint16 stack and add
    ; to the previous value.
    uint16_move_uint8
    uint16_add

    ; Duplicate the loaded tile address for later.
    uint16_dup 1

    ; Take this idx from the 16 stack and load that
    ; memory value from uint8 map memory onto the
    ; uint8 stack.
    uint8_load_uint16

    ; Store the tile value in a variable and remove
    ; it from the stack.
    uint8_store tile

    ; Set the map value to 10 to mark it as seen.
    uint8_push 10
    uint8_store_uint16

    return

; Uses a flood fill from the square the player is in
; to create a stack of wall faces that need to be
; rendered.
;
; The current x and y is stored on the top of the
; stack. It tests up, down, left and right of that
; coordinate and if there is a wall there is adds that
; face to a list.
;
; The result is a list of coords and wall faces on the
; uint16 stack. This can be used to generate the wall
; meshes that need to be drawn.
;
flood:
    ; Push the initial player x and y values. This is
    ; where the flood fill algo will start from. The
    ; y is pushed last so the idx routine works.
    ;
    uint8_load pl_x
    uint8_load pl_y

    ; This loads the tile where the player is. This is
    ; to set the player map value to 10: visited
    ; because its not possible to be in a wall.
    call load_tile

; The loop of the stack.
;
flood_loop:
    ; Test to see if there are any values on the
    ; uint8 stack.
    uint8_jump_empty flood_end

    uint8_store tmp_x
    uint8_store tmp_y

up_test:
    ; Change y to y - 1 and create the map index.
    uint8_load tmp_x
    uint8_load tmp_y
    uint8_push 1
    uint8_subtract
    call load_tile

    ; If its zero its more floor so leave the coords
    ; on the stack for the next loop and move on.
    uint8_load tile
    uint8_push 0
    uint8_eq
    uint8_jump_nzero down_test

    ; If its 10 it means its been visited so throw
    ; away the coords.
    uint8_load tile
    uint8_push 10
    uint8_eq
    uint8_jump_nzero up_throw

    ; Push the number 4 (down facing side) onto the
    ; uint16 stack. Then push the coords.
    uint16_push 4
    uint16_copy_uint8 2

up_throw:
    ; Throw away the up coords
    uint8_pop
    uint8_pop

down_test:
    ; Change y to y + 1 and create the map index.
    uint8_load tmp_x
    uint8_load tmp_y
    uint8_push 1
    uint8_add
    call load_tile

    ; If its zero its more floor so leave the coords
    ; on the stack for the next loop and move on.
    uint8_load tile
    uint8_push 0
    uint8_eq
    uint8_jump_nzero left_test

    ; If its 10 it means its been visited so throw
    ; away the coords.
    uint8_load tile
    uint8_push 10
    uint8_eq
    uint8_jump_nzero down_throw

    ; Push the number 1 (up facing side) onto the
    ; uint16 stack. Then push the coords.
    uint16_push 1
    uint16_copy_uint8 2

down_throw:
    ; Throw away the down coords
    uint8_pop
    uint8_pop

left_test:
    ; Change x to x - 1 and create the map index.
    uint8_load tmp_y
    uint8_load tmp_x
    uint8_push 1
    uint8_subtract
    uint8_swap
    call load_tile

    ; If its zero its more floor so leave the coords
    ; on the stack for the next loop and move on.
    uint8_load tile
    uint8_push 0
    uint8_eq
    uint8_jump_nzero right_test

    ; If its 10 it means its been visited so throw
    ; away the coords.
    uint8_load tile
    uint8_push 10
    uint8_eq
    uint8_jump_nzero left_throw

    ; Push the number 2 (right facing side) onto the
    ; uint16 stack. Then push the coords.
    uint16_push 2
    uint16_copy_uint8 2

left_throw:
    ; Throw away the left coords
    uint8_pop
    uint8_pop

right_test:
    ; Change x to x + 1 and create the map index.
    uint8_load tmp_y
    uint8_load tmp_x
    uint8_push 1
    uint8_add
    uint8_swap
    call load_tile

    ; If its zero its more floor so leave the coords
    ; on the stack for the next loop and move on.
    uint8_load tile
    uint8_push 0
    uint8_eq
    uint8_jump_nzero flood_loop

    ; If its 10 it means its been visited so throw
    ; away the coords.
    uint8_load tile
    uint8_push 10
    uint8_eq
    uint8_jump_nzero right_throw

    ; Push the number 8 (left facing side) onto the
    ; uint16 stack. Then push the coords.
    uint16_push 8
    uint16_copy_uint8 2

right_throw:
    uint8_pop
    uint8_pop

    jump flood_loop

flood_end:
    ; End of the flood call
    return
