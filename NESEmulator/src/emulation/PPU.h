#pragma once
#include <cstdint>
#include <memory>

#include "Cartridge.h"

class PPU
{
public:
	PPU();
	~PPU();
public:
	uint8_t readFromCPU(uint16_t addr, bool rdonly = false);
	void writeFromCPU(uint16_t addr, uint8_t data);

	uint8_t readFromPPU(uint16_t addr, bool rdonly = false);
	void writeFromPPU(uint16_t addr, uint8_t data);
private:
	std::shared_ptr<Cartridge> cart;
	uint8_t tblName[2][1024];
	uint8_t tblPalette[32];
	uint8_t tblPattern[2][4096];

public:
	void ConnectCartridge(const std::shared_ptr<Cartridge>& cartridge);
	void clock();

private:
	int16_t scanline = 0;
	int16_t cycle = 0;

	union {
		struct {
			uint8_t unused : 5;
			uint8_t sprite_overflow : 1;
			uint8_t sprite_zero_hit : 1;
			uint8_t vertical_blank : 1;
		};

		uint8_t reg;
	} status;

	union
	{
		struct
		{
			uint8_t grayscale : 1;
			uint8_t render_background_left : 1;
			uint8_t render_sprites_left : 1;
			uint8_t render_background : 1;
			uint8_t render_sprites : 1;
			uint8_t enhance_red : 1;
			uint8_t enhance_green : 1;
			uint8_t enhance_blue : 1;
		};

		uint8_t reg;
	} mask;

	union PPUCTRL
	{
		struct
		{
			uint8_t nametable_x : 1;
			uint8_t nametable_y : 1;
			uint8_t increment_mode : 1;
			uint8_t patter_sprite : 1;
			uint8_t pattern_background : 1;
			uint8_t sprite_size : 1;
			uint8_t slave_mode : 1;
			uint8_t enable_nmi : 1;
		};

		uint8_t reg;
	} control;

	uint8_t address_latch = 0x00;
	uint8_t ppu_data_buffer = 0x00;
	uint16_t ppu_address = 0x0000;
};

