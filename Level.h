#pragma once

class LevelSprite {
public:
	bool is_lua = false;
	uint_fast8_t number = 0;
	uint_fast16_t x_pos = 0;
	uint_fast16_t y_pos = 0;
	uint_fast8_t newstate = 0;
	uint_fast8_t dir = 0;

	void spawn()
	{
		spawnSpriteJFKMarioWorld(number, newstate, x_pos, y_pos, dir, is_lua);
	}
};

vector<LevelSprite> LevelSprites; //U know what they say. All toasters toast.

void CheckSpritesInCam(int x_pos)
{
	for (int i = 0; i < LevelSprites.size(); i++)
	{
		LevelSprite& CSprite = LevelSprites[i];
		int x = int(CSprite.x_pos);
		if (x > (x_pos - spawn_bound_x) && x < (x_pos + spawn_bound_x))
		{
			CSprite.spawn();
			LevelSprites.erase(LevelSprites.begin() + i);
			i--;
		}
	}

	for (uint_fast8_t i = 0; i < 128; i++)
	{
		int x = RAM[0x2100 + i] + RAM[0x2180 + i] * 256;
		if (x > (x_pos - spawn_bound_x) && x < (x_pos + spawn_bound_x))
		{
			if (!(RAM[0x2A80 + i] & 0b00000010))
			{
				RAM[0x2A80 + i] |= 0b00000010;
			}
		}
	}
}

class Level
{
public:
	Level() {}


	//vector<string, int> data;
	string current_level;
	double start_x = 16;
	double start_y = 16;
	int chunks = 0;
	unordered_map<string, uint_fast32_t> level_data;

	uint_fast32_t request_level_entry(string name)
	{
		auto entry = level_data.find(name);

		if (entry != level_data.end())
		{
			return entry->second;
		}

		cout << green
			<< "[Level Manager] Couldn't find entry " << name
			<< " : not created yet?" << white << endl;

		return 0;
	}

	void add_entry(string name, uint_fast32_t value)
	{

		cout << green
			<< "[Level Manager] Added entry " << name
			<< " = " << dec << value << white << endl;


		level_data.insert(make_pair(name, value));
	}


	string LoadLevelData(string FILENAME)
	{
		ifstream file;
		file.open(FILENAME);
		if (file.is_open())
		{

			stringstream strStream;
			strStream << file.rdbuf(); //read the file
			string str = strStream.str(); //str holds the content of the file
			file.close();
			return str;
		}
		file.close();
		return "Error";
	}
	void LoadLevelFromFile(string FILENAME, uint_fast16_t num)
	{
		current_level = FILENAME;
		cout << green
			<< "[Level Manager] Loading " << FILENAME
			<< white << endl;
		LoadLevelFromString(LoadLevelData(FILENAME), num);
	}

