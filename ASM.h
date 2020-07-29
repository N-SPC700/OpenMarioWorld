#pragma once
//something something this fucking sucks

bool asm_loaded = false;

uint_fast8_t RAM[RAM_Size];
uint_fast8_t RAM_old[0x8000];
uint_fast8_t RAM_decay_time_level[0x4000]; //for multiplayer

class JFKASM
{
public:
	/*
	
	This is just a very shitty SNES 65C16 emulator. It can do most stuff fine, but it'll have issues with some things.
	
	*/
	uint_fast16_t x, y, a, temp_, temp__;
	uint_fast8_t st;
	uint_fast32_t pointer;
	uint_fast8_t access_size = 1;

	bool flags[4];

	uint_fast8_t rom[rom_asm_size]; //This is basically a read-only memory.
	bool crashed = false;

	void CMP()
	{
		for (uint_fast8_t i = 0; i < 4; i++)
		{
			flags[i] = false;
		}

		if (temp_ == temp__) { flags[0] = true; }
		if (temp_ != temp__) { flags[1] = true; }
		if (temp_ >= temp__) { flags[2] = true; }
		if (temp_ <= temp__) { flags[3] = true; }

	}
	void start_JFK_thread() //This runs the ASM.
	{
		pointer = 0;

		string broke_reason = "invalid opcode";
		while (true)
		{
			access_size = (st & 0x20) ? 2 : 1;

			uint_fast8_t opcode = rom[pointer];

			pointer += 1;
			switch (opcode) {

			/*
			
				Note : can someone help make this not look so bad? jesus christ

				LDA/X/Y (Value)
			*/
			case 0xA0:
			case 0xA2:
			case 0xA9:
				temp_ = 0;
				for (int i = 0; i < access_size; i++) {
					temp_ += rom[pointer + i] << (i * 8);
				}

				pointer += access_size;

				if (opcode == 0xA9) { a = temp_; } if (opcode == 0xA0) { x = temp_; } if (opcode == 0xA2) { y = temp_; }

				break;
			/*
				LDA/X/Y (RAM)
			*/
			case 0xA5:
			case 0xA6:
			case 0xA4:
			case 0xAD:
			case 0xAE:
			case 0xAC:
				temp_ = Get_Ram(rom[pointer] + (opcode < 0xAA ? 0 : (rom[pointer + 1] << 8)), access_size);
				pointer += (opcode < 0xAA ? 1 : 2);

				if (opcode == 0xAD) { a = temp_; } if (opcode == 0xAE) { x = temp_; } if (opcode == 0xAC) { y = temp_; }
				break;
			/*
				STA/X/Y (Value)
			*/
			case 0x85:
			case 0x86:
			case 0x84: //da fuq? its not 87?
			case 0x8D:
			case 0x8E:
			case 0x8C:
				if (opcode == 0x85) { temp_ = a; } if (opcode == 0x86) { temp_ = x; } if (opcode == 0x84) { temp_ = y; }
				if (opcode == 0x8D) { temp_ = a; } if (opcode == 0x8E) { temp_ = x; } if (opcode == 0x8C) { temp_ = y; }
				Write_To_Ram(rom[pointer] + (rom[pointer + 1] << 8) * (opcode >= 0x8A), temp_, access_size);
				pointer += (opcode < 0x8A ? 1 : 2);
				break;
			/*
				STA $xxxx,x
			*/
			case 0x9D:
				Write_To_Ram((rom[pointer] + (rom[pointer + 1] << 8)) + x, a, access_size);
				pointer += 2;
				break;
			/*
				STA $xxxx,y
			*/
			case 0x99:
				Write_To_Ram((rom[pointer] + rom[pointer + 1] * 256) + y, a, access_size);
				pointer += 2;
				break;
			/*
				LDA $xxxx,x
			*/
			case 0xBD:
				temp_ = Get_Ram((rom[(pointer)] + rom[pointer + 1] * 256) + x, access_size);
				pointer += 2;

				a = temp_;
				break;

			/*
				LDA $xxxx,y
			*/
			case 0xB9:
				temp_ = Get_Ram((rom[(pointer)] + rom[pointer + 1] * 256) + y, access_size);
				pointer += 2;

				a = temp_;
				break;

			/*
				CLC/SEC
			*/
			case 0x18:
			case 0x38:
				break;
			/*
				ADC/SBC (RAM)
			*/
			case 0xED:
			case 0x6D:
				temp_ = Get_Ram(rom[pointer] + rom[pointer + 1] * 256, access_size); pointer += 2;
				if (opcode == 0xED) { a += temp_; }
				else { a -= temp_; }
				break;
			/*
				ADC/SBC (VALUE)
			*/
			case 0x69:
			case 0xE9:
				temp_ = 0;
				for (int i = 0; i < access_size; i++) {
					temp_ += rom[pointer + i] << (i * 8);
				}

				pointer += access_size;

				if (opcode == 0x69) { a += temp_; }
				else { a -= temp_; }
				break;
			/*
				INC A/X/Y
			*/
			case 0x1A:
				a += 1; break;
			case 0xE8:
				x += 1; break;
			case 0xC8:
				y += 1; break;
			/*
				DEC A/X/Y
			*/
			case 0x3A:
				a -= 1; break;
			case 0xCA:
				x -= 1; break;
			case 0x88:
				y -= 1; break;
			/*
				TA X/Y TX A/Y TY A/X
			*/
			case 0xAA: x = a; break;
			case 0xA8: y = a; break;
			case 0x8A: a = x; break;
			case 0x9B: y = x; break;
			case 0x98: a = y; break;
			case 0xBB: x = y; break;
			/*
				CMP/X/Y (Value)
			*/
			case 0xC9:
			case 0xE0:
			case 0xC0:
				temp__ = 0;
				for (int i = 0; i < access_size; i++) {
					temp__ += rom[pointer + i] << (i * 8);
				}
				if (opcode == 0xC9) { temp_ = a; CMP(); }
				if (opcode == 0xE0) { temp_ = x; CMP(); }
				if (opcode == 0xC0) { temp_ = y; CMP(); }

				pointer += access_size;
				break;
			/*
				EOR
			*/
			case 0x49:
				temp__ = 0;
				for (int i = 0; i < access_size; i++) {
					temp__ += rom[pointer + i] << (i * 8);
				}


				a |= temp__;

				pointer += access_size;
				break;
			/*
				AND
			*/
			case 0x29:
				temp__ = 0;
				for (int i = 0; i < access_size; i++) {
					temp__ += rom[pointer + i] << (i * 8);
				}

				a &= temp__;

				pointer += access_size;
				break;
			/*
				BRA/BEQ/BNE/BCS/BCC
			*/
			case 0x80:
			case 0xF0:
			case 0xD0:
			case 0xB0:
			case 0x90:
				pointer += 1;
				if (opcode == 0xF0 && !flags[0]) { break; }
				if (opcode == 0xD0 && !flags[1]) { break; }
				if (opcode == 0xB0 && !flags[2]) { break; }
				if (opcode == 0x90 && !flags[3]) { break; }

				pointer += int_fast8_t(rom[pointer-1]);
				break;
			/*
				REP/SEP
			*/
			case 0xC2:
				st |= rom[pointer];
				pointer += 1;
				break;
			case 0xE2:
				st &= rom[pointer];
				pointer += 1;
				break;
			/*
				ASL
			*/
			case 0x0A:
				a = a << 1;
				break;
			/*
				LSR
			*/
			case 0x4A:
				a = a >> 1;
				break;
			/*
				NOP
			*/
			case 0xEA:
				break;
			/*
				RTL/RTS
			*/
			case 0x60:
			case 0x6B:
				return;
			/*
			unknown opcode
			*/
			default:
				crashed = true;
				cout << red << "[ASM] Crash at pointer 0x" << hex << pointer << " : " << broke_reason << endl; 
				cout << "Information about crash (probably won't help much, you probably did something wrong)" << endl;
				cout << "Current opcode : " << int(opcode) << endl;
				cout << "PC : $" << int(pointer) << " (SNES : $" << pctosnes(pointer) << endl;
				cout << "A, X, Y = " << int(a) << ", " << int(x) << ", " << int(y) << endl;
				cout << "ST = " << int(st) << endl;
				cout << "Flags = ";
				for (int i = 0; i < 8; i++)
				{
					cout << flags[i];
				}
				cout << white << endl;
				return;
			}

		}
	}

