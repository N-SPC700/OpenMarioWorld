#pragma once

#define P_METER_REQUIRED 0x70

class MPlayer
{
public:

	int_fast8_t SKIDDING = 0;
	int_fast8_t WALKING_DIR = -1;

	bool CROUCH = false;
	uint_fast8_t INVINCIBILITY_FRAMES = 0;

	bool invisible = false;
	bool ON_FL = false; //on floor
	bool IN_WT = false; //on water
	bool OLD_WT = false; //old on water status
	uint_fast8_t SLOPE_TYPE = 0;
	/*
		0 : None
		1 : Right 45*
		2 : Left 45*
	*/
	uint_fast8_t P_METER = 0;

	double X_SPEED = 0.0;
	double Y_SPEED = 0.0;

	double LAST_X_SPEED_ON_FL = 0.0;

	double CAMERA_X = 0.0;
	double CAMERA_Y = 0.0;
	double FRM = 0;
	int_fast8_t to_scale = 1;

	double height = 14.0;
	double x = 16.0;
	double y = 16.0;

	uint_fast8_t player_index = 0;
	uint_fast8_t skin = 0;
	uint_fast8_t STATE = 0;

	uint_fast8_t WO_counter = 0;
	uint_fast8_t KO_counter = 0;

	uint_fast16_t mouse_x = 0;
	uint_fast16_t mouse_y = 0;
	bool mouse_state[4]; //left, right, mousewup, mousewdown

	uint_fast8_t flash_t = 0; //xxxxTTTT
	int_fast16_t flash_x = 0;
	int_fast16_t flash_y = 0;

	char player_name_cut[player_name_size] = "       ";

	uint_fast8_t GRABBED_SPRITE = 0xFF; //A sprite index from 0 to 7F

	bool DEAD = false;
	bool pressed_y = false;
	bool old_y = false;

	bool in_pipe = false;
	bool jump_is_spin = false; //If a jump is a spinjump
	int_fast8_t pipe_speed_x = 0;
	int_fast8_t pipe_speed_y = 0;


	int_fast8_t DEATH_TIMER = 0;

	bool PlayerControlled = false;

	bool pad[total_inputs] = { false,false,false,false,false,false,false };
	bool was_jumpin = false;

	bool CAN_SPRINT = false;
	string sprite = "STAND_0";

	//Chat strings (THIS IS ONLY SENT. NOT RECEIVED!)
	string curr_chat_string = "";

	//This is not sent, the server will compare curr_chat_string to this, and then update the thing.
	string last_chat_string = "";

	//Sounds
	MPlayer(double newX = 0.0, double newY = 0.0)
	{
		x = newX;
		y = newY;

		CAMERA_X = x;
		CAMERA_Y = y;
	}

	void Chat(string new_c)
	{
		curr_chat_string = "<";
		curr_chat_string += username.substr(0, 5);
		curr_chat_string += "> " + new_c;
	}

	void Die()
	{
		if (!DEAD)
		{
			if (RAM[0x1493] == 0)
			{
				WO_counter += 1;
				ASM.Write_To_Ram(0x1DFC, 100, 1);
			}
			DEAD = true;
			DEATH_TIMER = 27;
		}
	}

	bool Hurt()
	{
		if (!DEAD && INVINCIBILITY_FRAMES == 0)
		{

			if (STATE != 0)
			{
				//preserve this line below it hasnt been changed since 2018 lol
				//PlaySoundF("hurt.ogg", 2);


				STATE = 0;
				INVINCIBILITY_FRAMES = 90;
				ASM.Write_To_Ram(0x1DF9, 4, 1);
			}
			else
			{
				Die();
				return true;
			}
		}
		return false;

	}

	void Respawn()
	{
		x = LevelManager.start_x;
		y = LevelManager.start_y;


		flash_t = 0;
		DEATH_TIMER = 0;
		DEAD = false;
		STATE = 0;
		X_SPEED = 0;
		Y_SPEED = 0;
		in_pipe = false;
		jump_is_spin = false;
	}

	void getInput()
	{
		//Controller buttons currently held down. Format: byetUDLR.
		//b = A or B; y = X or Y; e = select; t = Start; U = up; D = down; L = left, R = right.

		if (RAM[0x1493] > 0 || Chatting)
		{
			for (int inputs = 0; inputs < total_inputs; inputs++)
			{
				pad[inputs] = false;
			}
			return;
		}
		if (PlayerControlled)
		{
			pad[button_y] = (state[input_settings[0]] || state[input_settings[3]]) || (BUTTONS_GAMEPAD[7] || BUTTONS_GAMEPAD[6]);
			pad[button_b] = state[input_settings[1]] || BUTTONS_GAMEPAD[4];
			pad[button_a] = state[input_settings[2]] || BUTTONS_GAMEPAD[5];
			pad[button_left] = state[input_settings[4]] || BUTTONS_GAMEPAD[3];
			pad[button_right] = state[input_settings[5]] || BUTTONS_GAMEPAD[1];
			pad[button_down] = state[input_settings[6]] || BUTTONS_GAMEPAD[2];;
			pad[button_up] = state[input_settings[7]] || BUTTONS_GAMEPAD[0];
		}

	}

	void Enemy_Jump()
	{
		flash_t = 0x18; //15 time, type 1
		flash_x = int_fast16_t(x);
		flash_y = int_fast16_t(y - 24.0);
		if (!jump_is_spin)
		{
			if (pad[button_b])
			{
				Y_SPEED = Calculate_Speed(1408);
			}
			else
			{
				Y_SPEED = Calculate_Speed(800);
			}
		}
	}

