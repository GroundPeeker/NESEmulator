#include "PPU.h"

PPU::PPU()
{
}

PPU::~PPU()
{
}

uint8_t PPU::readFromCPU(uint16_t addr, bool rdonly)
{
    uint8_t data = 0x00;

	if (rdonly)
	{
		switch (addr)
		{
		case 0x0000:
			data = control.reg;
			break;
		case 0x0001:
			data = mask.reg;
			break;
		case 0x0002:
			data = status.reg;
			break;
		case 0x0003:
			break;
		case 0x0004:
			break;
		case 0x0005:
			break;
		case 0x0006:
			break;
		case 0x0007:
			break;
		}
	}
	else
	{
		switch (addr)
		{
		case 0x0000: break;
		case 0x0001: break;
		case 0x0002:
			data = (status.reg & 0xE0) | (ppu_data_buffer & 0x1F);
			status.vertical_blank = 0;
			address_latch = 0;
			break;
		case 0x0003: break;
		case 0x0004: 
			data = pOAM[oam_addr];
			break;
		case 0x0005: break;
		case 0x0006: break;
		case 0x0007: 
			data = ppu_data_buffer;
			ppu_data_buffer = readFromPPU(vram_addr.reg);
			if (vram_addr.reg >= 0x3F00) data = ppu_data_buffer;
			vram_addr.reg += (control.increment_mode ? 32 : 1);
			break;
		}
	}

	return data;
}

void PPU::writeFromCPU(uint16_t addr, uint8_t data)
{
	switch (addr)
	{
	case 0x0000:
		control.reg = data;
		tram_addr.nametable_x = control.nametable_x;
		tram_addr.nametable_y = control.nametable_y;
		break;
	case 0x0001:
		mask.reg = data;
		break;
	case 0x0002:
		break;
	case 0x0003:
		oam_addr = data;
		break;
	case 0x0004:
		pOAM[oam_addr] = data;
		break;
	case 0x0005:
		if (address_latch == 0)
		{
			fine_x = data & 0x07;
			tram_addr.coarse_x = data >> 3;
			address_latch = 1;
		}
		else
		{
			tram_addr.fine_y = data & 0x07;
			tram_addr.coarse_y = data >> 3;
			address_latch = 0;
		}
		break;
	case 0x0006:
		if (address_latch == 0)
		{
			tram_addr.reg = (uint16_t)((data & 0x3F) << 8) | (tram_addr.reg & 0x00FF);
			address_latch = 1;
		}
		else
		{
			tram_addr.reg = (tram_addr.reg & 0xFF00) | data;
			vram_addr = tram_addr;
			address_latch = 0;
		}
		break;
	case 0x0007:
		writeFromPPU(vram_addr.reg, data);
		vram_addr.reg += (control.increment_mode ? 32 : 1);
		break;
	}
}

uint8_t PPU::readFromPPU(uint16_t addr, bool rdonly)
{
    uint8_t data = 0x00;
    addr &= 0x3FFF;

	if (cart->readFromPPU(addr, data)) {

	}
	else if (addr >= 0x0000 && addr <= 0x1FFF) {
		data = tblPattern[(addr & 0x1000)][addr & 0x0FFF];
	}
	else if (addr >= 0x2000 && addr <= 0x3EFF) {

	}
	else if (addr >= 0x3F00 && addr <= 0x3FFF) {
		addr &= 0x001F;
		if (addr == 0x0010) addr = 0x0000;
		if (addr == 0x0014) addr = 0x0004;
		if (addr == 0x0018) addr = 0x0008;
		if (addr == 0x001C) addr = 0x000C;

		data = tblPalette[addr];
	}

    return data;
}

void PPU::writeFromPPU(uint16_t addr, uint8_t data)
{
    addr &= 0x3FF;

	if (cart->writeFromPPU(addr, data)) {

	}
	else if (addr >= 0x0000 && addr <= 0x1FFF) {
		tblPattern[(addr & 0x1000)][addr & 0x0FFF] = data;
	}
	else if (addr >= 0x2000 && addr <= 0x3EFF) {

	}
	else if (addr >= 0x3F00 && addr <= 0x3FFF) {
		addr &= 0x001F;
		if (addr == 0x0010) addr = 0x0000;
		if (addr == 0x0014) addr = 0x0004;
		if (addr == 0x0018) addr = 0x0008;
		if (addr == 0x001C) addr = 0x000C;

		tblPalette[addr] = data;
	}
}

