/**
 * @file    ili9341.c
 * @brief   TFT LCD ILI9341 240x320, giao tiep SPI - implementation.
 *
 * @author  Khue Nguyen
 * @website khuenguyencreator.com
 */
#include "ili9341.h"
#include "glcdfont.h"

static SPI_HandleTypeDef *lcd_spi;

/* Bang lenh khoi tao: lenh, so byte tham so, cac byte tham so */
static const uint8_t init_cmds[] = {
	0xEF, 3, 0x03, 0x80, 0x02,
	0xCF, 3, 0x00, 0xC1, 0x30,
	0xED, 4, 0x64, 0x03, 0x12, 0x81,
	0xE8, 3, 0x85, 0x00, 0x78,
	0xCB, 5, 0x39, 0x2C, 0x00, 0x34, 0x02,
	0xF7, 1, 0x20,
	0xEA, 2, 0x00, 0x00,
	0xC0, 1, 0x23,             /* Power control 1 */
	0xC1, 1, 0x10,             /* Power control 2 */
	0xC5, 2, 0x3E, 0x28,       /* VCOM control 1 */
	0xC7, 1, 0x86,             /* VCOM control 2 */
	0x36, 1, 0x48,             /* Memory Access Control: doc, thu tu mau BGR */
	0x3A, 1, 0x55,             /* Pixel format: 16 bit/pixel (RGB565) */
	0xB1, 2, 0x00, 0x18,       /* Frame rate 79Hz */
	0xB6, 3, 0x08, 0x82, 0x27, /* Display function control */
	0xF2, 1, 0x00,             /* Tat 3Gamma */
	0x26, 1, 0x01,             /* Gamma curve */
	0xE0, 15, 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1,
	          0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00,
	0xE1, 15, 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1,
	          0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F,
	0x00                       /* ket thuc bang */
};

//************************* Low Level Layer *********************************************************/

static void ILI9341_WriteCommand(uint8_t cmd)
{
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET);   /* DC = 0: lenh */
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(lcd_spi, &cmd, 1, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

static void ILI9341_WriteData(const uint8_t *buff, uint16_t size)
{
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);     /* DC = 1: du lieu */
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(lcd_spi, (uint8_t *)buff, size, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

static void ILI9341_Reset(void)
{
	HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET);
	HAL_Delay(10);
	HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
	HAL_Delay(120);
}

//************************* High Level Layer *******************************************************/

void ILI9341_Init(SPI_HandleTypeDef *hspi)
{
	const uint8_t *p = init_cmds;
	uint8_t cmd, n;

	lcd_spi = hspi;
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
	ILI9341_Reset();

	ILI9341_WriteCommand(0x01);   /* Software reset */
	HAL_Delay(150);

	while ((cmd = *p++) != 0x00)
	{
		n = *p++;
		ILI9341_WriteCommand(cmd);
		ILI9341_WriteData(p, n);
		p += n;
	}

	ILI9341_WriteCommand(0x11);   /* Sleep out */
	HAL_Delay(120);
	ILI9341_WriteCommand(0x29);   /* Display on */
	HAL_Delay(20);
}

void ILI9341_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
	uint8_t data[4];

	ILI9341_WriteCommand(0x2A);   /* Column address set */
	data[0] = x0 >> 8; data[1] = x0 & 0xFF; data[2] = x1 >> 8; data[3] = x1 & 0xFF;
	ILI9341_WriteData(data, 4);

	ILI9341_WriteCommand(0x2B);   /* Page (row) address set */
	data[0] = y0 >> 8; data[1] = y0 & 0xFF; data[2] = y1 >> 8; data[3] = y1 & 0xFF;
	ILI9341_WriteData(data, 4);

	ILI9341_WriteCommand(0x2C);   /* Memory write */
}

void ILI9341_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
	uint8_t data[2] = {color >> 8, color & 0xFF};

	if (x >= ILI9341_WIDTH || y >= ILI9341_HEIGHT) return;
	ILI9341_SetAddressWindow(x, y, x, y);
	ILI9341_WriteData(data, 2);
}

void ILI9341_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
	uint8_t line[64];
	uint32_t total, chunk;
	uint16_t i;

	if (x >= ILI9341_WIDTH || y >= ILI9341_HEIGHT) return;
	if (x + w > ILI9341_WIDTH)  w = ILI9341_WIDTH - x;
	if (y + h > ILI9341_HEIGHT) h = ILI9341_HEIGHT - y;

	for (i = 0; i < sizeof(line); i += 2)
	{
		line[i] = color >> 8;
		line[i + 1] = color & 0xFF;
	}

	ILI9341_SetAddressWindow(x, y, x + w - 1, y + h - 1);
	total = (uint32_t)w * h * 2;   /* so byte can gui */
	while (total > 0)
	{
		chunk = (total > sizeof(line)) ? sizeof(line) : total;
		ILI9341_WriteData(line, chunk);
		total -= chunk;
	}
}

void ILI9341_FillScreen(uint16_t color)
{
	ILI9341_FillRect(0, 0, ILI9341_WIDTH, ILI9341_HEIGHT, color);
}

/* Ve 1 ky tu font 5x7, size la he so phong to (1 = 6x8 pixel) */
void ILI9341_DrawChar(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg, uint8_t size)
{
	uint8_t i, j, line;

	for (i = 0; i < 6; i++)
	{
		line = (i < 5) ? font[(uint8_t)c * 5 + i] : 0x00;   /* cot thu 6 de trong */
		for (j = 0; j < 8; j++, line >>= 1)
		{
			ILI9341_FillRect(x + i * size, y + j * size, size, size, (line & 0x01) ? color : bg);
		}
	}
}

void ILI9341_DrawString(uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bg, uint8_t size)
{
	while (*str)
	{
		if (x + 6 * size > ILI9341_WIDTH)   /* het dong thi xuong dong */
		{
			x = 0;
			y += 8 * size;
		}
		ILI9341_DrawChar(x, y, *str++, color, bg, size);
		x += 6 * size;
	}
}