	void Enemy_Jump_Spin()
	{
		flash_t = 0x18; //15 time, type 1
		flash_x = int_fast16_t(x);
		flash_y = int_fast16_t(y - 24.0);
		ASM.Write_To_Ram(0x1DF9, 0x2, 1);
		if (pad[button_b] || pad[button_a])
		{
			Y_SPEED = Calculate_Speed(1408);
		}
		else
		{
			Y_SPEED = Calculate_Speed(800);
		}
	}

	int DeathProcess()
	{

		x += X_SPEED;
		y += Y_SPEED;

		if (y < -16.0)
		{
			Respawn();
		}

		DEATH_TIMER -= 1;

		if (DEATH_TIMER == 0)
		{
			Y_SPEED = Calculate_Speed(1280.0);
		}
		if (DEATH_TIMER <= 0)
		{
			//cout << DEATH_TIMER % 5;
			if (DEATH_TIMER % 5 == -1)
			{
				to_scale *= -1;
			}
			Y_SPEED -= Calculate_Speed(48.0);
		}
		else
		{

			X_SPEED = 0.0;
			Y_SPEED = 0.0;
		}

		Get_Sprite();
		return 1;
	}

	void ProcessGrabbed()
	{
		if (GRABBED_SPRITE != 0xFF)
		{
			
			//cout << "Player 1 is holding sprite " << int(GRABBED_SPRITE) << endl;
			if (!in_pipe && ((!pad[button_y] || DEAD) || RAM[0x2000 + GRABBED_SPRITE] != 3))
			{
				
				uint_fast16_t x_position = uint_fast16_t(double(x + to_scale * -15.0));
				uint_fast16_t y_position = uint_fast16_t(double(y - (STATE > 0 ? 13.0 : 16.0)) + 17.0);
				RAM[0x2100 + GRABBED_SPRITE] = uint_fast8_t(x_position & 0xFF);
				RAM[0x2180 + GRABBED_SPRITE] = uint_fast8_t(x_position >> 8);
				RAM[0x2200 + GRABBED_SPRITE] = 0x00;

				RAM[0x2280 + GRABBED_SPRITE] = uint_fast8_t(y_position & 0xFF);
				RAM[0x2300 + GRABBED_SPRITE] = uint_fast8_t(y_position >> 8);
				RAM[0x2380 + GRABBED_SPRITE] = 0x00;
				RAM[0x2E00 + GRABBED_SPRITE] = 0x10;
				RAM[0x2780 + GRABBED_SPRITE] = 0;

				RAM[0x2000 + GRABBED_SPRITE] = 0x02;



				RAM[0x2480 + GRABBED_SPRITE] = 0;
				RAM[0x2400 + GRABBED_SPRITE] = uint_fast8_t(int_fast8_t(to_scale * -4)) + uint_fast8_t(int_fast8_t(X_SPEED * 16.0));

				if (pad[button_up])
				{
					RAM[0x2480 + GRABBED_SPRITE] = 0x70;
					RAM[0x2400 + GRABBED_SPRITE] = uint_fast8_t(int_fast8_t(X_SPEED * 8.0));
					ASM.Write_To_Ram(0x1DF9, 0x3, 1);
				}
				if (!pad[button_up])
				{
					if ((RAM[0x2E80 + GRABBED_SPRITE] > 0x80 && (!pad[button_down])) || (pad[button_right] || pad[button_left]))
					{
						RAM[0x2680 + GRABBED_SPRITE] = int_fast8_t(to_scale);
						RAM[0x2000 + GRABBED_SPRITE] = 0x04;
						RAM[0x2480 + GRABBED_SPRITE] = 0x00;
						ASM.Write_To_Ram(0x1DF9, 0x3, 1);
					}
				}
				GRABBED_SPRITE = 0xFF;
			}
		}
	}

