#ifndef OPTIONS_H_
#define OPTIONS_H_

struct gfxoptions {
	float render_res;	/* 0.5 means render at half-res and scale by 2 */
};

struct options {
	int xres, yres;
	int vsync;
	int fullscreen;
	int vol_master, vol_mus, vol_sfx;
	int music;

	int inv_mouse_y;
	int mouse_speed;

	struct gfxoptions gfx;
};

extern struct options opt;

int parse_options(int argc, char **argv);

int load_options(const char *fname);
int save_options(const char *fname);

#endif	/* OPTIONS_H_ */
