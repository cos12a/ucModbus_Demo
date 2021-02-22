


#include "fifo.h"

/* Memory set and copy functions */
#define BUF_MEMSET                      memset
#define BUF_MEMCPY                      memcpy

#define BUF_MAGIC1                      (0xDEADBEEF)
#define BUF_MAGIC2                      (~0xDEADBEEF)

#if FIFO_USE_MAGIC
#define BUF_IS_VALID(b)                 ((b) != NULL && (b)->magic1 == BUF_MAGIC1 && (b)->magic2 == BUF_MAGIC2 && (b)->buff != NULL && (b)->size > 0)
#else
#define BUF_IS_VALID(b)                 ((b) != NULL && (b)->buff != NULL && (b)->size > 0)
#endif /* FIFO_USE_MAGIC */
#define BUF_MIN(x, y)                   ((x) < (y) ? (x) : (y))
#define BUF_MAX(x, y)                   ((x) > (y) ? (x) : (y))
#define BUF_SEND_EVT(b, type, bp)       do { if ((b)->evt_fn != NULL) { (b)->evt_fn((b), (type), (bp)); } } while (0)

/**
 * \brief           Initialize buffer handle to default values with size and buffer data array
 * \param[in]       buff: Buffer handle
 * \param[in]       buffdata: Pointer to memory to use as buffer data
 * \param[in]       size: Size of `buffdata` in units of bytes
 *                      Maximum number of bytes buffer can hold is `size - 1`
 * \return          `1` on success, `0` otherwise
 */
uint8_t
fifo_init(FIFO_VOLATILE fifo_t* buff, void* buffdata, size_t size) {
    if (buff == NULL || buffdata == NULL || size == 0) {
        return 0;
    }

    BUF_MEMSET((void*)buff, 0x00, sizeof(*buff));

    buff->size = size;
    buff->buff = buffdata;

#if FIFO_USE_MAGIC
    buff->magic1 = BUF_MAGIC1;
    buff->magic2 = BUF_MAGIC2;
#endif /* FIFO_USE_MAGIC */

    return 1;
}

/**
 * \brief           Check if buff is initialized and ready to use
 * \param[in]       buff: Buffer handle
 * \return          `1` if ready, `0` otherwise
 */
uint8_t
fifo_is_ready(FIFO_VOLATILE fifo_t* buff) {
    return BUF_IS_VALID(buff);
}

/**
 * \brief           Free buffer memory
 * \note            Since implementation does not use dynamic allocation,
 *                  it just sets buffer handle to `NULL`
 * \param[in]       buff: Buffer handle
 */
void
fifo_free(FIFO_VOLATILE fifo_t* buff) {
    if (BUF_IS_VALID(buff)) {
        buff->buff = NULL;
    }
}

/**
 * \brief           Set event function callback for different buffer operations
 * \param[in]       buff: Buffer handle
 * \param[in]       evt_fn: Callback function
 */
void
fifo_set_evt_fn(FIFO_VOLATILE fifo_t* buff, fifo_evt_fn evt_fn) {
    if (BUF_IS_VALID(buff)) {
        buff->evt_fn = evt_fn;
    }
}

/**
 * \brief           Write data to buffer.
 * Copies data from `data` array to buffer and marks buffer as full for maximum `btw` number of bytes
 *
 * \param[in]       buff: Buffer handle
 * \param[in]       data: Pointer to data to write into buffer
 * \param[in]       btw: Number of bytes to write
 * \return          Number of bytes written to buffer.
 *                      When returned value is less than `btw`, there was no enough memory available
 *                      to copy full data array
 */
size_t
fifo_write(FIFO_VOLATILE fifo_t* buff, const void* data, size_t btw) {
    size_t tocopy, free;
    const uint8_t* d = data;

    if (!BUF_IS_VALID(buff) || data == NULL || btw == 0) {
        return 0;
    }

    /* Calculate maximum number of bytes available to write */
    free = fifo_get_free(buff);
    btw = BUF_MIN(free, btw);
    if (btw == 0) {
        return 0;
    }

    /* Step 1: Write data to linear part of buffer */
    tocopy = BUF_MIN(buff->size - buff->w, btw);
    BUF_MEMCPY(&buff->buff[buff->w], d, tocopy);
    buff->w += tocopy;
    btw -= tocopy;

    /* Step 2: Write data to beginning of buffer (overflow part) */
    if (btw > 0) {
        BUF_MEMCPY(buff->buff, &d[tocopy], btw);
        buff->w = btw;
    }

    /* Step 3: Check end of buffer */
    if (buff->w >= buff->size) {
        buff->w = 0;
    }
    BUF_SEND_EVT(buff, FIFO_EVT_WRITE, tocopy + btw);
    return tocopy + btw;
}