	bool Move(double xMove, double yMove, bool do_change = false)
	{
		double NewPositionX = x + xMove;
		double NewPositionY = y + yMove;
		bool willreturn = true;
		int_fast16_t PosXBlock = int_fast16_t(NewPositionX / 16);
		int_fast16_t PosYBlock = int_fast16_t(NewPositionY / 16);
		int_fast16_t startX = PosXBlock - TotalBlocksCollisionCheck;
		if (startX < 0)
			startX = 0;
		int_fast16_t startY = PosYBlock - TotalBlocksCollisionCheck;
		if (startY < 0)
			startY = 0;

		/*
			Enemy Collision

			0x2000 - Sprite Status
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
			0x2600 - Sprite Flags HSGO---
			0x2680 - Sprite Direction


			H = Hurts
			S = Solid
			G = Gravity
			O = Offscreen Processing
		*/
		for (uint_fast8_t sprite = 0; sprite < 128; sprite++)
		{
			if (RAM[0x2000 + sprite] != 0)
			{
				double sprite_x = double(RAM[0x2100 + sprite] + int(int_fast8_t(RAM[0x2180 + sprite])) * 256) + double(RAM[0x2200 + sprite]) / 256.0;
				double sprite_y = double(RAM[0x2280 + sprite] + int(int_fast8_t(RAM[0x2300 + sprite])) * 256) + double(RAM[0x2380 + sprite]) / 256.0;
				double sprite_x_size = double(RAM[0x2500 + sprite]);
				double sprite_y_size = double(RAM[0x2580 + sprite]);

				double BelowSprite = sprite_y - height;
				double AboveSprite = sprite_y + sprite_y_size;
				double RightSprite = sprite_x + sprite_x_size;
				double LeftSprite = sprite_x - 16.0;

				bool checkRight = RAM[0x2600 + sprite] & 0b1;
				bool checkLeft = RAM[0x2600 + sprite] & 0b10;
				bool checkBottom = RAM[0x2600 + sprite] & 0b100;
				bool checkTop = RAM[0x2600 + sprite] & 0b1000;

				bool results[4] = { false,false,false,false };



				if (NewPositionX < RightSprite && NewPositionX > LeftSprite && NewPositionY < AboveSprite && NewPositionY > BelowSprite)
				{
					if (RAM[0x2600 + sprite] & 0b10000000)
					{
						if (RAM[0x2600 + sprite] & 0b10000)
						{
							if (NewPositionY > (AboveSprite - bounds_y))
							{
								Enemy_Jump();
								NewPositionY += 1;
								if (jump_is_spin)
								{
									if (RAM[0x2880 + sprite] & 1)
									{
										if (RAM[0x2880 + sprite] & 2)
										{
											RAM[0x2A00 + sprite] = 0x01;
										}
										else
										{
											RAM[0x2000 + sprite] = 0;
										}
										Y_SPEED = Calculate_Speed(128);
										ASM.Write_To_Ram(0x1DF9, 0x8, 1);
									}
									else
									{
										Enemy_Jump_Spin();
									}
								}
								else
								{
									results[2] = true;
								}
							}
							else
							{
								Hurt();
							}
						}
						else
						{
							if (jump_is_spin && NewPositionY > (AboveSprite - bounds_y))
							{
								Enemy_Jump_Spin();
							}
							else
							{
								Hurt();
							}
						}
					}

					if (checkRight && xMove < 0.0 && NewPositionX < RightSprite && NewPositionX > RightSprite - bounds_x)
					{
						NewPositionX = RightSprite;
						willreturn = false;
						results[0] = true;
					}
					if (checkLeft && xMove > 0.0 && NewPositionX > LeftSprite && NewPositionX < LeftSprite + bounds_x)
					{
						NewPositionX = LeftSprite;
						willreturn = false;
						results[1] = true;
					}
					if (checkTop && yMove < 0.0 && NewPositionY < AboveSprite && NewPositionY > AboveSprite - bounds_y)
					{
						willreturn = false;
						if (Y_SPEED <= 0)
						{
							NewPositionY = AboveSprite;
							results[2] = true;
						}
						if (do_change)
						{
							NewPositionX += double(int_fast8_t(RAM[0x2400 + sprite]) * 16) / 256.0;
							NewPositionY += double(int_fast8_t(RAM[0x2480 + sprite]) * 16) / 256.0;
							xMove = double(int_fast8_t(RAM[0x2400 + sprite]) * 16) / 256.0;
						}


					}
					if (checkBottom && yMove > 0.0 && NewPositionY > BelowSprite && NewPositionY < BelowSprite + bounds_y)
					{
						NewPositionY = BelowSprite;
						willreturn = false;
						results[3] = true;
					}

					RAM[0x2700 + sprite] = 0;
					for (uint_fast8_t i = 0; i < 4; i++)
					{
						RAM[0x2700 + sprite] += results[i] << i;
					}

					//Death kick
					if (RAM[0x2880 + sprite] & 4)
					{
						RAM[0x2000 + sprite] = 0;

						ASM.Write_To_Ram(0x1DF9, 3, 1);
					}

					//Powerups
					if (RAM[0x2000 + sprite] == 5)
					{

						if (!networking)
						{
							RAM[0x2000 + sprite] = 0;

							if (STATE == 0)
							{
								INVINCIBILITY_FRAMES = 20;
							}
							STATE = 1;
							ASM.Write_To_Ram(0x1DF9, 0xA, 1);
							
						}
						else
						{
							if (STATE == 0)
							{
								STATE = 1;
								ASM.Write_To_Ram(0x1DF9, 0xA, 1);
								INVINCIBILITY_FRAMES = 20;
							}
						}
					}

					//Grabbing
					if (GRABBED_SPRITE == 0xFF)
					{
						if (RAM[0x2000 + sprite] == 2 && RAM[0x2E00 + sprite] == 0)
						{
							if (pad[button_y])
							{
								RAM[0x2000 + sprite] = 3;
								GRABBED_SPRITE = sprite;
							}
							else
							{
								if ((RAM[0x2880 + sprite] & 2) && jump_is_spin && RAM[0x2080 + sprite] != 0x53)
								{
									RAM[0x2A00 + sprite] = 0x01;
									RAM[0x2000 + sprite] = 0x01;
									Y_SPEED = Calculate_Speed(128);
									ASM.Write_To_Ram(0x1DF9, 0x8, 1);
								}
								else
								{
									RAM[0x2680 + sprite] = int_fast8_t(to_scale);
									RAM[0x2000 + sprite] = 4;
									RAM[0x2E00 + sprite] = 0x10;

									ASM.Write_To_Ram(0x1DF9, 3, 1);

									flash_t = 0x18; //15 time, type 1
									flash_x = int_fast16_t(x + X_SPEED);
									flash_y = int_fast16_t(y - 16.0);
								}
							}
						}
					}

				}
			}
		}

		/*
		Shitty block collision. Surprised this doesn't lag.
		*/
		for (int_fast16_t xB = startX; xB < PosXBlock + TotalBlocksCollisionCheck; xB++)
		{
			for (int_fast16_t yB = startY; yB < PosYBlock + TotalBlocksCollisionCheck + (STATE > 0); yB++)
			{
				map16_handler.update_map_tile(xB, yB);


				double f_h = map16_handler.ground_y(NewPositionX + 8.0 - (xB * 16), xB, yB);
				double f_w = map16_handler.ground_s(xB, yB);
				double BelowBlock = double(yB * 16) + (f_h - 16.0) - (height) - 1;
				double AboveBlock = double(yB * 16) + (f_h) - 1;
				double RightBlock = double(xB * 16) + f_w;
				double LeftBlock = double(xB * 16) - f_w;

				bool checkLeft = map16_handler.logic[3];
				bool checkRight = map16_handler.logic[2];
				bool checkBottom = map16_handler.logic[1];
				bool checkTop = map16_handler.logic[0];

				bool playHitSound = true;

				if (IN_WT && yMove > 0.0 && !RAM[0x85])
				{
					if (map16_handler.get_tile(xB, yB) == 0x25)
					{
						if (pad[button_up] && (pad[button_b] || pad[button_a]))
						{
							Y_SPEED = Calculate_Speed(0x500 + (STATE > 0) * 0x80);
							jump_is_spin = pad[button_a];

						}
						else
						{
							checkBottom = true;
							playHitSound = false;
						}
					}
				}

				if (NewPositionX < RightBlock && NewPositionX > LeftBlock && NewPositionY < AboveBlock && NewPositionY > BelowBlock)
				{
					if (map16_handler.logic[4])
					{
						Hurt();
					}

					map16_handler.process_block(xB, yB, inside);
					if (xMove < 0.0 && checkRight)
					{
						if (NewPositionX > RightBlock - bounds_x)
						{
							NewPositionX = RightBlock;
							willreturn = false;

							map16_handler.process_block(xB, yB, right, pressed_y);

							if (pad[button_left])
							{
								if (
									map16_handler.get_tile(xB, yB) == 0x13F &&
									ON_FL
									)
								{
									in_pipe = true;
									pipe_speed_x = -2;
									pipe_speed_y = 0;
									
									ASM.Write_To_Ram(0x1DF9, 0x4, 1);
								}
							}
						}
					}
					if (xMove > 0.0 && checkLeft)
					{
						if (NewPositionX < LeftBlock + bounds_x)
						{
							NewPositionX = LeftBlock;
							willreturn = false;
							map16_handler.process_block(xB, yB, left, pressed_y);

							if (pad[button_right])
							{
								if (
									map16_handler.get_tile(xB, yB) == 0x13F &&
									ON_FL
									)
								{
									in_pipe = true;
									pipe_speed_x = 2;
									pipe_speed_y = 0;
									ASM.Write_To_Ram(0x1DF9, 0x4, 1);
								}
							}
						}
					}
					if (yMove < 0.0 && checkTop)
					{
						double bound_y = bounds_y;
						uint_fast8_t new_s = map16_handler.get_slope(xB, yB);
						if (new_s != 0)
						{
							bound_y += 2;
						}
						if (NewPositionY > AboveBlock - bound_y)
						{
							willreturn = false;

							
							if (new_s != 0 && SLOPE_TYPE == 0)
							{
								SLOPE_TYPE = new_s;
							}

							if (Y_SPEED <= 0)
							{
								NewPositionY = AboveBlock;
							}

							map16_handler.process_block(xB, yB, top, pressed_y);
							if (pad[button_down])
							{
								if (
									map16_handler.get_tile(uint_fast16_t((NewPositionX + 4.0) / 16.0), yB) == 0x137 &&
									map16_handler.get_tile(uint_fast16_t((NewPositionX + 12.0) / 16.0), yB) == 0x138
									)
								{
									in_pipe = true;
									pipe_speed_x = 0;
									pipe_speed_y = -2;
									ASM.Write_To_Ram(0x1DF9, 0x4, 1);
								}
							}
						}
					}
					if (yMove > 0.0 && checkBottom)
					{
						if (NewPositionY < BelowBlock + bounds_y)
						{
							NewPositionY = BelowBlock;
							willreturn = false;


							if (playHitSound)
							{
								ASM.Write_To_Ram(0x1DF9, 0x1, 1);
							}
							map16_handler.process_block(xB, yB, bottom);
							if (pad[button_up])
							{
								if (
									map16_handler.get_tile(uint_fast16_t((NewPositionX + 4.0) / 16.0), yB) == 0x137 &&
									map16_handler.get_tile(uint_fast16_t((NewPositionX + 12.0) / 16.0), yB) == 0x138
									)
								{
									in_pipe = true;
									pipe_speed_x = 0;
									pipe_speed_y = 2;
									
									ASM.Write_To_Ram(0x1DF9, 0x4, 1);
								}
							}
						}
					}

					if (spawned_grabbable != 0xFF)
					{
						GRABBED_SPRITE = spawned_grabbable;
						RAM[0x2000 + GRABBED_SPRITE] = 0x03;
					}
					spawned_grabbable = 0xFF;
				}
			}
		}

		x = NewPositionX;
		y = NewPositionY;
		return willreturn;
	}
	void Get_Sprite()
	{
		if (DEAD)
		{
			sprite = "DEAD";
			return;
		}
		string NewSprite = "STAND";
		if (jump_is_spin && !ON_FL)
		{
			switch ((global_frame_counter / 2) % 4) {
			case 0:
				NewSprite = "PIPE";
				to_scale = 1;
				break;
			case 1:
				NewSprite = "STAND";
				to_scale = 1;
				break;
			case 2:
				NewSprite = "BACK";
				to_scale = 1;
				break;
			case 3:
				NewSprite = "STAND";
				to_scale = -1;
				break;
			}

			sprite = NewSprite + "_" + to_string(STATE);
			return;
		}
		if (in_pipe)
		{
			sprite = "PIPE_" + to_string(STATE);
			return;
		}
		if (!CROUCH)
		{
			if (!ON_FL)
			{
				if (IN_WT)
				{
					NewSprite = "SWIM";
				}
				else
				{
					if (CAN_SPRINT)
					{
						NewSprite = "JUMPB";
					}
					else
					{
						if (Y_SPEED > 0.0)
						{
							NewSprite = "JUMP";
						}
						else
						{
							NewSprite = "FALL";
						}
					}
				}
			}
			else
			{
				if (SKIDDING == 0.0)
				{
					if (X_SPEED != 0)
					{
						FRM += X_SPEED / 5;
						int Frame = abs(int(FRM) % (2 + (STATE > 0)));
						if (CAN_SPRINT)
						{
							NewSprite = "RUN" + to_string(Frame);
						}
						else
						{
							NewSprite = "WALK" + to_string(Frame);
						}
					}
				}
				else
				{
					NewSprite = "SKID";
				}
			}
		}
		else
		{
			NewSprite = "CROUCH";
		}
		if (WALKING_DIR > 0)
		{
			to_scale = -1;
		}
		if (WALKING_DIR < 0)
		{
			to_scale = 1;
		}
		sprite = NewSprite + "_" + to_string(STATE);
	}

