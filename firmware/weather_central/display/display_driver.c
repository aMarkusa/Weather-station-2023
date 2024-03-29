#include "display_driver.h"
#include "user_periph_setup.h"
#include "spi.h"
#include "systick.h"
#include "io_expander.h"
#include <stdio.h>

char font[FONT_ARRAY_LEN][CHAR_HEIGHT] = {
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+0020 (space)
		{ 0x00, 0xc6, 0xcc, 0x18, 0x30, 0x66, 0xc6, 0x00 },		// U+0025 (%)
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00 },		// U+002E (.)
		{ 0x7c, 0xc6, 0xce, 0xde, 0xf6, 0xe6, 0x7c, 0x00 },		// U+0030 (0)
		{ 0x30, 0x70, 0x30, 0x30, 0x30, 0x30, 0xfc, 0x00 },		// U+0031 (1)
		{ 0x78, 0xcc, 0x0c, 0x38, 0x60, 0xcc, 0xfc, 0x00 },		// U+0032 (2)
		{ 0x78, 0xcc, 0x0c, 0x38, 0x0c, 0xcc, 0x78, 0x00 },		// U+0033 (3)
		{ 0x1c, 0x3c, 0x6c, 0xcc, 0xfe, 0x0c, 0x1e, 0x00 },		// U+0034 (4)
		{ 0xfc, 0xc0, 0xf8, 0x0c, 0x0c, 0xcc, 0x78, 0x00 },		// U+0035 (5)
		{ 0x38, 0x60, 0xc0, 0xf8, 0xcc, 0xcc, 0x78, 0x00 },		// U+0036 (6)
		{ 0xfc, 0xcc, 0x0c, 0x18, 0x30, 0x30, 0x30, 0x00 },		// U+0037 (7)
		{ 0x78, 0xcc, 0xcc, 0x78, 0xcc, 0xcc, 0x78, 0x00 },		// U+0038 (8)
		{ 0x78, 0xcc, 0xcc, 0x7c, 0x0c, 0x18, 0x70, 0x00 },		// U+0039 (9)
		{ 0x00, 0x30, 0x30, 0x00, 0x00, 0x30, 0x30, 0x00 },		// U+003A (:)
		{ 0x3c, 0x66, 0xc0, 0xc0, 0xc0, 0x66, 0x3c, 0x00 },		// U+0043 (C)
		{ 0x78, 0x30, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00 },		// U+0049 (I)
		{ 0xc6, 0xe6, 0xf6, 0xde, 0xce, 0xc6, 0xc6, 0x00 },		// U+004E (N)
		{ 0x38, 0x6c, 0xc6, 0xc6, 0xc6, 0x6c, 0x38, 0x00 },		// U+004F (O)
		{ 0xfc, 0x66, 0x66, 0x7c, 0x60, 0x60, 0xf0, 0x00 },		// U+0050 (P)
		{ 0xfc, 0xb4, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00 },		// U+002E (.)
		{ 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xfc, 0x00 },		// U+002E (.)
		{ 0x00, 0x00, 0x78, 0x0c, 0x7c, 0xcc, 0x76, 0x00 },		// U+0061 (a)
    { 0x04, 0x0A, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00 },   // �
		{ 0x3C, 0x66, 0xC3, 0xC3, 0xC3, 0x66, 0x24, 0xE7 },   // Ohm (o)
};


// Configuration struct for SPI
static const spi_cfg_t spi_cfg = {
	
  .spi_ms = SPI_MS_MODE_MASTER,
  .spi_cp = SPI_CP_MODE_0,
  .spi_speed = SPI_SPEED_MODE_2MHz,
  .spi_wsz = SPI_MODE_8BIT,
  .spi_cs = SPI_CS_0,
  .cs_pad.port = DISPL_SPI_PORT,
  .cs_pad.pin = DISPL_CS_PIN,
  #if defined(CFG_SPI_DMA_SUPPORT)
  .spi_dma_channel = SPI_DMA_CHANNEL_01,
  .spi_dma_priority = DMA_PRIO_0,
  #endif
};

// function which returns the array index of the wanted character (from the "font" array)
uint8_t get_font_index(char* letter){
    char* mapping = " %.0123456789:CINOPTUa�o";
    uint8_t i = 0;
    while (*mapping != '\0') {
        if (*letter == *mapping) {
            return i % FONT_ARRAY_LEN; // Mapping to the range 0-FONT_ARRAY_LEN
        }
        mapping++;
        i++;
    }   
		return FONT_ARRAY_LEN + 1; // Not found
}

// send index byte to the display
void display_send_index(uint8_t index){
	GPIO_SetInactive(DISPL_SPI_PORT, DISPL_DC_PIN);
	spi_cs_low();
	spi_send(&index, 1, SPI_OP_BLOCKING);
	spi_cs_high();
	GPIO_SetActive(DISPL_SPI_PORT, DISPL_DC_PIN);
}

// send data byte to the display
void display_send_data(uint8_t data){
	spi_cs_low();
	spi_send(&data, 1, SPI_OP_BLOCKING);
	spi_cs_high();
}

