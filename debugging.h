#pragma once

void dump_ram()
{
	ofstream fp;
	fp.open(path + "ramdump.bin", ios::out | ios::binary);
	fp.write((char*)RAM, sizeof(RAM));
	fp.close();

	cout << red << "[ASM] RAM has been dumped to ramdump.bin" << white << endl;
}


bool pressed_hide = false;
bool pressed_diag = false;
bool pressed_drawl1 = false;
bool pressed_bg = false;
bool pressed_drawsprites = false;
bool pressed_dumpram = false;
bool pressed_dumplevel = false;

void dump_level_data()
{
	ofstream out(path + "level_data_dump.txt");
	out << "[level_data]" << endl;
	for (uint_fast16_t x = 0; x < mapWidth; x++)
	{
		for (uint_fast16_t y = 0; y < mapHeight; y++)
		{
			if (map16_handler.get_tile(x, y) != 0x25)
			{
				out << int_to_hex(map16_handler.get_tile(x, y)) << "," << x << "," << y << endl;
			}
		}
	}
	out.close();

	cout << green << "[Level Manager] Level dumped to level_data_dump.txt" << white << endl;
}

void debugging_functions()
{
	/*
	Debug input checks
	*/
	bool stat = false;
	if (!Chatting)
	{
		stat = state[input_settings[11]];
		if (stat != pressed_hide) {
			pressed_hide = stat;
			if (stat) {
				drawHud = !drawHud;
			}
		}

		stat = state[input_settings[12]];
		if (stat != pressed_diag) {
			pressed_diag = stat;
			if (stat) {
				drawDiag = !drawDiag;
			}
		}

		stat = state[input_settings[13]];
		if (stat != pressed_drawl1) {
			pressed_drawl1 = stat;
			if (stat) {
				drawL1 = !drawL1;
			}
		}

		stat = state[input_settings[14]];
		if (stat != pressed_bg) {
			pressed_bg = stat;
			if (stat) {
				drawBg = !drawBg;
			}
		}

		stat = state[input_settings[15]];
		if (stat != pressed_drawsprites) {
			pressed_drawsprites = stat;
			if (stat) {
				drawSprites = !drawSprites;
			}
		}

		stat = state[input_settings[16]];
		if (stat != pressed_dumpram) {
			pressed_dumpram = stat;
			if (stat) {
				dump_ram();
			}
		}

		stat = state[input_settings[17]];
		if (stat != pressed_dumplevel) {
			pressed_dumplevel = stat;
			if (stat) {
				dump_level_data();
			}
		}
	}
}