/**
 * \brief           Read data from buffer.
 * Copies data from buffer to `data` array and marks buffer as free for maximum `btr` number of bytes
 *
 * \param[in]       buff: Buffer handle
 * \param[out]      data: Pointer to output memory to copy buffer data to
 * \param[in]       btr: Number of bytes to read
 * \return          Number of bytes read and copied to data array
 */
size_t
fifo_read(FIFO_VOLATILE fifo_t* buff, void* data, size_t btr) {
    size_t tocopy, full;
    uint8_t* d = data;

    if (!BUF_IS_VALID(buff) || data == NULL || btr == 0) {
        return 0;
    }

    /* Calculate maximum number of bytes available to read */
    full = fifo_get_full(buff);
    btr = BUF_MIN(full, btr);
    if (btr == 0) {
        return 0;
    }

    /* Step 1: Read data from linear part of buffer */
    tocopy = BUF_MIN(buff->size - buff->r, btr);
    BUF_MEMCPY(d, &buff->buff[buff->r], tocopy);
    buff->r += tocopy;
    btr -= tocopy;

    /* Step 2: Read data from beginning of buffer (overflow part) */
    if (btr > 0) {
        BUF_MEMCPY(&d[tocopy], buff->buff, btr);
        buff->r = btr;
    }

    /* Step 3: Check end of buffer */
    if (buff->r >= buff->size) {
        buff->r = 0;
    }
    BUF_SEND_EVT(buff, FIFO_EVT_READ, tocopy + btr);
    return tocopy + btr;
}

/**
 * \brief           Read from buffer without changing read pointer (peek only)
 * \param[in]       buff: Buffer handle
 * \param[in]       skip_count: Number of bytes to skip before reading data
 * \param[out]      data: Pointer to output memory to copy buffer data to
 * \param[in]       btp: Number of bytes to peek
 * \return          Number of bytes peeked and written to output array
 */
size_t
fifo_peek(FIFO_VOLATILE fifo_t* buff, size_t skip_count, void* data, size_t btp) {
    size_t full, tocopy, r;
    uint8_t* d = data;

    if (!BUF_IS_VALID(buff) || data == NULL || btp == 0) {
        return 0;
    }

    r = buff->r;

    /* Calculate maximum number of bytes available to read */
    full = fifo_get_full(buff);

    /* Skip beginning of buffer */
    if (skip_count >= full) {
        return 0;
    }
    r += skip_count;
    full -= skip_count;
    if (r >= buff->size) {
        r -= buff->size;
    }

    /* Check maximum number of bytes available to read after skip */
    btp = BUF_MIN(full, btp);
    if (btp == 0) {
        return 0;
    }

    /* Step 1: Read data from linear part of buffer */
    tocopy = BUF_MIN(buff->size - r, btp);
    BUF_MEMCPY(d, &buff->buff[r], tocopy);
    btp -= tocopy;

    /* Step 2: Read data from beginning of buffer (overflow part) */
    if (btp > 0) {
        BUF_MEMCPY(&d[tocopy], buff->buff, btp);
    }
    return tocopy + btp;
}

/**
 * \brief           Get available size in buffer for write operation
 * \param[in]       buff: Buffer handle
 * \return          Number of free bytes in memory
 */
size_t
fifo_get_free(FIFO_VOLATILE fifo_t* buff) {
    size_t size, w, r;

    if (!BUF_IS_VALID(buff)) {
        return 0;
    }

    /* Use temporary values in case they are changed during operations */
    w = buff->w;
    r = buff->r;
    if (w == r) {
        size = buff->size;
    } else if (r > w) {
        size = r - w;
    } else {
        size = buff->size - (w - r);
    }

    /* Buffer free size is always 1 less than actual size */
    return size - 1;
}

/**
 * \brief           Get number of bytes currently available in buffer
 * \param[in]       buff: Buffer handle
 * \return          Number of bytes ready to be read
 */
size_t
fifo_get_full(FIFO_VOLATILE fifo_t* buff) {
    size_t w, r, size;

    if (!BUF_IS_VALID(buff)) {
        return 0;
    }

    /* Use temporary values in case they are changed during operations */
    w = buff->w;
    r = buff->r;
    if (w == r) {
        size = 0;
    } else if (w > r) {
        size = w - r;
    } else {
        size = buff->size - (r - w);
    }
    return size;
}

