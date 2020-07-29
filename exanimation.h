#pragma once
/*
	This should only be done in the client, if there's need for, a better implementation will be done later.
	Why it's not done on the server : It would be laggy
*/
void process_ex_animation()
{
	if (!networking || (networking && isClient))
	{
		//Question block (Corrected)
		for (int i = 0; i < 4; i++)
		{
			for (int b = 0; b < 32; b++)
			{
				RAM[VRAM_Location + (32 * 0x60) + (i * 32) + b] = RAM[VRAM_Location + 0x8000 + (0xC0 * 32) + b + (i * 32) + (((global_frame_counter / 8) % 4) * 0x10 * 32)];
			}
		}

		//Brown block
		for (int i = 0; i < 4; i++)
		{
			for (int b = 0; b < 32; b++)
			{
				RAM[VRAM_Location + (32 * 0x58) + (i * 32) + b] = RAM[VRAM_Location + 0x8000 + (0xB4 * 32) + b + (i * 32)];
			}
		}

		//Muncher
		for (int i = 0; i < 4; i++)
		{
			for (int b = 0; b < 32; b++)
			{
				RAM[VRAM_Location + (32 * 0x5C) + (i * 32) + b] = RAM[VRAM_Location + 0x8000 + b + ((0xF8 + 0x40) * 32) + (i * 32) + ((global_frame_counter / 8) % 2) * 0x80];
			}
		}

		//Water
		for (int i = 0; i < 4; i++)
		{
			for (int b = 0; b < 32; b++)
			{
				RAM[VRAM_Location + (32 * 0x70) + (i * 32) + b] = RAM[VRAM_Location + 0x8000 + (0x100 * 32) + b + (i * 32) + (((global_frame_counter / 8) % 4) * 0x10 * 32)];
			}
		}

		//L.A Coin
		for (int i = 0; i < 4; i++)
		{
			for (int b = 0; b < 32; b++)
			{
				RAM[VRAM_Location + (32 * 0x6C) + (i * 32) + b] = RAM[VRAM_Location + 0x8000 + (0xCC * 32) + b + (i * 32) + (((global_frame_counter / 8) % 4) * 0x10 * 32)];
			}
		}

		//On/Off Switch
		for (int i = 0; i < 4; i++)
		{
			for (int b = 0; b < 32; b++)
			{
				RAM[VRAM_Location + (32 * 0xDA) + (i * 32) + b] = RAM[VRAM_Location + 0x8000 + ((0xA0 + (RAM[0x14AF] << 4)) * 32) + b + (i * 32)];
			}
		}


		//On/Off Switch Blocks
		for (int i = 0; i < 4; i++)
		{
			for (int b = 0; b < 32; b++)
			{
				RAM[VRAM_Location + (32 * 0xAC) + (i * 32) + b] = RAM[VRAM_Location + 0x8000 + ((0xC8 + (RAM[0x14AF] << 5)) * 32) + b + (i * 32)];
				RAM[VRAM_Location + (32 * 0xCC) + (i * 32) + b] = RAM[VRAM_Location + 0x8000 + ((0xE8 - (RAM[0x14AF] << 5)) * 32) + b + (i * 32)];
			}
		}
	}
}