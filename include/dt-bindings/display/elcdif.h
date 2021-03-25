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

/* eLCDIF clock source and dividers */
#define MCUX_ELCDIF_PRE_CLK_SRC_PLL2 0      /*!< Set LCDIF root clock pre-multipler to PLL2 */
#define MCUX_ELCDIF_PRE_CLK_SRC_PLL3_PFD3 1 /*!< Set LCDIF root clock pre-multipler to PLL3_PFD3 */
#define MCUX_ELCDIF_PRE_CLK_SRC_PLL5 2      /*!< Set LCDIF root clock pre-multipler to PLL5 */
#define MCUX_ELCDIF_PRE_CLK_SRC_PLL2_PFD0 3 /*!< Set LCDIF root clock pre-multipler to PLL2_PFD0 */
#define MCUX_ELCDIF_PRE_CLK_SRC_PLL2_PFD1 4 /*!< Set LCDIF root clock pre-multipler to PLL2_PFD1 */
#define MCUX_ELCDIF_PRE_CLK_SRC_PLL3_PFD1 5 /*!< Set LCDIF root clock pre-multipler to PLL3_PFD1 */

/* Divisors */
#define MCUX_ELCDIF_DIVIDE_BY_1 0 /*!< Divide by 1 */
#define MCUX_ELCDIF_DIVIDE_BY_2 1 /*!< Divide by 2 */
#define MCUX_ELCDIF_DIVIDE_BY_3 2 /*!< Divide by 3 */
#define MCUX_ELCDIF_DIVIDE_BY_4 3 /*!< Divide by 4 */
#define MCUX_ELCDIF_DIVIDE_BY_5 4 /*!< Divide by 5 */
#define MCUX_ELCDIF_DIVIDE_BY_6 5 /*!< Divide by 6 */
#define MCUX_ELCDIF_DIVIDE_BY_7 6 /*!< Divide by 7 */
#define MCUX_ELCDIF_DIVIDE_BY_8 7 /*!< Divide by 8 */

#endif /* ZEPHYR_INCLUDE_DT_BINDINGS_DISPLAY_ELCDIF_H_ */
