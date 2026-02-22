#include <emscripten/emscripten.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

/** Explicit type definitions: */

typedef __UINT8_TYPE__ u8;
typedef __UINT16_TYPE__ u16;
typedef __UINT32_TYPE__ u32;
typedef __UINT64_TYPE__ u64;
typedef __SIZE_TYPE__ uZ;
typedef __INT8_TYPE__ i8;
typedef __INT16_TYPE__ i16;
typedef __INT32_TYPE__ i32;
typedef __INT64_TYPE__ i64;
typedef __PTRDIFF_TYPE__ iZ;
typedef float f32;
typedef double f64;

/*
 * This file simulates Conway's Game of Life using pixel colors as the live/dead state.
 * 
 * Full white pixels are considered alive, everything else is considered dead.
 * This is important to note because the pixels fase when they die, so not all are black or white.
 */

#define MIN(a, b) ((a) < (b) ? (a) : (b))

/** A pixel in an image. */
typedef union Pixel {
    u32 data;
    struct {
        u8 r;
        u8 g;
        u8 b;
        u8 a;
    };
} Pixel;

/** A live cell. */
static const Pixel LIVE_CELL = (Pixel){ .r = 0xFF, .g = 0xFF, .b = 0xFF, .a = 0xFF };

/** A dead cell. Note that any non-white cell is dead, but this is their end state. */
static const Pixel DEAD_CELL = (Pixel){ .r = 0x00, .g = 0x00, .b = 0x00, .a = 0xFF };

/** The pixel data state. */
static struct {
    /** The current game state buffer. */
    Pixel* current;

    /** The next game state buffer. */
    Pixel* next;

    /** Game width in number of cells. */
    uZ w;

    /** Game height in number of cells. */
    uZ h;
} state = { NULL, NULL, 0, 0 };

/** Is the cell alive? */
bool is_cell_alive(Pixel p) {
    return p.data == LIVE_CELL.data;
}

/** Returns a pointer to the pixel/cell state. */
u8* get_state_data() {
    return (u8*)state.current;
}

/** Returns the state's width. */
u32 get_state_width() {
    return state.w;
}

/** Returns the state's height. */
u32 get_state_height() {
    return state.h;
}

/** Destroys the state. */
void destroy_state() {
    if (state.current) {
        free(state.current);
    }
    if (state.next) {
        free(state.next);
    }
    state.current = NULL;
    state.next = NULL;
    state.w = 0;
    state.h = 0;
}

/**
 * Initializes the state.
 * 
 * @param w The target's width.
 * @param h The target's height.
 * @param seed The initial state seed.
 */
void init_state(u32 w, u32 h, u32 seed) {
    // Clear the old state if present.
    destroy_state();

    // Calculate dimensions and allocate pixel data.
    state.w = (uZ)w / 4;
    state.h = (uZ)h / 4;
    const uZ N_PIXELS = state.w * state.h;
    state.current = (Pixel*)malloc(sizeof(Pixel) * N_PIXELS);
    state.next = (Pixel*)malloc(sizeof(Pixel) * N_PIXELS);

    // Initialize data.
    srand(seed);
    for (uZ i = 0; i < N_PIXELS; i++) {
        state.current[i] = rand() % 2 == 0 ? LIVE_CELL : DEAD_CELL;
    }
}

/** Updates the state. */
void update_state() {
    for (uZ y = 0; y < state.h; y++) {
        for (uZ x = 0; x < state.w; x++) {
            const uZ I = (y * state.w) + x;
            const bool CAN_LEFT = x > 0;
            const bool CAN_RIGHT = x < state.w - 1;
            const bool CAN_UP = y > 0;
            const bool CAN_DOWN = y < state.h - 1;
            uZ nLiveNeighbors = 0;

            if (CAN_UP && CAN_LEFT && is_cell_alive(state.current[I - state.w - 1])) {
                nLiveNeighbors++;
            }

            if (CAN_UP && is_cell_alive(state.current[I - state.w])) {
                nLiveNeighbors++;
            }

            if (CAN_UP && CAN_RIGHT && is_cell_alive(state.current[I - state.w + 1])) {
                nLiveNeighbors++;
            }

            if (CAN_LEFT && is_cell_alive(state.current[I - 1])) {
                nLiveNeighbors++;
            }

            if (CAN_RIGHT && is_cell_alive(state.current[I + 1])) {
                nLiveNeighbors++;
            }

            if (CAN_DOWN && CAN_LEFT && is_cell_alive(state.current[I + state.w - 1])) {
                nLiveNeighbors++;
            }

            if (CAN_DOWN && is_cell_alive(state.current[I + state.w])) {
                nLiveNeighbors++;
            }

            if (CAN_DOWN && CAN_RIGHT && is_cell_alive(state.current[I + state.w + 1])) {
                nLiveNeighbors++;
            }

            // Tell whether the cell will be alive or dead.
            const bool WILL_LIVE = is_cell_alive(state.current[I])
                ? nLiveNeighbors >= 2 && nLiveNeighbors <= 3
                : nLiveNeighbors == 3;
            
            if (WILL_LIVE) {
                state.next[I] = LIVE_CELL;
            } else {
                // Decay color in dead cells.
                Pixel p = state.current[I];
                p.r -= !!p.r;
                p.g -= !!p.g;
                p.b -= !!p.b;
                state.next[I] = p;
            }
        }
    }

    // Swap buffers.
    Pixel* temp = state.current;
    state.current = state.next;
    state.next = temp;
}
