#pragma once
#include <cstdint>
#include <array>
#include <memory>

#include "CPU.h"
#include "olc2c02.h"
#include "Cartridge.h"


class Bus
{
public:
	Bus();
	~Bus();

public:
	CPU cpu;
	olc2c02 ppu;

	std::array<uint8_t, 2048> cpuRam;

	std::shared_ptr<Cartridge> cart;


public:
	void writeFromCPU(uint16_t addr, uint8_t data);
	uint8_t readFromCPU(uint16_t addr, bool bReadOnly = false);

public:
	void insertCartridge(const std::shared_ptr<Cartridge>& cartridge);
	void reset();
	void clock();

private:

	uint32_t nSystemClockCounter = 0;
};