void PPU::ConnectCartridge(const std::shared_ptr<Cartridge>& cartridge)
{
    this->cart = cartridge;
}

void PPU::clock()
{
	auto IncrementScrollX = [&]()
	{
		if (mask.render_background || mask.render_sprites)
		{
			if (vram_addr.coarse_x == 31)
			{
				vram_addr.coarse_x = 0;
				vram_addr.nametable_x = ~vram_addr.nametable_x;
			}
			else
			{
				vram_addr.coarse_x++;
			}
		}
	};
	auto IncrementScrollY = [&]()
	{
		if (mask.render_background || mask.render_sprites)
		{
			if (vram_addr.fine_y < 7)
			{
				vram_addr.fine_y++;
			}
			else
			{
				vram_addr.fine_y = 0;
				if (vram_addr.coarse_y == 29)
				{
					vram_addr.coarse_y = 0;
					vram_addr.nametable_y = ~vram_addr.nametable_y;
				}
				else if (vram_addr.coarse_y == 31)
				{
					vram_addr.coarse_y = 0;
				}
				else
				{
					vram_addr.coarse_y++;
				}
			}
		}
	};

	auto TransferAddressX = [&]()
	{
		if (mask.render_background || mask.render_sprites)
		{
			vram_addr.nametable_x = tram_addr.nametable_x;
			vram_addr.coarse_x    = tram_addr.coarse_x;
		}
	};
	auto TransferAddressY = [&]()
	{
		if (mask.render_background || mask.render_sprites)
		{
			vram_addr.fine_y      = tram_addr.fine_y;
			vram_addr.nametable_y = tram_addr.nametable_y;
			vram_addr.coarse_y    = tram_addr.coarse_y;
		}
	};

	auto LoadBackgroundShifters = [&]()
	{
		bg_shifter_pattern_lo = (bg_shifter_pattern_lo & 0xFF00) | bg_next_tile_lsb;
		bg_shifter_pattern_hi = (bg_shifter_pattern_hi & 0xFF00) | bg_next_tile_msb;
		bg_shifter_attrib_lo  = (bg_shifter_attrib_lo & 0xFF00) | ((bg_next_tile_attrib & 0b01) ? 0xFF : 0x00);
		bg_shifter_attrib_hi  = (bg_shifter_attrib_hi & 0xFF00) | ((bg_next_tile_attrib & 0b10) ? 0xFF : 0x00);
	};


	auto UpdateShifters = [&]()
	{
		if (mask.render_background)
		{
			bg_shifter_pattern_lo <<= 1;
			bg_shifter_pattern_hi <<= 1;

			bg_shifter_attrib_lo <<= 1;
			bg_shifter_attrib_hi <<= 1;
		}

		if (mask.render_sprites && cycle >= 1 && cycle < 258)
		{
			for (int i = 0; i < sprite_count; i++)
			{
				if (spriteScanline[i].x > 0)
				{
					spriteScanline[i].x--;
				}
				else
				{
					sprite_shifter_pattern_lo[i] <<= 1;
					sprite_shifter_pattern_hi[i] <<= 1;
				}
			}
		}
	};

	if (scanline >= -1 && scanline < 240)
	{
		if (scanline == 0 && cycle == 0)
		{
			cycle = 1;
		}

		if (scanline == -1 && cycle == 1)
		{
			status.vertical_blank = 0;
			status.sprite_overflow = 0;
			status.sprite_zero_hit = 0;
			for (int i = 0; i < 8; i++)
			{
				sprite_shifter_pattern_lo[i] = 0;
				sprite_shifter_pattern_hi[i] = 0;
			}
		}


		if ((cycle >= 2 && cycle < 258) || (cycle >= 321 && cycle < 338))
		{
			UpdateShifters();
			switch ((cycle - 1) % 8)
			{
			case 0:
				LoadBackgroundShifters();
				bg_next_tile_id = ppuRead(0x2000 | (vram_addr.reg & 0x0FFF));
				break;
			case 2:			
				bg_next_tile_attrib = ppuRead(0x23C0 | (vram_addr.nametable_y << 11) 
					                                 | (vram_addr.nametable_x << 10) 
					                                 | ((vram_addr.coarse_y >> 2) << 3) 
					                                 | (vram_addr.coarse_x >> 2));			
				if (vram_addr.coarse_y & 0x02) bg_next_tile_attrib >>= 4;
				if (vram_addr.coarse_x & 0x02) bg_next_tile_attrib >>= 2;
				bg_next_tile_attrib &= 0x03;
				break;

			case 4:
				bg_next_tile_lsb = ppuRead((control.pattern_background << 12) 
					                       + ((uint16_t)bg_next_tile_id << 4) 
					                       + (vram_addr.fine_y) + 0);

				break;
			case 6:
				bg_next_tile_msb = ppuRead((control.pattern_background << 12)
					                       + ((uint16_t)bg_next_tile_id << 4)
					                       + (vram_addr.fine_y) + 8);
				break;
			case 7:
				IncrementScrollX();
				break;
			}
		}
.
		if (cycle == 256)
		{
			IncrementScrollY();
		}

		if (cycle == 257)
		{
			LoadBackgroundShifters();
			TransferAddressX();
		}
		if (cycle == 338 || cycle == 340)
		{
			bg_next_tile_id = ppuRead(0x2000 | (vram_addr.reg & 0x0FFF));
		}

		if (scanline == -1 && cycle >= 280 && cycle < 305)
		{
			TransferAddressY();
		}

			std::memset(spriteScanline, 0xFF, 8 * sizeof(sObjectAttributeEntry));
			sprite_count = 0;
			for (uint8_t i = 0; i < 8; i++)
			{
				sprite_shifter_pattern_lo[i] = 0;
				sprite_shifter_pattern_hi[i] = 0;
			}
			uint8_t nOAMEntry = 0;

			bSpriteZeroHitPossible = false;

			while (nOAMEntry < 64 && sprite_count < 9)
			{
				int16_t diff = ((int16_t)scanline - (int16_t)OAM[nOAMEntry].y);

				
				if (diff >= 0 && diff < (control.sprite_size ? 16 : 8))
				{
					if (sprite_count < 8)
					{
						if (nOAMEntry == 0)
						{
							bSpriteZeroHitPossible = true;
						}

						memcpy(&spriteScanline[sprite_count], &OAM[nOAMEntry], sizeof(sObjectAttributeEntry));
						sprite_count++;
					}				
				}

				nOAMEntry++;
			}
			status.sprite_overflow = (sprite_count > 8);
		}

		if (cycle == 340)
		{
			for (uint8_t i = 0; i < sprite_count; i++)
			{

				uint8_t sprite_pattern_bits_lo, sprite_pattern_bits_hi;
				uint16_t sprite_pattern_addr_lo, sprite_pattern_addr_hi;

				if (!control.sprite_size)
				{
					if (!(spriteScanline[i].attribute & 0x80))
					{    
						sprite_pattern_addr_lo = 
						  (control.pattern_sprite << 12  )
						| (spriteScanline[i].id   << 4   )
						| (scanline - spriteScanline[i].y);
												
					}
					else
					{
						sprite_pattern_addr_lo = 
						  (control.pattern_sprite << 12  )
						| (spriteScanline[i].id   << 4   )
						| (7 - (scanline - spriteScanline[i].y));
					}

				}
				else
				{
					if (!(spriteScanline[i].attribute & 0x80))
					{
						if (scanline - spriteScanline[i].y < 8)
						{
							sprite_pattern_addr_lo = 
							  ((spriteScanline[i].id & 0x01)      << 12)
							| ((spriteScanline[i].id & 0xFE)      << 4 )
							| ((scanline - spriteScanline[i].y) & 0x07 );
						}
						else
						{
							sprite_pattern_addr_lo = 
							  ( (spriteScanline[i].id & 0x01)      << 12)
							| (((spriteScanline[i].id & 0xFE) + 1) << 4 )
							| ((scanline - spriteScanline[i].y) & 0x07  );
						}
					}
					else
					{
						if (scanline - spriteScanline[i].y < 8)
						{
							sprite_pattern_addr_lo = 
							  ( (spriteScanline[i].id & 0x01)      << 12)
							| (((spriteScanline[i].id & 0xFE) + 1) << 4 )
							| (7 - (scanline - spriteScanline[i].y) & 0x07);
						else
						{
							sprite_pattern_addr_lo = 
							  ((spriteScanline[i].id & 0x01)       << 12)
							| ((spriteScanline[i].id & 0xFE)       << 4 )
							| (7 - (scanline - spriteScanline[i].y) & 0x07);
						}
					}
				}
				sprite_pattern_addr_hi = sprite_pattern_addr_lo + 8;

				sprite_pattern_bits_lo = readFromPPU(sprite_pattern_addr_lo);
				sprite_pattern_bits_hi = readFromPPU(sprite_pattern_addr_hi);

				if (spriteScanline[i].attribute & 0x40)
				{
					
					auto flipbyte = [](uint8_t b)
					{
						b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
						b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
						b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
						return b;
					};

					sprite_pattern_bits_lo = flipbyte(sprite_pattern_bits_lo);
					sprite_pattern_bits_hi = flipbyte(sprite_pattern_bits_hi);
				}
				sprite_shifter_pattern_lo[i] = sprite_pattern_bits_lo;
				sprite_shifter_pattern_hi[i] = sprite_pattern_bits_hi;
			}
		}
	}

	if (scanline == 240)
	{
	}

	if (scanline >= 241 && scanline < 261)
	{
		if (scanline == 241 && cycle == 1)
		{
			status.vertical_blank = 1;

			if (control.enable_nmi) 
				nmi = true;
		}
	}

	uint8_t bg_pixel = 0x00;
	uint8_t bg_palette = 0x00; 
	if (mask.render_background)
	{
		uint16_t bit_mux = 0x8000 >> fine_x;
 
		uint8_t p0_pixel = (bg_shifter_pattern_lo & bit_mux) > 0;
		uint8_t p1_pixel = (bg_shifter_pattern_hi & bit_mux) > 0;

		bg_pixel = (p1_pixel << 1) | p0_pixel;

		uint8_t bg_pal0 = (bg_shifter_attrib_lo & bit_mux) > 0;
		uint8_t bg_pal1 = (bg_shifter_attrib_hi & bit_mux) > 0;
		bg_palette = (bg_pal1 << 1) | bg_pal0;
	}
	uint8_t fg_pixel = 0x00;
	uint8_t fg_palette = 0x00;
	uint8_t fg_priority = 0x00;
	if (mask.render_sprites)
	{

		bSpriteZeroBeingRendered = false;

		for (uint8_t i = 0; i < sprite_count; i++)
		{
			if (spriteScanline[i].x == 0) 
			{
				
				uint8_t fg_pixel_lo = (sprite_shifter_pattern_lo[i] & 0x80) > 0;
				uint8_t fg_pixel_hi = (sprite_shifter_pattern_hi[i] & 0x80) > 0;
				fg_pixel = (fg_pixel_hi << 1) | fg_pixel_lo;

				fg_palette = (spriteScanline[i].attribute & 0x03) + 0x04;
				fg_priority = (spriteScanline[i].attribute & 0x20) == 0;

				if (fg_pixel != 0)
				{
					if (i == 0)
					{
						bSpriteZeroBeingRendered = true;
					}

					break;
				}				
			}
		}		
	}


	uint8_t pixel = 0x00;
	uint8_t palette = 0x00;

	if (bg_pixel == 0 && fg_pixel == 0)
	{
		pixel = 0x00;
		palette = 0x00;
	}
	else if (bg_pixel == 0 && fg_pixel > 0)
	{
		pixel = fg_pixel;
		palette = fg_palette;
	}
	else if (bg_pixel > 0 && fg_pixel == 0)
	{
		pixel = bg_pixel;
		palette = bg_palette;
	}
	else if (bg_pixel > 0 && fg_pixel > 0)
	{
		if (fg_priority)
		{
			pixel = fg_pixel;
			palette = fg_palette;
		}
		else
		{
			pixel = bg_pixel;
			palette = bg_palette;
		}

		if (bSpriteZeroHitPossible && bSpriteZeroBeingRendered)
		{
			if (mask.render_background & mask.render_sprites)
			{
				if (~(mask.render_background_left | mask.render_sprites_left))
				{
					if (cycle >= 9 && cycle < 258)
					{
						status.sprite_zero_hit = 1;
					}
				}
				else
				{
					if (cycle >= 1 && cycle < 258)
					{
						status.sprite_zero_hit = 1;
					}
				}
			}
		}
	}

	sprScreen->SetPixel(cycle - 1, scanline, GetColourFromPaletteRam(palette, pixel));

	cycle++;
	if (cycle >= 341)
	{
		cycle = 0;
		scanline++;
		if (scanline >= 261)
		{
			scanline = -1;
			frame_complete = true;
		}
	}
}