	void load_asm(string file, int offset_pointer = 0x000000)
	{
		file = path + file;
		ifstream input(file, ios::binary);
		vector<unsigned char> buffer(istreambuf_iterator<char>(input), {});

		int current_byte = 0;
		for (auto &v : buffer) {
			rom[offset_pointer + current_byte] = uint8_t(v); current_byte += 1;
		}
		input.close();
	}

	void Write_To_Ram(uint_fast32_t pointer, uint_fast32_t value, uint_fast8_t size = 1)
	{
		if (pointer >= RAM_Size)
		{
			return;
		}
		if ((networking && !isClient) && (pointer >= 0x8000 && pointer < 0x10000))
		{
			RAM_decay_time_level[(pointer - 0x8000) & 0x3FFF] = level_ram_decay_time * PlayerAmount;

		}
		for (uint_fast8_t i = 0; i < size; i++) {
			RAM[pointer + i] = uint_fast8_t(value >> (i * 8));
		}
	}

	uint_fast32_t Get_Ram(uint_fast32_t pointer, uint_fast8_t size = 1)
	{
		if (pointer >= RAM_Size)
		{
			return -1;
		}
		uint_fast32_t temp = 0;
		for (uint_fast8_t i = 0; i < size; i++) {
			temp += RAM[pointer + i] << (i * 8);
		}
		return temp;
	}