	void LoadLevelFromString(string DLevel, uint_fast16_t num)
	{
		chunks = 0;
		lua_loaded = false;
		reset_map();

		cout << green
			<< "[Level Manager] Processing level string.."
			<< white << endl;

		string status;
		string line;

		istringstream str(DLevel); // string


		LevelSprites.clear();
		//int spr_index = 0;
		while (getline(str, line)) {
			// using printf() in all tests for consistency
			if (line != "")
			{
				//cout << line << endl;
				string CHECK = line.substr(0, 2);

				if (CHECK != "//")
				{
					if (line.substr(0, 1) == "[")
					{
						status = line.substr(1, line.length() - 2);
						cout << green << "[Level Manager] Status = " << status << white << endl;

						//data.push_back();
						continue;
					}

					if(status == "level_config")
					{
						line.erase(remove_if(line.begin(), line.end(), ::isspace),
							line.end());
						auto delimiterPos = line.find("=");
						auto name = line.substr(0, delimiterPos);
						auto value = line.substr(delimiterPos + 1);

						if (name == "music")
						{
							ASM.Write_To_Ram(0x1DFB, stoi(value, nullptr, 16), 1);
							continue;
						}
						if (name == "background")
						{
							ASM.Write_To_Ram(0x3F05, stoi(value, nullptr, 16), 1);
							continue;
						}
						if (name == "size_x")
						{
							ASM.Write_To_Ram(0x3F00, stoi(value), 2);
							mapWidth = stoi(value);
						}
						if (name == "size_y")
						{
							ASM.Write_To_Ram(0x3F02, stoi(value), 2);
							mapHeight = stoi(value);
						}
						// cout << green << "[Level Manager] adding level config entry " << name << " = " << value << white << endl;
						add_entry(name, stoi(value));
					}

					if (status == "scripts")
					{
						line.erase(remove_if(line.begin(), line.end(), ::isspace),
							line.end());
						auto delimiterPos = line.find("=");
						auto name = line.substr(0, delimiterPos);
						auto value = line.substr(delimiterPos + 1);
						auto type = name.substr(name.length() - 3);

						//cout << "[load script] type " << type << endl;
						cout << green
							<< "[Level Manager] Loading Script " << name << " of type " << type
							<< white << endl;
						// cout << green << "[Level Manager] adding level config entry " << name << " = " << value << white << endl;
						if (type == "lua")
						{
							lua_loadfile("Levels/" + int_to_hex(num) + "/" + name);
							lua_loaded = true;
							lua_run_init();
						}

						if (type == "asm")
						{
							ASM.load_asm("Levels/" + int_to_hex(num) + "/" + name, location_rom_levelasm);
							if (value == "init")
							{
								ASM.start_JFK_thread();
							}
							else
							{
								asm_loaded = true;
							}
						}

					}

					if (status == "level_data")
					{
						//cout << "Level loading Line = " + line << endl;
						vector<string> v = split(line.c_str(), ',');

						if (v.size() == 5) //Format type 1.
						{
							chunks++;
							int tile = stoi(v[0], nullptr, 16);
							int x_start = stoi(v[1]);
							int y_start = stoi(v[2]);
							int x_end = stoi(v[3]);
							int y_end = stoi(v[4]);

							for (int x = x_start; x <= x_end; x++)
							{
								for (int y = y_start; y <= y_end; y++)
								{
									map16_handler.replace_map_tile(tile, x, y);
								}
							}
						}
						
						if(v.size() == 3) //Format type 2
						{
							chunks++;
							int tile = stoi(v[0], nullptr, 16);
							int x = stoi(v[1]);
							int y = stoi(v[2]);

							map16_handler.replace_map_tile(tile, x, y);
						}
					}

					if (status == "sprite_data")
					{
						vector<string> v = split(line.c_str(), ',');


						/*RAM[0x2800 + spr_index] = v[0] == "lua" ? 1 : 0;

						RAM[0x2000 + spr_index] = 1;
						RAM[0x2080 + spr_index] = stoi(v[1], nullptr, 16);
						RAM[0x2100 + spr_index] = stoi(v[2]) % 256;
						RAM[0x2180 + spr_index] = stoi(v[2]) / 256;
						RAM[0x2200 + spr_index] = 0;

						RAM[0x2280 + spr_index] = stoi(v[3]) % 256;
						RAM[0x2300 + spr_index] = stoi(v[3]) / 256;
						RAM[0x2380 + spr_index] = 0;

						RAM[0x2680 + spr_index] = stoi(v[4]);
						RAM[0x2F80 + spr_index] = 0;*/

						LevelSprites.push_back(
							LevelSprite{
								v[0] == "lua",
								uint_fast8_t(stoi(v[1], nullptr, 16)),
								uint_fast16_t(stoi(v[2])),
								uint_fast16_t(stoi(v[3])),
								1,
								uint_fast8_t(stoi(v[4]))
							}
						);
					}

				}
			}

		}

		cout << green
			<< "[Level Manager] Finished loading level. " << chunks << " chunks loaded."
			<< white << endl;
	}

