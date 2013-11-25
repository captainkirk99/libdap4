/*
 * chunked_stream.h
 *
 *  Created on: Sep 15, 2013
 *      Author: jimg
 */

#ifndef CHUNK_STREAM_H_
#define CHUNK_STREAM_H_

#define CHUNK_DATA 0x00000000
#define CHUNK_END  0x01000000
#define CHUNK_ERR  0x02000000

#if !BYTE_ORDER_PREFIX
#define CHUNK_LITTLE_ENDIAN  0x04000000
#endif

#define CHUNK_TYPE_MASK 0xFF000000
#define CHUNK_SIZE_MASK 0x00FFFFFF

#define CHUNK_SIZE 4096;

#endif /* CHUNK_STREAM_H_ */
