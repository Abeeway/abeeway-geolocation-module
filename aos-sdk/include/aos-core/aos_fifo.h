/*!
 * \file aos_fifo.h
 *
 * \brief Basic FIFO (First-in/first-out) buffer functionality
 *
 * Copyright (C) 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 *
 */
#pragma once
#include <stdint.h>
#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif


/*!
 * \defgroup aos_fifo FIFO facility
 *
 * \addtogroup aos_fifo
 * @{
 */

/*!
 * \struct aos_fifo_t
 *
 * \brief FIFO structure
 */
typedef struct {
	uint8_t *buf;		//!< FIFO buffer storage
	int head;			//!< Head of the FIFO
	int tail;			//!< Tail of the FIFO
	int size;			//!< Size of the FIFO buffer
} aos_fifo_t;

/*!
 * \fn void aos_fifo_init(aos_fifo_t * f, uint8_t * buf, int size)
 *
 * \brief Prepare a FIFO for use.
 * This function initializes an aos_fifo_t for use by setting up the storage
 * buffer, buffer size, and internal head / tail pointers.
 *
 * \param f	Address of a aos_fifo_t structure to use
 * \param buf Address of a user provided buffer to use for FIFO storage
 * \param size Size of the user provided buffer
 */
void aos_fifo_init(aos_fifo_t * f, uint8_t * buf, int size);

/*!
 * \fn bool aos_fifo_is_full(aos_fifo_t *f)
 *
 * \brief Test whether a FIFO is full.
 *
 * \param f	Address of a aos_fifo_t structure to use
 *
 * \return This function returns true if the FIFO is full.
 */
bool aos_fifo_is_full(aos_fifo_t *f);

/*!
 * \fn bool aos_fifo_is_empty(aos_fifo_t *f)
 *
 * \brief Test whether a FIFO is empty.
 *
 * \param f	Address of a aos_fifo_t structure to use
 *
 * \return This function returns true if the FIFO is empty.
 */
bool aos_fifo_is_empty(aos_fifo_t *f);

/*!
 * \fn void aos_fifo_flush(aos_fifo_t *f)
 *
 * \brief Flushes out a FIFO, discarding any data within.
 *
 * \param f	Address of a aos_fifo_t structure to use
 */
void aos_fifo_flush(aos_fifo_t *f);

/*!
 * \fn int aos_fifo_level(aos_fifo_t *f)
 *
 * \brief Test how many bytes are currently in the FIFO.
 *
 * \param f	Address of a aos_fifo_t structure to use
 *
 * \return The number of bytes currently in the FIFO.
 */
int aos_fifo_level(aos_fifo_t *f);

/*!
 * \fn int aos_fifo_free_size(aos_fifo_t *f)
 *
 * \brief Test how many bytes are free in the FIFO.
 *
 * \param f	Address of a aos_fifo_t structure to use
 *
 * \return The number of bytes free in the FIFO.
 */
int aos_fifo_free_size(aos_fifo_t *f);

/*!
 * \fn int aos_fifo_read(aos_fifo_t *f, void *retbuf, int nbytes)
 *
 * \brief Read data from a FIFO.
 * This function reads data from a FIFO into a user provided buffer until
 * either the specified number of bytes has been reached, or the FIFO is
 * empty.
 *
 * \param f	Address of a aos_fifo_t structure to use
 * \param retbuf Address of a buffer to write FIFO data to.
 * \param nbytes Number of bytes to read from the FIFO
 *
 * \return This function returns the number of bytes actually copied into
 *the retbuf buffer.
 */
int aos_fifo_read(aos_fifo_t *f, void *retbuf, int nbytes);

/*!
 * \fn int aos_fifo_write(aos_fifo_t * f, const void *buf, int nbytes)
 *
 * \brief Write data to a FIFO.
 * This function writes data from a user buffer to a FIFO until
 * either the specified number of bytes has been reached, or the FIFO is
 * full.
 *
 * \param f	Address of a aos_fifo_t structure to use
 * \param buf Address of a buffer to copy to the FIFO..
 * \param nbytes Number of bytes to write to the FIFO
 *
 * \return This function returns the number of bytes actually written into
 *         the FIFO.
 */
int aos_fifo_write(aos_fifo_t * f, const void *buf, int nbytes);


/*! @}*/
#ifdef __cplusplus
}
#endif
