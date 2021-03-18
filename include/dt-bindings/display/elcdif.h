/*
 * Copyright (c) 2020 Teslabs Engineering S.L.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ZEPHYR_INCLUDE_DT_BINDINGS_DISPLAY_ELCDIF_H_
#define ZEPHYR_INCLUDE_DT_BINDINGS_DISPLAY_ELCDIF_H_

/* Pixel formats */
#define MCUX_PIXEL_FORMAT_RAW8 0     /*!< RAW 8 bit, four data use 32 bits. */
#define MCUX_PIXEL_FORMAT_RGB565 1   /*!< RGB565, two pixel use 32 bits. */
#define MCUX_PIXEL_FORMAT_BGR565 2   /*!< RGB565, two pixel use 32 bits. */
#define MCUX_PIXEL_FORMAT_RGB666 3   /*!< RGB666 unpacked, one pixel uses 32 bits, high byte unused, upper 2 bits of other bytes unused. */
#define MCUX_PIXEL_FORMAT_XRGB8888 4 /*!< XRGB8888 unpacked, one pixel uses 32 bits, high byte unused. */
#define MCUX_PIXEL_FORMAT_RGB888 5   /*!< RGB888 packed, one pixel uses 24 bits. */

#endif /* ZEPHYR_INCLUDE_DT_BINDINGS_DISPLAY_ELCDIF_H_ */
