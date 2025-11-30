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

bool LoROM::ram_in_range(uint32_t addr)
{
    uint8_t bank = (addr >> 16) & 0xFF;
    uint16_t word = addr & 0xFFFF;

    return bank >= 0x70 && bank <= 0x7D && word < 0x8000;
}

uint8_t LoROM::read_ram(uint32_t addr)
{
    if (ram.empty()) return 0;

    uint8_t bank = ((addr >> 16) & 0xFF) - 0x70;
    uint32_t idx = ((addr & 0xFFFF) + 0x8000 * bank) % ram.size();
    return ram[idx];
}

void LoROM::write_ram(uint32_t addr, uint8_t val)
{
    if (ram.empty()) return;

    uint8_t bank = ((addr >> 16) & 0xFF) - 0x70;
    uint32_t idx = ((addr & 0xFFFF) + 0x8000 * bank) % ram.size();
    ram[idx] = val;
}

HiROM::HiROM(std::vector<uint8_t> rom, std::vector<uint8_t> ram, bool rom_speed) :
    Cartridge(std::move(rom), std::move(ram), rom_speed)
{
}

bool HiROM::in_range(uint32_t addr)
{
    uint8_t bank = (addr >> 16) & 0xFF;
    uint16_t word = addr & 0xFFFF;

    return (bank >= 0xC0) || (bank >= 0x40 && bank <= 0x7D) || (bank <= 0x3F && word >= 0x8000) || (bank >= 0x80 && bank <= 0xBF && word >= 0x8000);
}

uint8_t HiROM::read(uint32_t addr)
{
    uint8_t bank = (addr >> 16) & 0xFF;
    uint16_t word = addr & 0xFFFF;
    uint32_t idx = 0;

    if (bank >= 0xC0)
        idx = addr - 0xC00000;
    else if (bank >= 0x40 && bank <= 0x7D)
        idx = addr - 0x400000;
    else if (bank <= 0x3F && word >= 0x8000)
        idx = word + bank * 0x10000;
    else if (bank >= 0x80 && bank <= 0xBF && word >= 0x8000)
        idx = word + (bank - 0x80) * 0x10000;

    return rom[idx % rom.size()];
}

bool HiROM::ram_in_range(uint32_t addr)
{
    uint8_t bank = (addr >> 16) & 0xFF;
    uint16_t word = addr & 0xFFFF;

    return (bank >= 0x30 && bank <= 0x3F) && (word >= 0x6000 && word <= 0x7FFF);
}

uint8_t HiROM::read_ram(uint32_t addr)
{
    if (ram.empty()) return 0;

    uint8_t bank = ((addr >> 16) & 0xFF) - 0x30;
    uint32_t idx = ((addr & 0xFFFF - 0x6000) + 0x2000 * bank) % ram.size();
    return ram[idx];
}

void HiROM::write_ram(uint32_t addr, uint8_t val)
{
    if (ram.empty()) return;

    uint8_t bank = ((addr >> 16) & 0xFF) - 0x30;
    uint32_t idx = ((addr & 0xFFFF - 0x6000) + 0x2000 * bank) % ram.size();
    ram[idx] = val;
}