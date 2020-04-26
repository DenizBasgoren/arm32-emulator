initial_sp:        .word    0x3FFFFFFF
reset_vector:     .word     _main

_main:
    bl init

    .game_loop:
    bl draw
    bl update
    b .game_loop

/////////////////////////////////////////////////
init:
    ldr r4, cells                        // cells

    .set_pixel_randomly:
        //.hword 0xde00
        ldr r0, seed                     // seed
        ldr r1, rand1                    // rand1
        ldr r2, rand2                    // rand2

        mul r0, r1                        // r0 *= r1
        add r0, r2                        // r0 += r2

        ldr r1, =seed                    // seed = r0
        str r0, [r1]

        cmp r0, #0                        // if r0 < 0
        ble .cell_is_zero

        // else cell_is_one
        mov r1, #1
        str r1, [r4]

        b .move_onto_next_cell

        .cell_is_zero:
        mov r1, #0
        str r1, [r4]

        .move_onto_next_cell:
        add r4, #4

        ldr r1, buffer
        cmp r4, r1
        bcc .set_pixel_randomly
    bx lr



// cells (current)
.balign 4
cells:            .word     0x20000000
buffer:           .word     0x20005460
rand1:            .word     1140671485
rand2:            .word     12820163
seed:             .word     1243


////////////////////////////////////////
draw:
	ldr		r0, =#0x20000000		// index -> r0
    mov     r1, #0                  // i
	.each_row_in_cells:
        mov     r2, #0                  // j
        .each_colum_in_cells:
            // draw 10x10 pixel to position r3=r1*10, r4=r2*10
            mov     r5, #10
            mov     r3, r1
            mul     r3, r5      // start index_row
            mov     r4, r2
            mul     r4, r5      // start index_col

            mov     r8, r1
            mov     r9, r2
            
            ldr     r1, [r0]
            cmp     r1, #1
            bne     .setblack
            ldr     r1, =0xffffffff
            b       .endcolor
            .setblack:
            ldr     r1, =0xff000000
            .endcolor:
            ldr     r5, =0x40010000     // peripheral

            mov     r10, r4             // col_start

            mov     r6, #0
            .each_row:
                str     r3, [r5]        // row
                add     r3, #1          // row++
                mov     r4, r10
                mov     r7, #0
                .each_colum:
                    str     r4, [r5, #0x4]  // col
                    str     r1, [r5, #0x8]  // write the color
                    add     r4, #1          // col++
                    // .hword  0xde00
                    add     r7, #1
                    cmp     r7, #9
                    blt     .each_colum
                add     r6, #1
                cmp     r6, #9
                blt     .each_row
            //////////////////////////////////////////////////////

            mov     r1, r8
            mov     r2, r9

            add     r0, #4
            add     r2, #1
            cmp     r2, #32
            blt     .each_colum_in_cells
        add     r1, #1
        cmp     r1, #24
        blt     .each_row_in_cells

    ldr     r5, =0x40010000     // peripheral
    str     r5, [r5, #0xC]
    bx lr



update:
	ldr		r0, =#0x20000000		// cell
	ldr		r4, =#0x20005460		// next
	ldr		r7, =#0x20005460		// next
    add     r0, #132
    add     r4, #132
    sub     r7, #132
    .check_neighbours:
        mov     r1, #0                  // neighbours = 0
        mov     r2, r0                  
        
        sub     r2, #132                 // goto top-left
        ldr     r3, [r2]
        add     r1, r3

        add     r2, #4                  // goto top
        ldr     r3, [r2]
        add     r1, r3   
        
        add     r2, #4                  // goto top-right
        ldr     r3, [r2]
        add     r1, r3  

        add     r2, #120                  // goto left
        ldr     r3, [r2]
        add     r1, r3  
        
        add     r2, #8                  // goto right
        ldr     r3, [r2]
        add     r1, r3  

        add     r2, #120                  // goto bot-left
        ldr     r3, [r2]
        add     r1, r3  

        add     r2, #4                  // goto bottom
        ldr     r3, [r2]
        add     r1, r3   
        
        add     r2, #4                  // goto bot-right
        ldr     r3, [r2]
        add     r1, r3  

        // r1 is neighbours
        ldr     r2, [r0]    // current
        cmp     r2, #1
        bne     .dead
        // is alive will be dead
        cmp     r1, #2
        blt     .alive
        cmp     r1, #3
        ble     .else
        .alive:
        mov     r5, #0    // next = 0
        b       .out
        .dead:
        cmp     r1, #3
        bne     .else
        mov     r5, #1
        b       .out
        .else:
        mov     r5, r2
        .out:
        str     r4, [r5]

        add     r0, #4
        add     r4, #4
        cmp     r0, r7
        blt     .check_neighbours


    // Swap buffers
	ldr		r0, =#0x20005460		// mem1 -> r0
	ldr		r1, =#0x20000000		// mem2 -> r1
	mov     r2, r0                  // mem2_buff
    .copy_mem:
    ldr     r3, [r0]
    str     r1, [r3]
    add     r0, #4
    add     r1, #4
    cmp     r1, r2
    bne     .copy_mem
    
    bx lr
