#pragma once
#define tile_table_size 14

#define tile_1 0
#define tile_2 2
#define tile_3 4
#define tile_4 6
#define tile_palette_1 8
#define tile_palette_2 9
#define tile_flips 10
#define collision 11
#define act_as_high 12
#define act_as_low 13
#define ram_level_low 0x8000
#define ram_level_high 0xC000

uint_fast8_t map16_entries[0x1C00];

uint_fast8_t spawned_grabbable = 0xFF;

void reset_map()
{
	for (int i = ram_level_low; i < ram_level_high; i++) {
		RAM[i] = 0x25;
		RAM[i + 0x4000] = 0x00;
	}

	for (int i = 0; i < 0x1000; i++) {
		RAM[0x2000 + i] = 0;
	}
}

void initialize_map16()
{
	string file = path + "Map16/Global.Cmap16";

	cout << blue << "[MAP16] Loading " << file << white << endl;
	ifstream input(file, ios::binary);
	vector<unsigned char> buffer(istreambuf_iterator<char>(input), {});


	uint8_t temp[16];
	int current_byte = 0;
	for (auto &v : buffer)
	{
		temp[current_byte] = uint8_t(v);
		current_byte += 1;
		if (current_byte >= 16) {
			uint16_t replace_p = temp[1] + temp[0] * 256; //this is actually a thing.
			for (int i = 0; i < tile_table_size; i++)
			{
				map16_entries[(replace_p * tile_table_size) + i] = temp[i + 2];
			}
			int integer = map16_entries[collision + replace_p * tile_table_size];
			current_byte = 0;
			
		}
	}
	input.close();
}

class map16blockhandler //Map16 loaded block
{
public:
	uint_fast16_t tile;
	uint_fast16_t act_as;
	bool logic[8];

	void get_map_16_details()
	{
		uint_fast16_t entry = (tile & 0x1FF) * tile_table_size;
		act_as = map16_entries[entry + act_as_low] + (map16_entries[entry + act_as_high] << 8);

		uint_fast8_t integer = map16_entries[entry + collision];
		for (uint_fast8_t i = 0; i < 8; i++)
		{
			logic[i] = ((integer >> (7 - i)) & 1) != 0;
		}

		if (tile >= 0x166 && tile <= 0x167)
		{
			bool solid = tile == 0x166 ? (!RAM[0x14AF]) : (RAM[0x14AF]);
			logic[0] = solid; logic[1] = solid; logic[2] = solid; logic[3] = solid;
		}
	}

	/*
		Update Map Tile
	*/
	void update_map_tile(uint_fast16_t x, uint_fast16_t y)
	{
		uint_fast32_t index = (x % mapWidth) + (max(uint_fast16_t(0), min(mapHeight, y)) * mapWidth);
		tile = RAM[ram_level_low + index] + (RAM[ram_level_high + index] << 8);
		get_map_16_details();
	}

	/*
		Replace Map tile with anything
	*/
	void replace_map_tile(uint16_t tile, uint_fast16_t x, uint_fast16_t y)
	{
		uint_fast32_t index = (x % mapWidth) + (max(uint_fast16_t(0), min(mapHeight, y)) * mapWidth);
		RAM[ram_level_low + index] = tile & 0xFF; RAM[ram_level_high + index] = tile >> 8;

		RAM_decay_time_level[index] = level_ram_decay_time * PlayerAmount;
	}

	/*
		Get ground
	*/
	double ground_y(double x_relative, uint_fast16_t x, uint_fast16_t y)
	{
		uint_fast16_t tile = get_tile(x, y);
		if (tile == 0x1AA) //45* slope Right
		{
			if (x_relative <= 0 || x_relative >= 16) { return -9999; }
			return x_relative;
		}
		if (tile == 0x1AF) //45* slope Left
		{
			if (x_relative <= 0 || x_relative >= 16) { return -9999; }
			return 16.0 - x_relative;
		}
		return 16.0;
	}

	double ground_s(uint_fast16_t x, uint_fast16_t y)
	{
		uint_fast16_t tile = get_tile(x, y);
		if (tile == 0x1AA || tile == 0x1AF) //45* slope Right/Left
		{
			return 17.0;
		}
		return 15.0;
	}
	/*
		Check if a tile is sloped
	*/
	uint_fast8_t get_slope(uint_fast16_t x, uint_fast16_t y)
	{
		if (tile == 0x1AA) { return 1; }
		if (tile == 0x1AF) { return 2; }

		return 0;
	}

	/*
		Process block hit.
	*/
	void process_block(uint_fast16_t x, uint_fast16_t y, uint8_t side, bool pressing_y = false)
	{
		if (!networking || !isClient)
		{
			uint_fast32_t index = (x % mapWidth) + (y * mapWidth);


			uint_fast16_t t = RAM[ram_level_low + index] + (RAM[ram_level_high + index] << 8);
			if (t == 0x0124 && side == bottom)
			{
				replace_map_tile(0x0132, x, y);
			}
			if (t == 0x011F && side == bottom)
			{
				replace_map_tile(0x0132, x, y);
				x *= 16;
				y *= 16;
				uint_fast8_t spr = spawnSpriteJFKMarioWorld(0x74, 5, x, y + 8, 1, true);
				RAM[0x2480 + spr] = 0x20;
			}
			if (t == 0x0112 && side == bottom)
			{
				RAM[0x14AF] = !RAM[0x14AF];
				RAM[0x1DF9] = 0xB;
			}
			if (t == 0x0038)
			{
				replace_map_tile(0x0025, x, y);
				RAM[0x1DF9] = 5;

				ASM.Write_To_Ram(0x3F0B, x * 16, 2);
				ASM.Write_To_Ram(0x3F0D, y * 16, 2);
				midway_activated = true;
			}
			if (t == 0x002B)
			{
				replace_map_tile(0x0025, x, y);
				RAM[0x1DFC] = 1;
				RAM[0x0DBF] += 1;
			}

			if (t == 0x012E && pressing_y)
			{
				replace_map_tile(0x0025, x, y);
				x *= 16;
				y *= 16;
				spawned_grabbable = spawnSpriteJFKMarioWorld(0x53, 2, x, y, 0, true);
			}
		}

	}

	/*
		Get tile on map.
	*/
	uint_fast16_t get_tile(uint_fast16_t x, uint_fast16_t y)
	{
		uint_fast32_t index = (x % mapWidth) + (max(uint_fast16_t(0),min(mapHeight-1,y)) * mapWidth);
		return RAM[ram_level_low + index] + (RAM[ram_level_high + index] << 8);
	}


};

map16blockhandler map16_handler;