	//ASM.Write_To_Ram(0x1DF9, 0x1);

	void Reset_ASM_Variables_Server() //This only resets some of the important variables.
	{
		//cout << red << "[ASM] Reset variables." << white << endl;
		Write_To_Ram(0x1DFC, 0, 1);
		Write_To_Ram(0x1DF9, 0, 1);

	}

};


JFKASM ASM;




#if not defined(DISABLE_NETWORK)
void compressHDMAnet()
{
	CurrentPacket << RAM[0x420C];
	CurrentPacket << RAM[0x420B];

	//Compress all data in actually enabled HDMA channels.
	for (uint_fast8_t c = 0; c < 8; c++)
	{
		uint_fast8_t channel = c << 4;
		bool enabled = (RAM[0x420C] >> c) & 1;

		if (enabled) //This HDMA channel is enabled
		{
			//Send mode and reg
			CurrentPacket << RAM[0x4300 + channel];
			CurrentPacket << RAM[0x4301 + channel];

			//Send data size
			CurrentPacket << hdma_size[c];

			uint_fast32_t bank = RAM[0x4302 + channel] + (RAM[0x4303 + channel] << 8) + (RAM[0x4304 + channel] << 16);

			//Send bank and then data
			CurrentPacket << bank;
			for (uint_fast16_t i = 0; i <= hdma_size[c]; i++)
			{
				CurrentPacket << RAM[bank + i];
			}
		}
	}
}

