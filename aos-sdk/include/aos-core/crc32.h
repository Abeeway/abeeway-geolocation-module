/*
 * \file crc32.h
 *
 * \brief Calculate the standard CRC32
 *
 */

/*!
 * \brief crc32 - Calculate the CRC32
 *
 *
 * \param crc Partial CRC32. Should be initialized to 0 the first time.
 *
 *
 * \param buf Buffer containing the data
 * \param len Buffer length
 *
 * \return The CRC32.
 *
 * \note This function supports partial CRC32: If you wish to perform the CRC32
 * on several data blocks, then the first call should have the crc parameter set to 0,
 * then you should pass the returned function value along this parameter for subsequent calls.
 * For a single block CRC, the crc parameter should also be be initialized to 0.
 */
uint32_t crc32(uint32_t crc, const uint8_t *buf, uint32_t len);
