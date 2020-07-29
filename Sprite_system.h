#pragma once
/*
0x2000 - Sprite Status :
0 - Non existant
1 - Main
2 - Grabbable (Not grabbed)
3 - Grabbable (Grabbed by a player)
4 - Kicked


0x2080 - Sprite Number
0x2100 - Sprite X Position (L)
0x2180 - Sprite X Position (H)
0x2200 - Sprite X Position (F)
0x2280 - Sprite Y Position (L)
0x2300 - Sprite Y Position (H)
0x2380 - Sprite Y Position (F)
0x2400 - Sprite X Speed
0x2480 - Sprite Y Speed
0x2500 - Sprite Size X
0x2580 - Sprite Size Y
0x2600 - Sprite Flags HSGTUDLR
0x2680 - Sprite Direction
0x2700 - Sprite interacing with... (player number in hex)
0x2780 - Sprite block flags
0x2800 - Sprite is lua/asm type
0x2880 - Sprite Flags 2 ----lksS
0x2900 - Unknown
0x2A00 - Spinjump death timer for sprites, otherwise unused
0x2A80 - Offscreen status and flags -----DoO



0x2E00 - Unused, used in the throwblock and grabbed sprites for a "how much frames til we can hurt mario" timer
0x2E80 - Unused, extra property for grabbed sprites, which props/palettes it uses
0x2F00 - Unused, extra property for grabbed sprites, which tile it uses
0x2F80 - Sprite Initialized

off flags:
O - Process when off screen (0 is true, 1 is false)
o - Sprite is off sceren
D - Destroy if off screen

flags 2:
S - Death By Spinjump
s - Uses custom spinjump death
k - Collision kills sprite
l - Lower grav
o - Process when offscreen

Flags :
H = Hurts
S = Solid
G = Gravity
T = Jumpable
UDLR = Solid on up/down/left/right
*/



class sprite_system
{
public:
	double x, y;
	lua_State* SPR_STATE[128];

	void process_sprite_logic(uint_fast8_t entry = 0)
	{
		x = double(RAM[0x2100 + entry] + double(int_fast8_t(RAM[0x2180 + entry])) * 256.0) + double(RAM[0x2200 + entry]) / 256.0;
		y = double(RAM[0x2280 + entry] + double(int_fast8_t(RAM[0x2300 + entry])) * 256.0) + double(RAM[0x2380 + entry]) / 256.0;

		if (RAM[0x2600 + entry] & 0b100000) //if gravity bit is on
		{
			int_fast8_t sprgravity = RAM[0x85] ? -16 : -82;
			if (int_fast8_t(RAM[0x2480 + entry]) > sprgravity)
			{
				int grav = RAM[0x2880 + entry] & 0b1000 ? 2 : 3;

				RAM[0x2480 + entry] = max(sprgravity, RAM[0x2480 + entry] - grav);
			}
		}


		double xMove = double(double(int_fast8_t(RAM[0x2400 + entry]))*16)/256.0;
		double yMove = double(double(int_fast8_t(RAM[0x2480 + entry]))*16)/256.0;

		double x_size = double(RAM[0x2500 + entry]);
		double y_size = double(RAM[0x2580 + entry]);

		for (uint_fast8_t spr = 0; spr < 0x80; spr++)
		{
			if (spr != entry && RAM[0x2000 + spr] == 4)
			{
				double t_x = double(RAM[0x2100 + spr] + double(RAM[0x2180 + spr]) * 256.0) + double(RAM[0x2200 + spr]) / 256.0;
				double t_y = double(RAM[0x2280 + spr] + double(RAM[0x2300 + spr]) * 256.0) + double(RAM[0x2380 + spr]) / 256.0;

				if (
					t_x > (x - 16.0) &&
					t_x < (x + x_size) &&
					t_y > (y - 16.0) &&
					t_y < (y + y_size)
					)
				{
					RAM[0x2700 + entry] = 0xFF;
				}
			}
		}

		
		if (RAM[0x2600 + entry] & 0b1000000) //if solid bit is on
		{
			RAM[0x2780 + entry] = 0;
			bool g = RAM[0x2000 + entry] == 2 || RAM[0x2000 + entry] == 4;
			if (!Move(xMove, 0.0, x_size, y_size, g))
			{
				//RAM[0x2680 + entry] *= -1;
				RAM[0x2780 + entry] |= 0b00000001;
			}

			if (!Move(0.0, yMove, x_size, y_size, g))
			{
				//RAM[0x2480 + entry] = 0;
				RAM[0x2780 + entry] |= 0b00000010;

			}
		}
		else
		{
			RAM[0x2780 + entry] = 0;
			x += xMove;
			y += yMove;
		}


		if (y < (-y_size-16))
		{
			for (int i = 0; i < 32; i++)
			{
				RAM[0x2000 + entry + (i * 128)] = 0;
			}
			return;
		}

		RAM[0x2100 + entry] = uint_fast8_t(int(x));
		RAM[0x2180 + entry] = uint_fast8_t(int(x) / 256);
		RAM[0x2200 + entry] = uint_fast8_t(int(x*256.0));
		RAM[0x2280 + entry] = uint_fast8_t(int(y));
		RAM[0x2300 + entry] = uint_fast8_t(int(y) / 256); 
		RAM[0x2380 + entry] = uint_fast8_t(int(y*256.0));
		if (x < 0)
		{
			RAM[0x2180 + entry] -= 1;
		}
		if (y < 0)
		{
			RAM[0x2300 + entry] -= 1;
		}
	}

