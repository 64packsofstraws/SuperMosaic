#pragma once
#include <cstdint>
#include <vector>

class Cartridge
{
protected:
	std::vector<uint8_t> rom;
	std::vector<uint8_t> ram;
	bool rom_speed;

public:

	Cartridge(std::vector<uint8_t> rom, std::vector<uint8_t> ram, bool rom_speed);

	bool get_rom_speed() const;

	virtual bool in_range(uint32_t addr) = 0;
	virtual uint8_t read(uint32_t addr) = 0;

	virtual bool ram_in_range(uint32_t addr) = 0;
	virtual uint8_t read_ram(uint32_t addr) = 0;
	virtual void write_ram(uint32_t addr, uint8_t val) = 0;
};

class LoROM : public Cartridge
{
public:
	LoROM(std::vector<uint8_t> rom, std::vector<uint8_t> ram, bool rom_speed);

	bool in_range(uint32_t addr);
	uint8_t read(uint32_t addr);

	bool ram_in_range(uint32_t addr);
	uint8_t read_ram(uint32_t addr);
	void write_ram(uint32_t addr, uint8_t val);
};

class HiROM : public Cartridge
{
public:
	HiROM(std::vector<uint8_t> rom, std::vector<uint8_t> ram, bool rom_speed);

	bool in_range(uint32_t addr);
	uint8_t read(uint32_t addr);

	bool ram_in_range(uint32_t addr);
	uint8_t read_ram(uint32_t addr);
	void write_ram(uint32_t addr, uint8_t val);
};