	void in_pipe_process()
	{
		y += pipe_speed_y * 3;
		x += pipe_speed_x * 3;
		uint_fast16_t check_y_1 = uint_fast16_t((y + height - 1) / 16.0);
		uint_fast16_t check_y_2 = uint_fast16_t((y + 1) / 16.0);
		uint_fast16_t check_x_1 = uint_fast16_t((x)/16.0);
		uint_fast16_t check_x_2 = uint_fast16_t((x + 16.0) / 16.0);
		CROUCH = false;
		X_SPEED = 0;
		Y_SPEED = 0;
		if (
			map16_handler.get_tile(check_x_1, check_y_1) < 0x100 && map16_handler.get_tile(check_x_1, check_y_2) < 0x100 &&
			map16_handler.get_tile(check_x_2, check_y_1) < 0x100 && map16_handler.get_tile(check_x_2, check_y_2) < 0x100
			
			)
		{
			in_pipe = false;
			pipe_speed_x = 0;
			pipe_speed_y = 0;


			ASM.Write_To_Ram(0x1DF9, 0x4, 1);
		}
		invisible = true;

	}

	void FlashProcess()
	{
		uint_fast8_t flash_timer = flash_t & 0xF;
		uint_fast8_t flash_type = flash_t >> 4;

		if (flash_timer > 0)
		{
			uint_fast16_t oam_index = 0;
			while (oam_index < 0x400)
			{
				if (RAM[0x200 + oam_index] == 0 && RAM[0x206 + oam_index] == 0) { //Empty OAM slot found
					break;
				}
				oam_index += 8;
			}


			if (flash_type == 1)
			{
				RAM[0x200 + oam_index] = 0x44;
				RAM[0x201 + oam_index] = 0x11;

				RAM[0x202 + oam_index] = flash_x;
				RAM[0x203 + oam_index] = flash_x >> 8;

				RAM[0x204 + oam_index] = flash_y;
				RAM[0x205 + oam_index] = flash_y >> 8;

				RAM[0x206 + oam_index] = 0x08 | (((flash_timer / 2) % 2) * 0x20);
				RAM[0x207 + oam_index] = 0;
				flash_timer -= 1;
			}
			else
			{
				RAM[0x200 + oam_index] = 0x60 + ((15 - flash_timer)/4)*2;
				RAM[0x201 + oam_index] = 0x11;

				RAM[0x202 + oam_index] = flash_x;
				RAM[0x203 + oam_index] = flash_x >> 8;

				RAM[0x204 + oam_index] = flash_y;
				RAM[0x205 + oam_index] = flash_y >> 8;

				RAM[0x206 + oam_index] = 0x08;
				RAM[0x207 + oam_index] = 0;

				if (!(global_frame_counter % 2))
				{
					flash_timer -= 1;
				}
			}

			
		}
		flash_t = flash_timer + (flash_type << 4);
	}