void decompressHDMAnet()
{
	CurrentPacket >> RAM[0x420C];
	CurrentPacket >> RAM[0x420B];

	//Decompress all data in actually enabled HDMA channels.
	for (uint_fast8_t c = 0; c < 8; c++)
	{
		uint_fast8_t channel = c << 4;
		bool enabled = (RAM[0x420C] >> c) & 1;
		if (enabled) //This HDMA channel is enabled
		{
			//Receive mode and reg
			CurrentPacket >> RAM[0x4300 + channel];
			CurrentPacket >> RAM[0x4301 + channel];

			//Receive data size
			uint_fast32_t bank;
			CurrentPacket >> hdma_size[c];
			CurrentPacket >> bank;

			RAM[0x4302 + channel] = bank;
			RAM[0x4303 + channel] = bank >> 8;
			RAM[0x4304 + channel] = bank >> 16;

			for (uint_fast16_t i = 0; i <= hdma_size[c]; i++)
			{
				CurrentPacket >> RAM[bank + i];
			}
		}
	}
}

void Sync_Server_RAM(bool compressed = false)
{
	while (doing_write || doing_read) {
		sf::sleep(sf::milliseconds(1));
	}
	doing_read = true;
	if (!compressed)
	{
		CurrentPacket >> latest_sync;
		ClearSpriteCache();
		for (uint_fast32_t i = 0; i < RAM_Size; i++)
		{
			CurrentPacket >> RAM[i];
		}
	}
	else
	{
		uint_fast16_t entries = 0;
		CurrentPacket >> entries;
		for (uint_fast32_t i = 0; i < entries; i++)
		{
			uint_fast16_t pointer;
			CurrentPacket >> pointer;
			CurrentPacket >> RAM[pointer];
		}

		uint_fast16_t map16_entries_n;
		CurrentPacket >> map16_entries_n;
		for (uint_fast16_t i = 0; i < map16_entries_n; i++)
		{
			uint_fast16_t p;
			CurrentPacket >> p;
			CurrentPacket >> RAM[0x8000 + p];
			CurrentPacket >> RAM[0xC000 + p];
		}


		//HDMA
		decompressHDMAnet();

		//receive Mode 7 stuff
		CurrentPacket >> RAM[0x36];
		CurrentPacket >> RAM[0x38];
		CurrentPacket >> RAM[0x39];

		//Get screen stuff
		CurrentPacket >> RAM[0x1411];
		CurrentPacket >> RAM[0x1412];
		CurrentPacket >> RAM[0x40];

		//receive on/off status
		CurrentPacket >> RAM[0x14AF];

		//receive water status
		CurrentPacket >> RAM[0x85];

		//receive clear status & brightness flag
		CurrentPacket >> RAM[0x1493];

		//receive shake timer
		CurrentPacket >> RAM[0x1887];

		//receive grav
		CurrentPacket >> RAM[0x7C];

		//recieve level start
		CurrentPacket >> RAM[0x3F0B];
		CurrentPacket >> RAM[0x3F0C];
		CurrentPacket >> RAM[0x3F0D];
		CurrentPacket >> RAM[0x3F0E];

		//Could be a loop once again
		CurrentPacket >> RAM[0x1462];
		CurrentPacket >> RAM[0x1463];
		CurrentPacket >> RAM[0x1464];
		CurrentPacket >> RAM[0x1465];
		CurrentPacket >> RAM[0x1466];
		CurrentPacket >> RAM[0x1467];
		CurrentPacket >> RAM[0x1468];
		CurrentPacket >> RAM[0x1469];

		//Decompress OAM
		memset(&RAM[0x200], 0, 0x400);

		uint_fast8_t oam_entries = 0;
		uint_fast16_t pointer = 0;
		CurrentPacket >> oam_entries;
		for (uint_fast8_t i = 0; i < oam_entries; i++)
		{
			CurrentPacket >> RAM[0x0200 + pointer];
			CurrentPacket >> RAM[0x0201 + pointer];
			CurrentPacket >> RAM[0x0202 + pointer];
			CurrentPacket >> RAM[0x0203 + pointer];
			CurrentPacket >> RAM[0x0204 + pointer];
			CurrentPacket >> RAM[0x0205 + pointer];
			CurrentPacket >> RAM[0x0206 + pointer];
			CurrentPacket >> RAM[0x0207 + pointer];
			pointer += 8;
		}


		memset(&RAM[0x2000], 0, 0x80);

		//Decompress sprite entries (Fuck do you mean )
		uint_fast8_t spr_entries;
		CurrentPacket >> spr_entries;
		for (uint_fast8_t i = 0; i < spr_entries; i++) {
			uint_fast8_t p;
			CurrentPacket >> p;
			for (uint_fast16_t n = 0; n < 0x20; n++) {
				CurrentPacket >> RAM[0x2000 + (n << 7) + p];
			}
			//p++;
		}

		//Decompress T3
		memset(&RAM[0x1B800], 0xFF, 0x800);	

		uint_fast16_t T3_entries;
		CurrentPacket >> T3_entries;
		for (uint_fast16_t i = 0; i < T3_entries; i++) {
			uint_fast16_t p; uint_fast8_t t1; uint_fast8_t t2;
			CurrentPacket >> p;
			CurrentPacket >> t1; CurrentPacket >> t2;
			if (p < 0x800)
			{
				RAM[VRAM_Location + 0xB800 + p] = t1;
				RAM[VRAM_Location + 0xB801 + p] = t2;
			}
			else
			{
				break;
			}
		}

	}
	doing_read = false;
	//cout << red << "received ram" << white << endl;
}

