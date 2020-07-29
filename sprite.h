#pragma once


SDL_Surface* loadSurface(string path)
{
	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	return loadedSurface;
}

class TextureManager
{
public:
	unordered_map<string, SDL_Texture*> Textures;
	SDL_Texture *loadTexture(string file)
	{
		auto entry = Textures.find(file);

		if (entry != Textures.end())
		{
			return entry->second;
		}
		SDL_Surface *s = loadSurface(file);
		SDL_Texture *t = SDL_CreateTextureFromSurface(ren, s);
		SDL_FreeSurface(s);
		Textures.insert(make_pair(file, t));

		return t;

	}

	void ClearManager()
	{
		if (Textures.size() > 0)
		{
			cout << purple_int << "[TexManager] Clearing TexManager" << white << endl;
			for (unordered_map<string, SDL_Texture*>::iterator it = Textures.begin(); it != Textures.end(); ++it)
			{
				SDL_DestroyTexture(it->second);
			}
			Textures.clear();
		}
	}
};
TextureManager TexManager;

SDL_Texture *bg_texture;

class Sprite
{
public:
	Sprite(string sprite, int x, int y, int size_x, int size_y, double angle = 0.0)
	{
		SDL_Rect DestR;

		DestR.x = sp_offset_x + x * scale;
		DestR.y = sp_offset_y + y * scale;
		DestR.w = abs(size_x) * scale;
		DestR.h = abs(size_y) * scale;
		SDL_RendererFlip flip = SDL_FLIP_NONE;
		if (size_x < 0) { flip = SDL_FLIP_HORIZONTAL; }



		SDL_RenderCopyEx(ren, TexManager.loadTexture(sprite), NULL, &DestR, angle, NULL, flip);

	}

};


void RenderBackground(int x, int y)
{
	double bg_scale_x = 32.0 / double(RAM[0x38]);
	double bg_scale_y = 32.0 / double(RAM[0x39]);

	SDL_Rect SourceR;
	SourceR.x = sp_offset_x + x * scale;
	SourceR.y = sp_offset_y + y * scale;
	SourceR.w = int(512.0 * bg_scale_x) * scale;
	SourceR.h = int(512.0 * bg_scale_y) * scale;

	SDL_RenderCopyEx(ren, bg_texture, NULL, &SourceR, (double(ASM.Get_Ram(0x36,1))*360.0)/256.0, NULL, SDL_FLIP_NONE);
}
