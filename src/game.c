#include <stdlib.h>
#include <string.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "game.h"
#include "options.h"

int mouse_x, mouse_y, mouse_state[3];
int mouse_grabbed;
unsigned int modkeys;
int win_width, win_height;
float win_aspect;
int fullscr;

long time_msec;

struct game_screen *cur_scr;
char *start_scr_name;

/* available screens */
extern struct game_screen scr_menu, scr_game, scr_map, scr_lvled;
#define MAX_SCREENS	4
static struct game_screen *screens[MAX_SCREENS];
static int num_screens;


int game_init(int argc, char **argv)
{
	int i;
	char *env;

	load_options(GAME_CFG_FILE);
	if(parse_options(argc, argv) == -1) {
		return -1;
	}
	game_resize(opt.xres, opt.yres);
	game_vsync(opt.vsync);
	if(opt.fullscreen) {
		game_fullscreen(1);
	}

	/* initialize screens */
	screens[num_screens++] = &scr_menu;
	screens[num_screens++] = &scr_game;
	screens[num_screens++] = &scr_map;
	screens[num_screens++] = &scr_lvled;

	if((env = getenv("START_SCREEN"))) {
		start_scr_name = env;
	}

	for(i=0; i<num_screens; i++) {
		if(screens[i]->init() == -1) {
			return -1;
		}
	}

	glClearColor(0.1, 0.1, 0.1, 1);

	for(i=0; i<num_screens; i++) {
		if(screens[i]->name && start_scr_name && strcmp(screens[i]->name, start_scr_name) == 0) {
			game_chscr(screens[i]);
			break;
		}
	}
	if(!cur_scr) {
		game_chscr(&scr_game);	/* TODO: scr_menu */
	}
	return 0;
}

void game_shutdown(void)
{
	int i;

	save_options(GAME_CFG_FILE);

	for(i=0; i<num_screens; i++) {
		if(screens[i]->destroy) {
			screens[i]->destroy();
		}
	}
}

void game_display(void)
{
	time_msec = game_getmsec();

	if(cur_scr) {
		cur_scr->display();
	}

	game_swap_buffers();
}

void game_reshape(int x, int y)
{
	win_width = x;
	win_height = y;
	win_aspect = (float)x / (float)y;
	glViewport(0, 0, x, y);

	if(cur_scr && cur_scr->reshape) {
		cur_scr->reshape(x, y);
	}
}

void game_keyboard(int key, int press)
{
	if(press) {
		switch(key) {
		case 27:
			game_quit();
			break;

		case '\n':
		case '\r':
			if(modkeys & GKEY_MOD_ALT) {
		case GKEY_F11:
				game_fullscreen(-1);
				return;
			}
			break;
		}
	}

	if(cur_scr && cur_scr->keyboard) {
		cur_scr->keyboard(key, press);
	}
}

void game_mouse(int bn, int st, int x, int y)
{
	mouse_x = x;
	mouse_y = y;
	if(bn < 3) {
		mouse_state[bn] = st;
	}

	if(cur_scr && cur_scr->mouse) {
		cur_scr->mouse(bn, st, x, y);
	}
}

void game_motion(int x, int y)
{
	if(cur_scr && cur_scr->motion) {
		cur_scr->motion(x, y);
	}
	mouse_x = x;
	mouse_y = y;
}

void game_chscr(struct game_screen *scr)
{
	struct game_screen *prev = cur_scr;

	if(!scr) return;

	if(scr->start && scr->start() == -1) {
		return;
	}
	if(scr->reshape) {
		scr->reshape(win_width, win_height);
	}

	if(prev && prev->stop) {
		prev->stop();
	}
	cur_scr = scr;
}