	int Process()
	{
		//X_SPEED = 10;
		if (RAM[0x1493] > 0 && RAM[0x1493] < 8)
		{
			CAMERA_X /= 2;
			CAMERA_Y /= 2;
			if (!DEAD)
			{
				Die(); //This is so we can get warped
			}
		}
		SLOPE_TYPE = 0;

		getInput();
		if (pad[button_y] != old_y) {
			old_y = pad[button_y];
			if (old_y) {
				pressed_y = true;
			}
		}

		height = (STATE > 0 && CROUCH == 0) ? 28.0 : 14.0;

		if (DEAD) {
			return DeathProcess();
		}

		if (y < -16.0) {
			Die();
		}

		if (INVINCIBILITY_FRAMES > 0) {
			INVINCIBILITY_FRAMES -= 1;
		}
		invisible = INVINCIBILITY_FRAMES > 0 ? !invisible : false;

		double GRAV = -double(RAM[0x7C]);
		bool RUN = false;
		bool MOV = false;
		bool SLIGHT_HIGH_SPEED = false;


		
		uint_fast16_t check_x_1 = uint_fast16_t((x + 8) / 16.0);
		uint_fast16_t check_y_1 = uint_fast16_t((y + height) / 16.0);

		IN_WT = RAM[0x85] != 0 || map16_handler.get_tile(check_x_1, check_y_1) < 4;

		if (IN_WT != OLD_WT)
		{
			OLD_WT = IN_WT;
			if (IN_WT)
			{
				Y_SPEED = 0;
				X_SPEED = 0;
				P_METER = 0;

				flash_t = 0x2F; //15 time, type 1
				flash_x = int_fast16_t(x);
				flash_y = int_fast16_t(y - (STATE == 0) * 16.0);
			}
		}


		
		if (in_pipe) {
			in_pipe_process();
		}
		else
		{
			if (!IN_WT)
			{
				if (ON_FL)
				{
					CAN_SPRINT = false;
				}

				ON_FL = false;
				if (!Move(0.0, -1.0, true)) { //Detected a floor below

					if (Y_SPEED <= 0)
					{
						ON_FL = true;
						jump_is_spin = false;
					}
					else
					{
						y += 1;
					}
				}
				else {
					y += 1;
				}

				SKIDDING = 0;
				if (abs(X_SPEED) >= Calculate_Speed(576.0)) {

					SLIGHT_HIGH_SPEED = true;
					if (pad[button_y] && (pad[button_left] || pad[button_right]))
					{

						if (CAN_SPRINT || (ON_FL && !CROUCH)) {
							P_METER += 2;
							if (P_METER > P_METER_REQUIRED) {
								P_METER = P_METER_REQUIRED;
							}
						}
						else
						{
							if (P_METER > 0) {
								P_METER -= 1;
							}
						}
					}
					else
					{
						if (P_METER > 0) {
							P_METER -= 1;
						}
					}
				}
				else {
					if (P_METER > 0) {
						P_METER -= 1;
					}
				}

				if (P_METER >= (P_METER_REQUIRED - 1) && pad[button_y]) {
					CAN_SPRINT = true;
				}
				if (pad[button_left]) {
					WALKING_DIR = -1;
					MOV = true;
				}
				if (pad[button_right]) {
					WALKING_DIR = 1;
					MOV = true;
				}
				if (pad[button_down]) {
					if (ON_FL) {
						WALKING_DIR = 0;
						MOV = true;
						CROUCH = true;
					}
				}
				else {
					if (ON_FL) {
						CROUCH = false;
					}
				}
				if (pad[button_y]) {
					RUN = true;
				}


				if ((pad[button_a] || pad[button_b]) != was_jumpin) {
					was_jumpin = pad[button_a] || pad[button_b];
					if (was_jumpin && ON_FL) {
						if (pad[button_a] && GRABBED_SPRITE == 0xFF)
						{
							//Spinjump
							Y_SPEED = Calculate_Speed(1136.0 + (abs(X_SPEED) * 64.0)); //(148.0 * SLIGHT_HIGH_SPEED) + (32.0 * (X_SPEED > Calculate_Speed(320+256+176)))
							ASM.Write_To_Ram(0x1DFC, 0x04, 1);

							jump_is_spin = true;
						}
						else
						{
							//Normal jump
							Y_SPEED = Calculate_Speed(1232.0 + (abs(X_SPEED) * 64.0)); //(148.0 * SLIGHT_HIGH_SPEED) + (32.0 * (X_SPEED > Calculate_Speed(320+256+176)))
							ASM.Write_To_Ram(0x1DFA, 0x01, 1);

							//jump_is_spin = false;
						}
					}
				}
				if (pad[button_a] || pad[button_b]) {
					GRAV = GRAV * 0.5;
				}

				double SLOPE_ADD = 0;
				if (!CROUCH)
				{
					if (SLOPE_TYPE == 1)
					{
						if (!MOV || (WALKING_DIR == -1 && MOV)) { SLOPE_ADD = Calculate_Speed(-256); }
						if (MOV) { SLOPE_ADD = Calculate_Speed(-80); }
					}
					if (SLOPE_TYPE == 2)
					{
						if (!MOV || (WALKING_DIR == 1 && MOV)) { SLOPE_ADD = Calculate_Speed(256); }
						if (MOV) { SLOPE_ADD = Calculate_Speed(80); }
					}
				}
				double SPEED_X_TO_SET = SLOPE_ADD + (Calculate_Speed(320.0 + (RUN * 256.0) + (CAN_SPRINT * 192.0)) * WALKING_DIR) * MOV;
				double SPEED_ACCEL_X = Calculate_Speed(24.0);
				double STOPPING_DECEL = Calculate_Speed(16.0);
				double SKID_ACCEL = Calculate_Speed(16.0 + (24.0 * RUN) + (CAN_SPRINT * 40.0));

				/*
				Accel start
				*/
				if (MOV) {
					if (X_SPEED > 0.0 && SPEED_X_TO_SET < 0.0 && WALKING_DIR == -1) {
						SKIDDING = -1;
						X_SPEED -= SKID_ACCEL;
					}
					if (X_SPEED < 0.0 && SPEED_X_TO_SET > 0.0 && WALKING_DIR == 1) {
						SKIDDING = 1;
						X_SPEED += SKID_ACCEL;
					}
					if (!ON_FL) {
						SKIDDING = 0;
					}
					if (X_SPEED > SPEED_X_TO_SET) {
						X_SPEED = max(SPEED_X_TO_SET, X_SPEED - SPEED_ACCEL_X);
					}

					if (X_SPEED < SPEED_X_TO_SET) {
						X_SPEED = min(SPEED_X_TO_SET, X_SPEED + SPEED_ACCEL_X);
					}
				}
				else {
					if (ON_FL == 1.0) {
						if (!SLOPE_TYPE)
						{
							if (X_SPEED > 0.0) {
								X_SPEED = max(0, X_SPEED - STOPPING_DECEL);
							}
							if (X_SPEED < 0.0) {
								X_SPEED = min(0, X_SPEED + STOPPING_DECEL);
							}
						}
						else
						{

							if (X_SPEED > SPEED_X_TO_SET) {
								X_SPEED = max(SPEED_X_TO_SET, X_SPEED - SPEED_ACCEL_X);
							}

							if (X_SPEED < SPEED_X_TO_SET) {
								X_SPEED = min(SPEED_X_TO_SET, X_SPEED + SPEED_ACCEL_X);
							}
						}
					}
				}
				/*
					Accel end
				*/


				if (!ON_FL || SLOPE_TYPE) {
					Y_SPEED += Calculate_Speed(GRAV);
				}
				else {
					LAST_X_SPEED_ON_FL = X_SPEED;
				}
				if (Y_SPEED < Calculate_Speed(-1312.0)) {
					Y_SPEED = Calculate_Speed(-1312.0);
				}

				if (!Move(X_SPEED + Calculate_Speed(double(int_fast8_t(RAM[0x7B])) * 16.0), 0.0)) {
					X_SPEED = 0.0;
				}
				if (!Move(0.0, Y_SPEED + Calculate_Speed(double(int_fast8_t(RAM[0x7D])) * 16.0))) {
					if (SLOPE_TYPE)
					{
						if (Y_SPEED > 0) { Y_SPEED = 0; }
						if (Y_SPEED < Calculate_Speed(-768)) { Y_SPEED = Calculate_Speed(-768); }
					}
					else
					{
						Y_SPEED = 0;
					}
				}
			}
			else
			{
				/*
				SWIMCODE
				*/

				jump_is_spin = false;
				ON_FL = false;
				if (!Move(0.0, -1.0, true)) { //Detected a floor below

					if (Y_SPEED <= 0)
					{
						ON_FL = true;
						jump_is_spin = false;
					}
					else
					{
						y += 1;
					}
				}
				else {
					y += 1;
				}

				SLIGHT_HIGH_SPEED = false;
				CAN_SPRINT = false;
				MOV = false;
				CROUCH = false;

				if (pad[button_left]) {
					WALKING_DIR = -1;
					MOV = true;
				}
				if (pad[button_right]) {
					WALKING_DIR = 1;
					MOV = true;
				}
				if (pad[button_down]) {
					if (ON_FL && GRABBED_SPRITE == 0xFF) {
						WALKING_DIR = 0;
						MOV = true;
						CROUCH = true;
					}
				}

				if ((pad[button_a] || pad[button_b]) != was_jumpin) {
					was_jumpin = pad[button_a] || pad[button_b];
					if (was_jumpin) {
						RAM[0x1DF9] = 0x0E;
						Y_SPEED = 10.0;
					}
				}

				double SPEED_X_TO_SET = Calculate_Speed(256.0 / (1.0 + double(ON_FL))) * WALKING_DIR;
				double SPEED_ACCEL_X = Calculate_Speed(24.0);
				double STOPPING_DECEL = Calculate_Speed(24.0);
				double SKID_ACCEL = Calculate_Speed(40.0);
				if (GRABBED_SPRITE == 0xFF)
				{
					Y_SPEED = Y_SPEED + Calculate_Speed(-8);
					Y_SPEED = max(Calculate_Speed(-1024), min(Y_SPEED, Calculate_Speed(384.0 - (pad[button_down] * 256.0) + (pad[button_up] * 384.0))));
				}
				else
				{
					Y_SPEED = min(1, Y_SPEED + Calculate_Speed(8));
					MOV = true;
					double sp = 256 + (pad[button_right] || pad[button_left]) * 256;
					SPEED_ACCEL_X = Calculate_Speed(16.0);
					SPEED_X_TO_SET = Calculate_Speed(sp / (1.0 + double(ON_FL)))* WALKING_DIR;

					if (pad[button_down])
					{
						Y_SPEED = -1;
						if (!(global_frame_counter % 8))
						{
							RAM[0x1DF9] = 0x0E;
						}
					}
				}


				/*
				Accel start
				*/
				if (MOV) {
					if (X_SPEED > SPEED_X_TO_SET) {
						X_SPEED = max(SPEED_X_TO_SET, X_SPEED - SPEED_ACCEL_X);
					}
					if (X_SPEED < SPEED_X_TO_SET) {
						X_SPEED = min(SPEED_X_TO_SET, X_SPEED + SPEED_ACCEL_X);
					}
					if (X_SPEED > 0.0 && SPEED_X_TO_SET < 0.0 && WALKING_DIR == -1) {
						X_SPEED -= SKID_ACCEL;
					}
					if (X_SPEED < 0.0 && SPEED_X_TO_SET > 0.0 && WALKING_DIR == 1) {
						X_SPEED += SKID_ACCEL;
					}
				}
				else {
					if (X_SPEED > 0.0) {
						X_SPEED = max(0, X_SPEED - STOPPING_DECEL);
					}
					if (X_SPEED < 0.0) {
						X_SPEED = min(0, X_SPEED + STOPPING_DECEL);
					}
				}
				/*
				Accel end
				*/


				if (!Move(X_SPEED + Calculate_Speed(double(int_fast8_t(RAM[0x7B])) * 16.0), 0.0)) {
					X_SPEED = 0.0;
				}
				if (!Move(0.0, Y_SPEED + Calculate_Speed(double(int_fast8_t(RAM[0x7D])) * 16.0))) {
					Y_SPEED = 0;
				}
			}
			/*
				No more swimming
			*/
		}
		pressed_y = false;
		Get_Sprite();


		if (x < 8.0) { x = 8.0; }
		if (x > double(-24 + mapWidth * 16)) { x = double(-24 + mapWidth * 16); }

		ProcessGrabbed();
		FlashProcess();
		return 1;
	}

