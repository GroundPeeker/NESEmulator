#include "Bus.h"



Bus::Bus()
{
	cpu.ConnectBus(this);
	for (auto &i : cpuRam) i = 0x00;
}


Bus::~Bus()
{
}

void Bus::writeFromCPU(uint16_t addr, uint8_t data)
{
	if (cart->cpuWrite(addr, data)) {

	} else if (addr >= 0x0000 && addr <= 0x1FFF)
		cpuRam[addr & 0x07FF] = data;
	else if (addr >= 0x2000 && addr <= 0x3FFF) {
		ppu.writeFromCPU(addr & 0x0007, data);
	}
}

uint8_t Bus::readFromCPU(uint16_t addr, bool bReadOnly)
{
	uint8_t data = 0x00;

	if (addr >= 0x0000 && addr <= 0x1FFF)
		data = cpuRam[addr & 0x07FF];
	else if (addr >= 0x2000 && addr <= 0x3FFF) {
		data = ppu.readFromCPU(addr & 0x0007, bReadOnly);
	}

	return data;
}

void Bus::reset()
{
	cpu.reset();
	nSystemClockCounter = 0;
}

void Bus::clock()
{
	ppu.clock();

	if (nSystemClockCounter % 3 == 0) {
		cpu.clock();
	}

	nSystemClockCounter++;
}

void Bus::insertCartridge(const std::shared_ptr<Cartridge>& cartridge) {
	this->cart = cartridge;
	ppu.ConnectCartridge(cartridge);
}