	void Initialize_Level()
	{
		ASM.Write_To_Ram(0x1411, 1, 1);
		ASM.Write_To_Ram(0x1412, 1, 1);

		ASM.Write_To_Ram(0x1462, 0, 2);
		ASM.Write_To_Ram(0x1464, 0, 2);
		ASM.Write_To_Ram(0x1466, 0, 2);
		ASM.Write_To_Ram(0x1468, 0, 2);

		ASM.Write_To_Ram(0x14AF, 0, 1);

		ASM.Write_To_Ram(0x7C, 96, 1);

		ASM.Write_To_Ram(0x85, 0, 1);

		ASM.Write_To_Ram(0x36, 0, 1);
		ASM.Write_To_Ram(0x38, 0x20, 1);
		ASM.Write_To_Ram(0x39, 0x20, 1);

		ASM.Write_To_Ram(0x40, 0, 1);
		ASM.Write_To_Ram(0x1493, 0, 1);

		ResetDMAandHDMA();
	}

	void LoadLevel(uint_fast16_t num)
	{

		while (!level_data.empty())
		{
			level_data.erase(level_data.begin());
		}

		Initialize_Level();
		cout << green
			<< "[Level Manager] Loading level " << int_to_hex(num) << ".."
			<< white << endl;

		read_from_palette(path + "Levels/" + int_to_hex(num) + "/level_palette.mw3");
		LoadLevelFromFile(path + "Levels/" + int_to_hex(num) + "/level_data.txt", num);

		decode_graphics_file("Graphics/GFX" + int_to_hex(request_level_entry("gfx_1"), true) + ".bin", 0); //FG1
		decode_graphics_file("Graphics/GFX" + int_to_hex(request_level_entry("gfx_2"), true) + ".bin", 1); //FG2
		decode_graphics_file("Graphics/GFX" + int_to_hex(request_level_entry("gfx_3"), true) + ".bin", 2); //FG3
		decode_graphics_file("Graphics/GFX" + int_to_hex(request_level_entry("gfx_4"), true) + ".bin", 3); //FG4
		decode_graphics_file("Graphics/GFX" + int_to_hex(request_level_entry("gfx_5"), true) + ".bin", 4); //FG5
		decode_graphics_file("Graphics/GFX" + int_to_hex(request_level_entry("gfx_6"), true) + ".bin", 5); //FG6
		decode_graphics_file("Graphics/GFX" + int_to_hex(request_level_entry("gfx_7"), true) + ".bin", 6); //FG7
		decode_graphics_file("Graphics/GFX" + int_to_hex(request_level_entry("gfx_8"), true) + ".bin", 7); //FG8

		decode_graphics_file("Graphics/GFX" + int_to_hex(request_level_entry("sp_1"), true) + ".bin", 12); //SP1
		decode_graphics_file("Graphics/GFX" + int_to_hex(request_level_entry("sp_2"), true) + ".bin", 13); //SP2
		decode_graphics_file("Graphics/GFX" + int_to_hex(request_level_entry("sp_3"), true) + ".bin", 14); //SP3
		decode_graphics_file("Graphics/GFX" + int_to_hex(request_level_entry("sp_4"), true) + ".bin", 15); //SP4

		if (networking && !isClient)
		{
			midway_activated = false;
		}
		if (!midway_activated) //if is server or midway isn't activated, load the start pos
		{
			ASM.Write_To_Ram(0x3F0B, request_level_entry("start_x") * 16, 2);
			ASM.Write_To_Ram(0x3F0D, request_level_entry("start_y") * 16, 2);

			start_x = RAM[0x3F0B] + RAM[0x3F0C] * 256;
			start_y = RAM[0x3F0D] + RAM[0x3F0E] * 256;
		}
		Do_RAM_Change();
		Set_Server_RAM();
	}
};

Level LevelManager;