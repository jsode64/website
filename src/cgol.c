#include <emscripten/emscripten.h>
#include <stdbool.h>
#include <stdlib.h>

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
 * This is important to note because the pixels fade when they die, so not all are black or white.
 */

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

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
static const Pixel LIVE_CELL = (Pixel){ .r = 0x00, .g = 0x00, .b = 0xFF, .a = 0xFF };

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

/** Is the cell at the given index alive? */
inline bool is_cell_alive_i(const uZ i) {
    return state.current[i].data == LIVE_CELL.data;
}

/** Is the cell at the given coordinates alive? */
inline bool is_cell_alive_xy(const uZ x, const uZ y) {
    const uZ i = (y * state.w) + x;
    return is_cell_alive_i(i);
}

/** Returns a pointer to the pixel/cell state. */
EMSCRIPTEN_KEEPALIVE
u8* get_state_data() {
    return (u8*)state.current;
}

/** Returns the state's width. */
EMSCRIPTEN_KEEPALIVE
u32 get_state_width() {
    return state.w;
}

EMSCRIPTEN_KEEPALIVE
/** Returns the state's height. */
u32 get_state_height() {
    return state.h;
}

EMSCRIPTEN_KEEPALIVE
/**
 * Resizes (or initializes) the state.
 * 
 * @param w The new width.
 * @param h The new height.
 * @param seed The seed for `srand`.
 */
void resize_state(const uZ w, const uZ h, const u32 seed) {
    // Create new buffers.
    const uZ bufferSize = sizeof(Pixel) * w * h;
    Pixel* const newCurrent = (Pixel*)malloc(bufferSize);
    Pixel* const newNext = (Pixel*)malloc(bufferSize);

    // Fill new current.
    srand(seed);
    for (uZ y = 0; y < h; y++) {
        for (uZ x = 0; x < w; x++) {
            const uZ i = (y * w) + x;
            if (x < state.w && y < state.h) {
                // Carryover from previous.
                newCurrent[i] = state.current[(y * state.w) + x];
            } else {
                // New cell.
                newCurrent[i] = (!state.current && rand() % 4 == 0) ? LIVE_CELL : DEAD_CELL;
            }
        }
    }

    // Clean up old state.
    if (state.current) {
        free(state.current);
    }
    if (state.next) {
        free(state.next);
    }

    // Update state.
    state.current = newCurrent;
    state.next = newNext;
    state.w = w;
    state.h = h;
}
EMSCRIPTEN_KEEPALIVE

/** Updates the state. */
void update_state() {
    for (uZ y = 0; y < state.h; y++) {
        for (uZ x = 0; x < state.w; x++) {
            const uZ leftX = (x > 0) ? x - 1 : state.w - 1;
            const uZ rightX = (x < state.w - 1) ? x + 1 : 0;
            const uZ downY = (y < state.h - 1) ? y + 1 : 0;
            const uZ upY = (y > 0) ? y - 1 : state.h - 1;
            const uZ i = (y * state.w) + x;
            const bool isAlive = is_cell_alive_i(i);
            u32 nLiveNeighbors = 0;

            // Count live neighbors.
            nLiveNeighbors += (u32)is_cell_alive_xy(leftX, upY);
            nLiveNeighbors += (u32)is_cell_alive_xy(x, upY);
            nLiveNeighbors += (u32)is_cell_alive_xy(rightX, upY);
            nLiveNeighbors += (u32)is_cell_alive_xy(leftX, y);
            nLiveNeighbors += (u32)is_cell_alive_xy(rightX, y);
            nLiveNeighbors += (u32)is_cell_alive_xy(leftX, downY);
            nLiveNeighbors += (u32)is_cell_alive_xy(x, downY);
            nLiveNeighbors += (u32)is_cell_alive_xy(rightX, downY);

            // Tell whether the cell will be alive or dead.
            const bool willLive = (isAlive)
                ? nLiveNeighbors >= 2 && nLiveNeighbors <= 3
                : nLiveNeighbors == 3;
            if (willLive) {
                state.next[i] = LIVE_CELL;
            } else {
                // Decay color in dead cells.
                Pixel p = state.current[i];
                p.b -= !!p.b;
                state.next[i] = p;
            }
        }
    }

    // Swap buffers.
    Pixel* temp = state.current;
    state.current = state.next;
    state.next = temp;
}
