#pragma once

double tick = 0;

void game_init()
{
	decode_graphics_file("Graphics/exanimations.bin", 8);
	decode_graphics_file("Graphics/hud.bin", 11);
	memset(&RAM[0x1B800], 0xFF, 0x800);
}

void game_loop_code()
{
	global_frame_counter += 1;


#if not defined(DISABLE_NETWORK)
	if (networking)
	{
		if (isClient && !(global_frame_counter % 60))
		{
			data_size_now = data_size_current;
			data_size_current = 0;
		}

		if ((clients.size() == 0 && !isClient))
		{
			return;
		}
	}
#endif
	if (!isClient && RAM[0x1493] > 0)
	{
		if ((global_frame_counter % 3) == 0)
		{
			RAM[0x1493] -= 1;
		}
		if (RAM[0x1493] == 0)
		{
			if (ASM.Get_Ram(0x3f08, 2) != 0)
			{
				midway_activated = false;
				LevelManager.LoadLevel(uint_fast16_t(ASM.Get_Ram(0x3f08, 2)));

				game_init();
#if not defined(DISABLE_NETWORK)
				Set_Server_RAM();
#endif
				Do_RAM_Change();

				ASM.Write_To_Ram(0x3f08, 0, 2);
			}
			else
			{
				ASM.Write_To_Ram(0x1DFB, 0, 1);
			}
		}
	}
	int total_time_ticks_d = min(65535, int(total_time_ticks.count() * 3584.0));
	uint_fast16_t count = uint_fast16_t(total_time_ticks_d);
	RAM[0x4207] = count & 0xFF;
	RAM[0x4209] = count / 256;





	if (!isClient && RAM[0x1887] > 0)
	{
		RAM[0x1887]--;
	}

	if (RAM[0x1887] > 0)
	{
		if (!networking || isClient)
		{
			vibrate_controller(1.0, 32);
		}
	}

	mapWidth = RAM[0x3F00] + RAM[0x3F01] * 256;
	if (mapWidth == 0)
	{
		mapWidth = 256;
	}
	mapHeight = RAM[0x3F02] + RAM[0x3F03] * 256;

	RAM[0x3F0A] = networking;

	LevelManager.start_x = RAM[0x3F0B] + RAM[0x3F0C] * 256;
	LevelManager.start_y = RAM[0x3F0D] + RAM[0x3F0E] * 256;

	CheckForPlayers();

	process_ex_animation();

	debugging_functions();

	if (!isClient)
	{
		for (uint_fast16_t i = 0; i < 256; i++)
		{
			if (death_timer[i] > 0) {
				death_timer[i]--;
				if (death_timer[i] == 0x80){
					death_timer[i] = 0;
				}
			}
		}
	}
	if (!isClient || !networking) //if we are the server or we are playing locally...
	{
		memset(&RAM[0x200], 0, 0x400);
		Sprites.process_all_sprites(); //we're processing sprites. we're either the server or a player in local mode.
		for (uint_fast8_t i = 0; i < 128; i++)
		{
			RAM[0x2A80 + i] &= 5;
		}
	}

	uint_fast8_t player = 1;
	int camera_total_x = 0; int camera_total_y = 0;

	for (list<MPlayer>::iterator item = Mario.begin(); item != Mario.end(); ++item)
	{
		MPlayer& CurrPlayer = *item;
		//cout << "Player " << dec << int(player) << " location is " << hex << &CurrPlayer << dec << endl;
		CurrPlayer.PlayerControlled = networking ? (player == SelfPlayerNumber) : true; //Only control myself
		if (!isClient && networking) {
			CurrPlayer.PlayerControlled = false;
		}
		if (CurrPlayer.PlayerControlled == true){
			CurrPlayer.mouse_x = mouse_x + CameraX;
			CurrPlayer.mouse_y = (int_res_y - mouse_y) + CameraY;
			CurrPlayer.mouse_state[0] = mouse_down;
			CurrPlayer.mouse_state[1] = mouse_down_r;
			CurrPlayer.mouse_state[2] = mouse_w_up;
			CurrPlayer.mouse_state[3] = mouse_w_down;

			CurrPlayer.skin = my_skin;
			for (int i = 0; i < player_name_size; i++)
			{
				if (i >= username.length())
				{
					CurrPlayer.player_name_cut[i] = ' ';
				}
				else
				{
					CurrPlayer.player_name_cut[i] = username.at(i);
				}
			}

			//Genius code
			if (death_timer[player - 1] > 0x00 && death_timer[player - 1] <= 0x7F)
			{
				CurrPlayer.Die();
			}
			if (death_timer[player - 1] > 0x80 && death_timer[player - 1] <= 0xFF)
			{
				CurrPlayer.Hurt();
			}
		}
		//cout << SelfPlayerNumber;
		CurrPlayer.player_index = player;
		CurrPlayer.Process();

		camera_total_x += max(0, int(CurrPlayer.CAMERA_X - 128.0));
		camera_total_y += max(0, int(CurrPlayer.CAMERA_Y - 112.0));

		uint_fast16_t x_r = uint_fast16_t(CurrPlayer.x);
		uint_fast16_t y_r = uint_fast16_t(CurrPlayer.y);
		uint_fast8_t x_s_r = uint_fast8_t(CurrPlayer.X_SPEED * 16.0);
		uint_fast8_t y_s_r = uint_fast8_t(CurrPlayer.Y_SPEED * 16.0);

		uint_fast16_t m_state_1 = (CurrPlayer.mouse_x & 0x3FFF) + (CurrPlayer.mouse_state[0] << 15) + (CurrPlayer.mouse_state[2] << 14);
		uint_fast16_t m_state_2 = (CurrPlayer.mouse_y & 0x3FFF) + (CurrPlayer.mouse_state[1] << 15) + (CurrPlayer.mouse_state[3] << 14);


		RAM[0x5000 + player - 1] = x_r;
		RAM[0x5100 + player - 1] = x_r >> 8;
		RAM[0x5200 + player - 1] = y_r;
		RAM[0x5300 + player - 1] = y_r >> 8;
		RAM[0x5400 + player - 1] = x_s_r;
		RAM[0x5500 + player - 1] = y_s_r;
		RAM[0x5600 + player - 1] = CurrPlayer.KO_counter;
		RAM[0x5700 + player - 1] = CurrPlayer.WO_counter;
		RAM[0x5800 + player - 1] = CurrPlayer.STATE;
		RAM[0x5900 + player - 1] = CurrPlayer.DEAD;
		RAM[0x5A00 + player - 1] = m_state_1;
		RAM[0x5B00 + player - 1] = m_state_1 >> 8;
		RAM[0x5C00 + player - 1] = m_state_2;
		RAM[0x5D00 + player - 1] = m_state_2 >> 8;

		if (!isClient && RAM[0x1493] == 0)
		{
			CheckSpritesInCam(int(max(128.0, CurrPlayer.CAMERA_X)));
		}

		player++;


	}
	RAM[0x3F0F] = uint_fast8_t(Mario.size());

	PlayerInteraction();
	ProcessChat();

	camera_total_x /= uint_fast32_t(Mario.size());
	camera_total_y /= uint_fast32_t(Mario.size());

	if (!isClient)
	{
		//cout << "clearing OAM";

		if (RAM[0x1411] != 0)
		{
			ASM.Write_To_Ram(0x1462, uint_fast32_t(camera_total_x), 2);
		}
		if (RAM[0x1412] != 0)
		{
			ASM.Write_To_Ram(0x1464, uint_fast32_t(camera_total_y), 2);
		}
		if (asm_loaded) {
			ASM.start_JFK_thread(); //This runs the ASM.
		}
		if (lua_loaded) {
			lua_run_main();
		}

		RAM[0x14] = global_frame_counter & 0xFF;
	}

	ProcessHDMA();

}

void game_loop()
{
	if (networking && !isClient)
	{
		game_loop_code();
		return;
	}
	if (monitor_hz == 60.0 || monitor_hz <= 0)
	{
		game_loop_code();
		return;
	}
	tick += 60.0 / monitor_hz;
	while (tick >= 1)
	{
		tick -= 1;
		game_loop_code();
	}
}