// initialize the display and the IO-expander
void display_init(){
	spi_initialize(&spi_cfg);
	init_io();
	
	// configure the SPI-pins
	GPIO_ConfigurePin(DISPL_SPI_PORT, DISPL_CS_PIN, OUTPUT, PID_SPI_EN, true);
	GPIO_ConfigurePin(DISPL_SPI_PORT, DISPL_SCK_PIN, OUTPUT, PID_SPI_CLK, false);
	GPIO_ConfigurePin(DISPL_SPI_PORT, DISPL_SDO_PIN, OUTPUT, PID_SPI_DO, false);
	GPIO_ConfigurePin(DISPL_SPI_PORT, DISPL_DC_PIN, OUTPUT, PID_GPIO, true);
	
}

// power on the displays driver chip
void display_power_on(){
	displ_rst_high();
	systick_wait(5000);
	displ_rst_low();
	systick_wait(10000);
	displ_rst_high();
	systick_wait(5000);
	
	display_send_index(0x00);
	display_send_data(0x0E);  // soft-reset
	systick_wait(5000);
}

// configure the display
void display_config(){
	display_send_index(0xE5); // set ambient temperature
	display_send_data(0x19);
	
	display_send_index(0xE0);
	display_send_data(0x02);
	
	display_send_index(0x00); // set the display size (4.2")
	display_send_data(0x0F);
	display_send_data(0x89);
}

void display_update_image(){
	while(!displ_busy_read());
	display_send_index(0x04);
	while(!displ_busy_read());
	display_send_index(0x12);
	wdg_freeze();
	while(!displ_busy_read());
	wdg_resume();
 	display_send_index(0x02);
}

// scale the characters according to font size (nearest neighbour algorithm)
uint8_t* scale_byte(uint8_t byte){
    static uint8_t bytes[FONT_SIZE];

    for (uint8_t i = 0; i < FONT_SIZE; i++){  // scale the original byte with FONT_SIZE
        uint8_t new_byte = 0;
        for (uint8_t j = 0; j < 8/FONT_SIZE; j++){  // 8/FONT_SIZE = how many bits are being scaled to a byte
            uint8_t bit = (uint8_t)(byte & 0x80) >> (j*FONT_SIZE);  // read a bit
            for (uint8_t k = 0; k < FONT_SIZE; k++){  // set the bits in the new byte
                new_byte = new_byte | bit;
                bit = bit >> 1;
            }
            byte = byte << 1;
        }
				
        bytes[i] = new_byte;
    }
    
    return bytes;
}

void draw_string(char* string, uint8_t len){
	uint8_t char_index;
	uint8_t data;
	uint8_t byte = 0;
	uint8_t* bytes = NULL;
  for (uint8_t i = 0; i < 8; i++) {
		for (uint8_t j = 0; j < FONT_SIZE; j++){
			for (uint8_t k = 0; k < BYTESPERROW; k++) {
				if (k >= LEFTEDGEBYTE && *string != '\0') {
					if (byte == 0){
						char_index = get_font_index(string);
						data = font[char_index][i];
						bytes = scale_byte(data);
					}
  					display_send_data(bytes[byte]);
					byte++;
					if (byte == FONT_SIZE){
						byte = 0;
						string++;
					}
				}
				else{
					display_send_data(0x00);
				}
			}
			string -= len ;
		}	
	}
}

void display_send_image(struct environment_data* measured_data){
	char node_temp[10];
	char temperature[10];
	char pressure[10]; 
	char humidity[10];
	char gas_resistance[10];
	char celsius[2] = "�C";
	char pascal[3] = " Pa";
	char percent[2] = " %";
	char ohm[2] = " o";
	

	sprintf(node_temp, "%.2f", measured_data->node_temperature);
	sprintf(temperature, "%.2f", measured_data->temperature);
	sprintf(pressure, "%d", measured_data->pressure);
	sprintf(humidity, "%.2f", measured_data->humidity);
	sprintf(gas_resistance, "%d", measured_data->gas_resistance);
	
	
	char string1[20] = "IN:       ";
	char string2[20] = "          ";
	char string3[20] = "          ";
	char string4[20] = "          ";
	char string5[20] = "OUT:      ";
	
  strcat(string1, temperature);
	strcat(string1, celsius);
	strcat(string2, humidity);
	strcat(string2, percent);
	strcat(string3, pressure);
	strcat(string3, pascal);
	strcat(string4, gas_resistance);
	strcat(string4, ohm);
	strcat(string5, node_temp);
	strcat(string5, celsius);
	
	
	display_send_index(0x10);
	draw_empty_row(2);
	draw_string(string1, strlen(string1));
	draw_empty_row(4);
	draw_string(string2, strlen(string2));
	draw_empty_row(4);
	draw_string(string3, strlen(string3));
	draw_empty_row(4);
	draw_string(string4, strlen(string4));
	draw_empty_row(7);
	draw_string(string5, strlen(string5));
	draw_empty_row(8);
	
	for (uint8_t i = 1; i <= 200; i++){
		display_send_data(0x00);
	}
	
	display_send_index(0x13);
	for (uint16_t i = 1; i <= 15000; i++){
		display_send_data(0x00);		
	}
}

void draw_empty_row(uint8_t number_of_rows){
	for (uint8_t i = 1; i <= number_of_rows; i++){
		for (uint16_t i = 1; i <= BYTESPERCHARROW; i++) {
			display_send_data(0x00);	
		}
	}
}	
