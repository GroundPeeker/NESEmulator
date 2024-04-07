#pragma once
#include <cstdint>
#include <memory>

#include "Cartridge.h"
#include "olcPixelGameEngine.h"

class olc2c02
{
public:
	olc2c02();
	~olc2c02();
public:
	uint8_t cpuRead(uint16_t addr, bool rdonly = false);
	void cpuWrite(uint16_t addr, uint8_t data);

	uint8_t ppuRead(uint16_t addr, bool rdonly = false);
	void ppuWrite(uint16_t addr, uint8_t data);
private:
	std::shared_ptr<Cartridge> cart;
	uint8_t tblName[2][1024];
	uint8_t tblPalette[32];
	uint8_t tblPattern[2][4096];

public:
	void ConnectCartridge(const std::shared_ptr<Cartridge>& cartridge);
	void clock();

private:
	olc::Pixel palScreen[0x40];
	olc::Sprite* sprScreen;
	olc::Sprite* sprNameTable[2];
	olc::Sprite* sprPatternTable[2];

public:
	olc::Sprite& GetScreen();
	olc::Sprite& GetNameTable(uint8_t i);
	olc::Sprite& GetPatternTable(uint8_t i);
	bool frame_complete = false;

private:
	int16_t scanline = 0;
	int16_t cycle = 0;
};