/**
 * \brief           Resets buffer to default values. Buffer size is not modified
 * \param[in]       buff: Buffer handle
 */
void
fifo_reset(FIFO_VOLATILE fifo_t* buff) {
    if (BUF_IS_VALID(buff)) {
        buff->w = 0;
        buff->r = 0;
        BUF_SEND_EVT(buff, FIFO_EVT_RESET, 0);
    }
}

/**
 * \brief           Get linear address for buffer for fast read
 * \param[in]       buff: Buffer handle
 * \return          Linear buffer start address
 */
void*
fifo_get_linear_block_read_address(FIFO_VOLATILE fifo_t* buff) {
    if (!BUF_IS_VALID(buff)) {
        return NULL;
    }
    return &buff->buff[buff->r];
}

/**
 * \brief           Get length of linear block address before it overflows for read operation
 * \param[in]       buff: Buffer handle
 * \return          Linear buffer size in units of bytes for read operation
 */
size_t
fifo_get_linear_block_read_length(FIFO_VOLATILE fifo_t* buff) {
    size_t w, r, len;

    if (!BUF_IS_VALID(buff)) {
        return 0;
    }

    /* Use temporary values in case they are changed during operations */
    w = buff->w;
    r = buff->r;
    if (w > r) {
        len = w - r;
    } else if (r > w) {
        len = buff->size - r;
    } else {
        len = 0;
    }
    return len;
}

/**
 * \brief           Skip (ignore; advance read pointer) buffer data
 * Marks data as read in the buffer and increases free memory for up to `len` bytes
 *
 * \note            Useful at the end of streaming transfer such as DMA
 * \param[in]       buff: Buffer handle
 * \param[in]       len: Number of bytes to skip and mark as read
 * \return          Number of bytes skipped
 */
size_t
fifo_skip(FIFO_VOLATILE fifo_t* buff, size_t len) {
    size_t full;

    if (!BUF_IS_VALID(buff) || len == 0) {
        return 0;
    }

    full = fifo_get_full(buff);
    len = BUF_MIN(len, full);
    buff->r += len;
    if (buff->r >= buff->size) {
        buff->r -= buff->size;
    }
    BUF_SEND_EVT(buff, FIFO_EVT_READ, len);
    return len;
}

/**
 * \brief           Get linear address for buffer for fast read
 * \param[in]       buff: Buffer handle
 * \return          Linear buffer start address
 */
void*
fifo_get_linear_block_write_address(FIFO_VOLATILE fifo_t* buff) {
    if (!BUF_IS_VALID(buff)) {
        return NULL;
    }
    return &buff->buff[buff->w];
}

/**
 * \brief           Get length of linear block address before it overflows for write operation
 * \param[in]       buff: Buffer handle
 * \return          Linear buffer size in units of bytes for write operation
 */
size_t
fifo_get_linear_block_write_length(FIFO_VOLATILE fifo_t* buff) {
    size_t w, r, len;

    if (!BUF_IS_VALID(buff)) {
        return 0;
    }

    /* Use temporary values in case they are changed during operations */
    w = buff->w;
    r = buff->r;
    if (w >= r) {
        len = buff->size - w;
        /*
         * When read pointer is 0,
         * maximal length is one less as if too many bytes
         * are written, buffer would be considered empty again (r == w)
         */
        if (r == 0) {
            /*
             * Cannot overflow:
             * - If r is not 0, statement does not get called
             * - buff->size cannot be 0 and if r is 0, len is greater 0
             */
            --len;
        }
    } else {
        len = r - w - 1;
    }
    return len;
}

/**
 * \brief           Advance write pointer in the buffer.
 * Similar to skip function but modifies write pointer instead of read
 *
 * \note            Useful when hardware is writing to buffer and application needs to increase number
 *                      of bytes written to buffer by hardware
 * \param[in]       buff: Buffer handle
 * \param[in]       len: Number of bytes to advance
 * \return          Number of bytes advanced for write operation
 */
size_t
fifo_advance(FIFO_VOLATILE fifo_t* buff, size_t len) {
    size_t free;

    if (!BUF_IS_VALID(buff) || len == 0) {
        return 0;
    }

    free = fifo_get_free(buff);
    len = BUF_MIN(len, free);
    buff->w += len;
    if (buff->w >= buff->size) {
        buff->w -= buff->size;
    }
    BUF_SEND_EVT(buff, FIFO_EVT_WRITE, len);
    return len;
}
