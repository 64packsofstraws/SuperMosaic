#include "Cartridge.h"

Cartridge::Cartridge(std::vector<uint8_t> rom, std::vector<uint8_t> ram, bool rom_speed) : 
    rom(std::move(rom)), ram(std::move(ram)), rom_speed(rom_speed)
{
}

bool Cartridge::get_rom_speed() const
{
    return rom_speed;
}

LoROM::LoROM(std::vector<uint8_t> rom, std::vector<uint8_t> ram, bool rom_speed) :
    Cartridge(std::move(rom), std::move(ram), rom_speed)
{
}

bool LoROM::in_range(uint32_t addr)
{
    uint8_t bank = (addr >> 16) & 0xFF;
    uint32_t idx = ((addr & 0xFFFF) - 0x8000) + 0x8000 * (bank % 0x80);

    return (addr & 0xFFFF) >= 0x8000 && (bank != 0x7E && bank != 0x7F) && idx < rom.size();
}

uint8_t LoROM::read(uint32_t addr)
{
    uint8_t bank = (addr >> 16) & 0xFF;
    uint32_t idx = ((addr & 0xFFFF) - 0x8000) + 0x8000 * (bank % 0x80);
    return rom[idx];
}