	/*
		Shitty Movement Code
	*/
	bool Move(double xMove, double yMove, double x_size, double y_size, bool kickedgrabbed)
	{
		bool finna_return = true;
		double NewPositionX = x + xMove;
		double NewPositionY = y + yMove;

		int_fast16_t PosXBlock = int_fast16_t(NewPositionX / 16); 
		int_fast16_t PosYBlock = int_fast16_t(NewPositionY / 16);
		int_fast16_t startX = PosXBlock - TotalBlocksCollisionCheckSprite - int(x_size / 16.0);
		if (startX < 0) { startX = 0; }
		int_fast16_t startY = PosYBlock - TotalBlocksCollisionCheckSprite - int(y_size / 16.0);
		if (startY < 0) { startY = 0; }
		for (int_fast16_t xB = startX; xB < PosXBlock + TotalBlocksCollisionCheckSprite + int(x_size/16.0); xB++)
		{
			for (int_fast16_t yB = startY; yB < PosYBlock + TotalBlocksCollisionCheckSprite + int(y_size / 16.0); yB++)
			{
				map16_handler.update_map_tile(xB, yB);


				double f_h = map16_handler.ground_y(NewPositionX + (x_size / 2.0) - (xB * 16), xB, yB);
				double BelowBlock = double(yB * 16) + (f_h - 16.0) - y_size;
				double AboveBlock = double(yB * 16) + f_h;
				double RightBlock = double(xB * 16) + 16.0;
				double LeftBlock = double(xB * 16) - x_size;

				bool checkLeft = map16_handler.logic[3];
				bool checkRight = map16_handler.logic[2];
				bool checkBottom = map16_handler.logic[1];
				bool checkTop = map16_handler.logic[0];

				if (NewPositionX < RightBlock && NewPositionX > LeftBlock && NewPositionY < AboveBlock && NewPositionY > BelowBlock)
				{

					if (xMove < 0.0 && checkRight)
					{
						if (NewPositionX < RightBlock && NewPositionX > RightBlock - bounds_x)
						{
							NewPositionX = RightBlock;
							finna_return = false;

							if (kickedgrabbed)
							{
								map16_handler.process_block(xB, yB, bottom);
							}
						}
					}
					if (xMove > 0.0 && checkLeft)
					{
						if (NewPositionX > LeftBlock && NewPositionX < LeftBlock + bounds_x)
						{
							NewPositionX = LeftBlock;
							finna_return = false;

							if (kickedgrabbed)
							{
								map16_handler.process_block(xB, yB, bottom);
							}
						}
					}
					if (yMove < 0.0 && checkTop)
					{
						double bound_y = bounds_y;
						uint_fast8_t new_s = map16_handler.get_slope(xB, yB);
						if (new_s != 0)
						{
							bound_y += 4;
						}
						if (NewPositionY < AboveBlock && NewPositionY > AboveBlock - bound_y)
						{
							NewPositionY = AboveBlock;
							finna_return = false;
						}

					}
					if (yMove > 0.0 && checkBottom)
					{
						if (NewPositionY > BelowBlock && NewPositionY < BelowBlock + bounds_y)
						{
							NewPositionY = BelowBlock;
							finna_return = false;

							if (kickedgrabbed)
							{
								map16_handler.process_block(xB, yB, bottom);
							}
						}
					}

				}
			}
		}

		x = NewPositionX;
		y = NewPositionY;
		return finna_return;
	}

	void call_sprite_lua(int index)
	{
		lua_getglobal(SPR_STATE[index], "Main");
		lua_pushinteger(SPR_STATE[index], index);
		lua_pcall(SPR_STATE[index], 1, 0, 0); // run script
	}

	void init_sprite_lua(int index, string file)
	{
		if (SPR_STATE[index])
		{
			lua_close(SPR_STATE[index]);
		}

		SPR_STATE[index] = luaL_newstate();

		load_lua_libs(SPR_STATE[index]);


		int ret = luaL_dofile(SPR_STATE[index], (path + file).c_str());
		if (ret != 0)
		{
			//lua_print("Error occurred when calling luaL_loadfile()");
			lua_print("Error: " + string(lua_tostring(SPR_STATE[index], -1)));
			//lua_close(SPR_STATE[index]);
			RAM[0x2000 + index] = 0;
			return;
		}

		lua_connect_functions(SPR_STATE[index]);
		lua_getglobal(SPR_STATE[index], "Init");
		lua_pushinteger(SPR_STATE[index], index);
		lua_pcall(SPR_STATE[index], 1, 0, 0); // run script
	}

	void process_all_sprites()
	{
		for (uint_fast8_t i = 0; i < 128; i++)
		{
			
			if (RAM[0x2000 + i] != 0) //If sprite exists..
			{
				if (RAM[0x2A80 + i] & 4 && !(RAM[0x2A80 + i] & 2))
				{
					RAM[0x2000 + i] = 0;
					continue;
				}
				if (RAM[0x2F80 + i] == 0)
				{
					if (RAM[0x2800 + i])
					{
						init_sprite_lua(int(i), "Code/Sprites/" + int_to_hex(RAM[0x2080 + i], true) + ".lua");
					}
					RAM[0x2F80 + i] = 1;
				}
				else
				{
					process_sprite_logic(uint_fast8_t(i));

					if (RAM[0x2000 + i] == 0)
					{
						continue;
					}
				}
				if (RAM[0x2800 + i])
				{
					call_sprite_lua(i);
				}
				else
				{
					ASM.load_asm("Code/Sprites/" + to_string(RAM[0x2080 + i]) + ".oasm");
					ASM.x = i;
					ASM.start_JFK_thread();
					if (ASM.crashed)
					{
						RAM[0x2000 + i] = 0;
					}

				
				}
				
			}
		}
	}
};


sprite_system Sprites;