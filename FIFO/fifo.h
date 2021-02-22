


#ifndef __FIFO_HEAD_H
#define __FIFO_HEAD_H

#include <string.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \defgroup        FIFO Lightweight ring buffer manager
 * \brief           Lightweight ring buffer manager
 * \{
 */

/**
 * \brief           Enable buffer structure pointer parameter as volatile
 * To use this feature, uncomment keyword below
 */
#define FIFO_VOLATILE                           /* volatile */

/**
 * \brief           Adds 2 magic words to make sure if memory is corrupted
 *                  application can detect wrong data pointer and maximum size
 */
#define FIFO_USE_MAGIC                      1

/**
 * \brief           Event type for buffer operations
 */
typedef enum {
    FIFO_EVT_READ,                              /*!< Read event */
    FIFO_EVT_WRITE,                             /*!< Write event */
    FIFO_EVT_RESET,                             /*!< Reset event */
} fifo_evt_type_t;

/**
 * \brief           Buffer structure forward declaration
 */
struct fifo;

/**
 * \brief           Event callback function type
 * \param[in]       buff: Buffer handle for event
 * \param[in]       evt: Event type
 * \param[in]       bp: Number of bytes written or read (when used), depends on event type
 */
typedef void (*fifo_evt_fn)(FIFO_VOLATILE struct fifo* buff, fifo_evt_type_t evt, size_t bp);

/**
 * \brief           Buffer structure
 */
typedef struct fifo {
#if FIFO_USE_MAGIC
    uint32_t magic1;                            /*!< Magic 1 word */
#endif /* FIFO_USE_MAGIC */
    uint8_t* buff;                              /*!< Pointer to buffer data.
                                                    Buffer is considered initialized when `buff != NULL` and `size > 0` */
    size_t size;                                /*!< Size of buffer data. Size of actual buffer is `1` byte less than value holds */
    size_t r;                                   /*!< Next read pointer. Buffer is considered empty when `r == w` and full when `w == r - 1` */
    size_t w;                                   /*!< Next write pointer. Buffer is considered empty when `r == w` and full when `w == r - 1` */
    fifo_evt_fn evt_fn;                         /*!< Pointer to event callback function */
#if FIFO_USE_MAGIC
    uint32_t magic2;                            /*!< Magic 2 word */
#endif /* FIFO_USE_MAGIC */
} fifo_t;

uint8_t     fifo_init(FIFO_VOLATILE fifo_t* buff, void* buffdata, size_t size);
uint8_t     fifo_is_ready(FIFO_VOLATILE fifo_t* buff);
void        fifo_free(FIFO_VOLATILE fifo_t* buff);
void        fifo_reset(FIFO_VOLATILE fifo_t* buff);
void        fifo_set_evt_fn(FIFO_VOLATILE fifo_t* buff, fifo_evt_fn fn);

/* Read/Write functions */
size_t      fifo_write(FIFO_VOLATILE fifo_t* buff, const void* data, size_t btw);
size_t      fifo_read(FIFO_VOLATILE fifo_t* buff, void* data, size_t btr);
size_t      fifo_peek(FIFO_VOLATILE fifo_t* buff, size_t skip_count, void* data, size_t btp);

/* Buffer size information */
size_t      fifo_get_free(FIFO_VOLATILE fifo_t* buff);
size_t      fifo_get_full(FIFO_VOLATILE fifo_t* buff);

/* Read data block management */
void*       fifo_get_linear_block_read_address(FIFO_VOLATILE fifo_t* buff);
size_t      fifo_get_linear_block_read_length(FIFO_VOLATILE fifo_t* buff);
size_t      fifo_skip(FIFO_VOLATILE fifo_t* buff, size_t len);

/* Write data block management */
void*       fifo_get_linear_block_write_address(FIFO_VOLATILE fifo_t* buff);
size_t      fifo_get_linear_block_write_length(FIFO_VOLATILE fifo_t* buff);
size_t      fifo_advance(FIFO_VOLATILE fifo_t* buff, size_t len);

/**
 * \}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* FIFO_HDR_H */
