/*
 * Copyright (c) 2019, NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT fsl_imx6sx_lcdif

#include <devicetree.h>
#include <drivers/display.h>
#include <fsl_elcdif.h>
#include <dt-bindings/display/elcdif.h>

#ifdef CONFIG_HAS_MCUX_CACHE
#include <fsl_cache.h>
#endif

#include <logging/log.h>

LOG_MODULE_REGISTER(display_mcux_elcdif, CONFIG_DISPLAY_LOG_LEVEL);

K_HEAP_DEFINE(mcux_elcdif_pool,
	      CONFIG_MCUX_ELCDIF_POOL_BLOCK_MAX *
	      CONFIG_MCUX_ELCDIF_POOL_BLOCK_NUM);

struct mcux_elcdif_config {
	LCDIF_Type *base;
	void (*irq_config_func)(const struct device *dev);
	elcdif_rgb_mode_config_t rgb_mode;
	enum display_pixel_format pixel_format;
	uint8_t bits_per_pixel;
};

struct mcux_mem_block {
	void *data;
};

struct mcux_elcdif_data {
	struct mcux_mem_block fb[2];
	struct k_sem sem;
	size_t pixel_bytes;
	size_t fb_bytes;
	uint8_t write_idx;
};

static int mcux_elcdif_write(const struct device *dev, const uint16_t x,
			     const uint16_t y,
			     const struct display_buffer_descriptor *desc,
			     const void *buf)
{
	const struct mcux_elcdif_config *config = dev->config;
	struct mcux_elcdif_data *data = dev->data;

	uint8_t write_idx = data->write_idx;
	uint8_t read_idx = !write_idx;

	int h_idx;
	const uint8_t *src;
	uint8_t *dst;

	__ASSERT((data->pixel_bytes * desc->pitch * desc->height) <=
		 desc->buf_size, "Input buffer too small");

	LOG_DBG("W=%d, H=%d, @%d,%d", desc->width, desc->height, x, y);

	k_sem_take(&data->sem, K_FOREVER);

	memcpy(data->fb[write_idx].data, data->fb[read_idx].data,
	       data->fb_bytes);

	src = buf;
	dst = data->fb[data->write_idx].data;
	dst += data->pixel_bytes * (y * config->rgb_mode.panelWidth + x);

	for (h_idx = 0; h_idx < desc->height; h_idx++) {
		memcpy(dst, src, data->pixel_bytes * desc->width);
		src += data->pixel_bytes * desc->pitch;
		dst += data->pixel_bytes * config->rgb_mode.panelWidth;
	}

#ifdef CONFIG_HAS_MCUX_CACHE
	DCACHE_CleanByRange((uint32_t) data->fb[write_idx].data,
			    data->fb_bytes);
#endif

	ELCDIF_SetNextBufferAddr(config->base,
				 (uint32_t) data->fb[write_idx].data);

	data->write_idx = read_idx;

	return 0;
}

static int mcux_elcdif_read(const struct device *dev, const uint16_t x,
			    const uint16_t y,
			    const struct display_buffer_descriptor *desc,
			    void *buf)
{
	LOG_ERR("Read not implemented");
	return -ENOTSUP;
}

static void *mcux_elcdif_get_framebuffer(const struct device *dev)
{
	LOG_ERR("Direct framebuffer access not implemented");
	return NULL;
}

static int mcux_elcdif_display_blanking_off(const struct device *dev)
{
	LOG_ERR("Display blanking control not implemented");
	return -ENOTSUP;
}

static int mcux_elcdif_display_blanking_on(const struct device *dev)
{
	LOG_ERR("Display blanking control not implemented");
	return -ENOTSUP;
}

static int mcux_elcdif_set_brightness(const struct device *dev,
				      const uint8_t brightness)
{
	LOG_WRN("Set brightness not implemented");
	return -ENOTSUP;
}

static int mcux_elcdif_set_contrast(const struct device *dev,
				    const uint8_t contrast)
{
	LOG_ERR("Set contrast not implemented");
	return -ENOTSUP;
}

static int mcux_elcdif_set_pixel_format(const struct device *dev,
					const enum display_pixel_format
					pixel_format)
{
	const struct mcux_elcdif_config *config = dev->config;

	if (pixel_format == config->pixel_format) {
		return 0;
	}
	LOG_ERR("Pixel format change not implemented");
	return -ENOTSUP;
}

static int mcux_elcdif_set_orientation(const struct device *dev,
		const enum display_orientation orientation)
{
	if (orientation == DISPLAY_ORIENTATION_NORMAL) {
		return 0;
	}
	LOG_ERR("Changing display orientation not implemented");
	return -ENOTSUP;
}

static void mcux_elcdif_get_capabilities(const struct device *dev,
		struct display_capabilities *capabilities)
{
	const struct mcux_elcdif_config *config = dev->config;

	memset(capabilities, 0, sizeof(struct display_capabilities));
	capabilities->x_resolution = config->rgb_mode.panelWidth;
	capabilities->y_resolution = config->rgb_mode.panelHeight;
	capabilities->supported_pixel_formats = config->pixel_format;
	capabilities->current_pixel_format = config->pixel_format;
	capabilities->current_orientation = DISPLAY_ORIENTATION_NORMAL;
}

static void mcux_elcdif_isr(const struct device *dev)
{
	const struct mcux_elcdif_config *config = dev->config;
	struct mcux_elcdif_data *data = dev->data;
	uint32_t status;

	status = ELCDIF_GetInterruptStatus(config->base);
	ELCDIF_ClearInterruptStatus(config->base, status);

	k_sem_give(&data->sem);
}

static int mcux_elcdif_init(const struct device *dev)
{
	const struct mcux_elcdif_config *config = dev->config;
	struct mcux_elcdif_data *data = dev->data;
	int i;

	elcdif_rgb_mode_config_t rgb_mode = config->rgb_mode;

	data->pixel_bytes = config->bits_per_pixel / 8U;
	data->fb_bytes = data->pixel_bytes *
			 rgb_mode.panelWidth * rgb_mode.panelHeight;
	data->write_idx = 1U;

	for (i = 0; i < ARRAY_SIZE(data->fb); i++) {
		data->fb[i].data = k_heap_alloc(&mcux_elcdif_pool,
						data->fb_bytes, K_NO_WAIT);
		if (data->fb[i].data == NULL) {
			LOG_ERR("Could not allocate frame buffer %d", i);
			return -ENOMEM;
		}
		memset(data->fb[i].data, 0, data->fb_bytes);
	}
	rgb_mode.bufferAddr = (uint32_t) data->fb[0].data;

	k_sem_init(&data->sem, 1, 1);

	config->irq_config_func(dev);

	ELCDIF_RgbModeInit(config->base, &rgb_mode);
	ELCDIF_EnableInterrupts(config->base,
				kELCDIF_CurFrameDoneInterruptEnable);
	ELCDIF_RgbModeStart(config->base);

	return 0;
}

static const struct display_driver_api mcux_elcdif_api = {
	.blanking_on = mcux_elcdif_display_blanking_on,
	.blanking_off = mcux_elcdif_display_blanking_off,
	.write = mcux_elcdif_write,
	.read = mcux_elcdif_read,
	.get_framebuffer = mcux_elcdif_get_framebuffer,
	.set_brightness = mcux_elcdif_set_brightness,
	.set_contrast = mcux_elcdif_set_contrast,
	.get_capabilities = mcux_elcdif_get_capabilities,
	.set_pixel_format = mcux_elcdif_set_pixel_format,
	.set_orientation = mcux_elcdif_set_orientation,
};

static void mcux_elcdif_config_func_1(const struct device *dev);

#define PANEL DT_GPARENT(DT_NODELABEL(lcd_panel_in))
/*#define TIMING DT_CHILD(DT_CHILD(PANEL, display_timings), timing)*/
#define PANEL_DATA_BUS_WIDTH_HELPER(x) kELCDIF_DataBus ## x ## Bit
#define PANEL_DATA_BUS_WIDTH(x) PANEL_DATA_BUS_WIDTH_HELPER(x)

