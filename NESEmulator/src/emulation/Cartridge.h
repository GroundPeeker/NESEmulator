#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <fstream>

#include "Mapper.h"
#include "Mapper_000.h"

class Cartridge
{
public:
	Cartridge(const std::string& sFileName);
	~Cartridge();

private:
	std::vector<uint8_t> vPRGMemory;
	std::vector<uint8_t> vCHRMemory;

	uint8_t nMapperID = 0;
	uint8_t nPRGBanks = 0;
	uint8_t nCHRBanks = 0;

	std::shared_ptr<Mapper> pMapper;

public:
	bool cpuRead(uint16_t addr, uint8_t& data);
	bool cpuWrite(uint16_t addr, uint8_t data);

	bool ppuRead(uint16_t addr, uint8_t& data);
	bool ppuWrite(uint16_t addr, uint8_t data);
};