	void ProcessCamera() {

		if (RAM[0x1411] == 0)
		{
			int min_x = RAM[0x1462] + RAM[0x1463] * 256;
			if (int(x) < (min_x + 8))
			{
				x = double(min_x + 8);
			}
			if (int(x) > (min_x + 256 - 24))
			{
				x = double(min_x + 256 - 24);
			}
			CAMERA_X = double(120 + RAM[0x1462] + RAM[0x1463] * 256);
		}
		else
		{
			//Camera lol

			if (RAM[0x1493] == 0)
			{
				double new_x = x;

				if (smooth_camera)
				{
					CAMERA_X += (new_x - CAMERA_X) / smooth_camera_speed;
				}
				else
				{
					if (CAMERA_X < new_x)
					{
						CAMERA_X += 5.0;
						if (CAMERA_X > new_x)
						{
							CAMERA_X = new_x;
						}
					}

					if (CAMERA_X > new_x)
					{
						CAMERA_X -= 5.0;
						if (CAMERA_X < new_x)
						{
							CAMERA_X = new_x;
						}
					}
				}

			}
		}

		if (RAM[0x1412] == 0) {
			CAMERA_Y = double(104 + RAM[0x1464] + RAM[0x1465] * 256);
		}
		else {
			if (RAM[0x1493] == 0)
			{
				double new_y = (y + 16);

				if (smooth_camera)
				{
					CAMERA_Y += (new_y - CAMERA_Y) / smooth_camera_speed;
				}
				else
				{
					if (CAMERA_Y < new_y)
					{
						CAMERA_Y += 4.0;
						if (CAMERA_Y > new_y)
						{
							CAMERA_Y = new_y;
						}
					}

					if (CAMERA_Y > new_y)
					{
						CAMERA_Y -= 4.0;
						if (CAMERA_Y < new_y)
						{
							CAMERA_Y = new_y;
						}
					}
				}
			}
		}
	}
};