bool checkRAMarea_net(uint_fast32_t i)
{
	return 
		((i < 0x200 || i > 0x5FF) && (i < 0x2000 || i >= 0x3000)) && //OAM and SPR
		((i < 0x5000 || i > 0x5FFF) && (i < 0x1B800 || i >= 0x1C000)) && //PLR ram
		(i < 0x7000 || i > 0x7FFF) //Free ram
		;
}

bool checkRamDecay(uint_fast16_t i, bool dec)
{
	if (RAM_decay_time_level[i] > 0)
	{
		if (dec)
		{
			RAM_decay_time_level[i]--;
		}
		return true;
	}
	return false;
}

void Do_RAM_Change()
{
	recent_big_change = true;
	latest_sync++;
}

void Push_Server_RAM(bool compress = false)
{
	while (doing_write) {
		sf::sleep(sf::milliseconds(1));
	}

	doing_read = true;


	if (!compress)
	{
		CurrentPacket << latest_sync;
		for (uint_fast32_t i = 0; i < RAM_Size; i++)
		{
			CurrentPacket << RAM[i];
		}
		memset(&RAM_decay_time_level, 0, 0x4000);
	}
	else
	{
		uint_fast16_t entries = 0;
		for (uint_fast16_t i = 0; i < 0x8000; i++)
		{
			if (checkRAMarea_net(i))
			{
				if (RAM[i] != RAM_old[i])
				{
					entries++; //you stupid //no i not //whats 9 + 10 //twenty one.
				}
			}
		}
		CurrentPacket << entries;

		for (uint_fast16_t i = 0; i < 0x8000; i++)
		{
			if (checkRAMarea_net(i))
			{
				if (RAM[i] != RAM_old[i])
				{
					CurrentPacket << i;
					CurrentPacket << RAM[i];
				}
			}
		}

		uint_fast16_t map16_entries_n = 0;
		for (uint_fast16_t i = 0; i < 0x4000; i++)
		{
			if (checkRamDecay(i, false))
			{
				map16_entries_n++;
			}
		}
		CurrentPacket << map16_entries_n;

		for (uint_fast16_t i = 0; i < 0x4000; i++)
		{
			if (checkRamDecay(i, true))
			{
				CurrentPacket << i;
				CurrentPacket << RAM[0x8000 + i];
				CurrentPacket << RAM[0xC000 + i];
			}
		}

		//HDMA
		compressHDMAnet();

		//Send Mode 7 stuff
		CurrentPacket << RAM[0x36];
		CurrentPacket << RAM[0x38];
		CurrentPacket << RAM[0x39];

		//Send screen stuff
		CurrentPacket << RAM[0x1411];
		CurrentPacket << RAM[0x1412];
		CurrentPacket << RAM[0x40];

		//Send on/off status
		CurrentPacket << RAM[0x14AF];

		//send water status
		CurrentPacket << RAM[0x85];

		//Send level clear status
		CurrentPacket << RAM[0x1493];

		//Send shake timer
		CurrentPacket << RAM[0x1887];

		//Send grav
		CurrentPacket << RAM[0x7C];

		//Send level start
		CurrentPacket << RAM[0x3F0B];
		CurrentPacket << RAM[0x3F0C];
		CurrentPacket << RAM[0x3F0D];
		CurrentPacket << RAM[0x3F0E];

		//Could put these in a loop but I don't care right now
		CurrentPacket << RAM[0x1462];
		CurrentPacket << RAM[0x1463];
		CurrentPacket << RAM[0x1464];
		CurrentPacket << RAM[0x1465];
		CurrentPacket << RAM[0x1466];
		CurrentPacket << RAM[0x1467];
		CurrentPacket << RAM[0x1468];
		CurrentPacket << RAM[0x1469];

		//Compress OAM (send it though)
		uint_fast8_t oam_entries = 0;
		for (uint_fast16_t i = 0; i < 0x400; i += 8) {
			if (RAM[0x200 + i] != 0 || RAM[0x206 + i] != 0) {
				oam_entries += 1;
			}
		}
		CurrentPacket << oam_entries;

		for (uint_fast16_t i = 0; i < 0x400; i += 8) {
			if (RAM[0x200 + i] != 0 || RAM[0x206 + i] != 0) {
				CurrentPacket << RAM[0x0200 + i];
				CurrentPacket << RAM[0x0201 + i];
				CurrentPacket << RAM[0x0202 + i];
				CurrentPacket << RAM[0x0203 + i];
				CurrentPacket << RAM[0x0204 + i];
				CurrentPacket << RAM[0x0205 + i];
				CurrentPacket << RAM[0x0206 + i];
				CurrentPacket << RAM[0x0207 + i];
			}
		}


		//Compress sprite entries
		uint_fast8_t spr_entries = 0;
		for (uint_fast8_t i = 0; i < 0x80; i++) {
			if (RAM[0x2000 + i] != 0) { //Sprites exist
				spr_entries += 1;
			}
		}
		CurrentPacket << spr_entries;
		for (uint_fast8_t i = 0; i < 0x80; i++) {
			if (RAM[0x2000 + i] != 0) {
				CurrentPacket << i;
				for (uint_fast16_t n = 0; n < 0x20; n++) {
					CurrentPacket << RAM[0x2000 + (n << 7) + i];
				}
			}
		}

		
		//Compress T3
		uint_fast16_t T3_entries = 0;
		for (uint_fast16_t T3_loop = 0; T3_loop < 0x800; T3_loop += 2) {
			if (RAM[VRAM_Location + 0xB800 + T3_loop] < MAX_L3_TILES) { //This tile exists
				T3_entries++;
			}
		}
		CurrentPacket << T3_entries;

		for (uint_fast16_t T3_loop = 0; T3_loop < 0x800; T3_loop += 2) {
			if (RAM[VRAM_Location + 0xB800 + T3_loop] < MAX_L3_TILES) { //This tile exists
				CurrentPacket << T3_loop;
				CurrentPacket << RAM[VRAM_Location + 0xB800 + T3_loop];
				CurrentPacket << RAM[VRAM_Location + 0xB801 + T3_loop];
			}
		}
		
	}

	doing_read = false;
}
#endif

void Set_Server_RAM() {
	memcpy(&RAM_old, &RAM, 0x8000 * sizeof(uint_fast8_t));
}