#if DT_PROP(PANEL, de_active_low) == 1
	#define PANEL_DE_ACTIVE kELCDIF_DataEnableActiveLow
#else
	#define PANEL_DE_ACTIVE kELCDIF_DataEnableActiveHigh
#endif

#if DT_PROP(PANEL, vsync_active_high) == 1
	#define PANEL_VSYNC_ACTIVE kELCDIF_VsyncActiveHigh
#else
	#define PANEL_VSYNC_ACTIVE kELCDIF_VsyncActiveLow
#endif

#if DT_PROP(PANEL, hsync_active_high) == 1
	#define PANEL_HSYNC_ACTIVE kELCDIF_HsyncActiveHigh
#else
	#define PANEL_HSYNC_ACTIVE kELCDIF_HsyncActiveLow
#endif

#if DT_PROP(PANEL, pixelclk_falling) == 1
	#define PANEL_PIXELCLK_DATADRIVE kELCDIF_DriveDataOnFallingClkEdge
#else
	#define PANEL_PIXELCLK_DATADRIVE kELCDIF_DriveDataOnRisingClkEdge
#endif

#if DT_PROP(PANEL, pixel_format) == MCUX_PIXEL_FORMAT_RAW8
	#define PANEL_PIXEL_FORMAT_1 kELCDIF_PixelFormatRAW8
	#define PANEL_PIXEL_FORMAT_2 PIXEL_FORMAT_MONO01