//Mario management

list<MPlayer> Mario; //This is how much players exist. Starts at 0.
void AddNewPlayer() { MPlayer NewPlayer = MPlayer(LevelManager.start_x, LevelManager.start_y); Mario.push_back(NewPlayer); }
void RemovePlayer() { Mario.pop_back(); }
void CheckForPlayers() //Have to be careful when fucking with this. Or else memory might leak.
{
	uint_fast8_t player_am = uint_fast8_t(Mario.size());
	if (player_am != PlayerAmount) //example, player am is 0, and amount is 1.
	{
		int_fast8_t distance = PlayerAmount - player_am; //1 - 0 = 1, needs to add!
		while (distance > 0) { distance -= 1; AddNewPlayer(); }
		while (distance < 0) { distance += 1; RemovePlayer(); }

	}
}

// Get Mario

MPlayer& get_mario(uint_fast8_t number)
{
	CheckForPlayers();
	
	uint_fast8_t player = 1;
	for (list<MPlayer>::iterator item = Mario.begin(); item != Mario.end(); ++item)
	{
		if (player == number)
		{
			return *item;
		}
		player += 1;

	}

	return *Mario.begin();
}

//player interaction with other players, for now it's just mario combat
void PlayerInteraction()
{
	if (!pvp)
	{
		return;
	}
	for (list<MPlayer>::iterator curr_p = Mario.begin(); curr_p != Mario.end(); ++curr_p)
	{
		MPlayer& CurrPlayer = *curr_p;
		if (CurrPlayer.DEAD)
		{
			continue;
		}
		else
		{
			for (list<MPlayer>::iterator item = Mario.begin(); item != Mario.end(); ++item)
			{
				MPlayer& PlrInteract = *item;
				if ((&PlrInteract == &CurrPlayer || PlrInteract.DEAD) || PlrInteract.INVINCIBILITY_FRAMES > 0)
				{
					continue;
				}
				else
				{
					double LeftBound = PlrInteract.x - 16.0;
					double RightBound = PlrInteract.x + 16.0;
					double UpBound = PlrInteract.y + (PlrInteract.STATE == 0 ? 16.0 : 32.0);
					double DownBound = PlrInteract.y - (CurrPlayer.STATE == 0 ? 16.0 : 32.0);
					if (CurrPlayer.x > LeftBound && CurrPlayer.x < RightBound && CurrPlayer.y < UpBound && CurrPlayer.y > DownBound && CurrPlayer.Y_SPEED < -2.0)
					{
						if (CurrPlayer.jump_is_spin)
						{
							CurrPlayer.Enemy_Jump_Spin();
						}
						else
						{
							CurrPlayer.Enemy_Jump();
						}

						if (PlrInteract.Hurt())
						{
							CurrPlayer.KO_counter += 1;
						}
					}
				}

			}
		}

	}
}