#elif DT_PROP(PANEL, pixel_format) == MCUX_PIXEL_FORMAT_RGB666
	#define PANEL_PIXEL_FORMAT_1 kELCDIF_PixelFormatRGB666
	#define PANEL_PIXEL_FORMAT_2 PIXEL_FORMAT_RGB_888
#elif DT_PROP(PANEL, pixel_format) == MCUX_PIXEL_FORMAT_XRGB8888
	#define PANEL_PIXEL_FORMAT_1 kELCDIF_PixelFormatXRGB8888
	#define PANEL_PIXEL_FORMAT_2 PIXEL_FORMAT_ARGB_8888
#elif DT_PROP(PANEL, pixel_format) == MCUX_PIXEL_FORMAT_RGB888
	#define PANEL_PIXEL_FORMAT_1 kELCDIF_PixelFormatRGB888
	#define PANEL_PIXEL_FORMAT_2 PIXEL_FORMAT_RGB_888
#else /* if DT_PROP(PANEL, pixel_format) == MCUX_PIXEL_FORMAT_RGB565 */
	#define PANEL_PIXEL_FORMAT_1 kELCDIF_PixelFormatRGB565
	#define PANEL_PIXEL_FORMAT_2 PIXEL_FORMAT_RGB_565
#endif

static struct mcux_elcdif_config mcux_elcdif_config_1 = {
	.base = (LCDIF_Type *) DT_INST_REG_ADDR(0),
	.irq_config_func = mcux_elcdif_config_func_1,
	.rgb_mode = {
		.panelWidth = DT_PROP(PANEL, hactive),
		.panelHeight = DT_PROP(PANEL, vactive),
		.hsw = DT_PROP(PANEL, hsync_len),
		.hfp = DT_PROP(PANEL, hfront_porch),
		.hbp = DT_PROP(PANEL, hback_porch),
		.vsw = DT_PROP(PANEL, vsync_len),
		.vfp = DT_PROP(PANEL, vfront_porch),
		.vbp = DT_PROP(PANEL, vback_porch),
		.polarityFlags = PANEL_DE_ACTIVE |
				 PANEL_VSYNC_ACTIVE |
				 PANEL_HSYNC_ACTIVE |
				 PANEL_PIXELCLK_DATADRIVE,
		.pixelFormat = PANEL_PIXEL_FORMAT_1,
		.dataBus = PANEL_DATA_BUS_WIDTH(DT_PROP(PANEL, bus_width)),
	},
	.pixel_format = PANEL_PIXEL_FORMAT_2,
	.bits_per_pixel = DT_PROP(PANEL, bits_per_pixel),
};

static struct mcux_elcdif_data mcux_elcdif_data_1;

DEVICE_DT_INST_DEFINE(0,
		    &mcux_elcdif_init,
		    device_pm_control_nop,
		    &mcux_elcdif_data_1, &mcux_elcdif_config_1,
		    POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEVICE,
		    &mcux_elcdif_api);

static void mcux_elcdif_config_func_1(const struct device *dev)
{
	IRQ_CONNECT(DT_INST_IRQN(0),
		    DT_INST_IRQ(0, priority),
		    mcux_elcdif_isr, DEVICE_DT_INST_GET(0), 0);

	irq_enable(DT_INST_IRQN(0));
}
