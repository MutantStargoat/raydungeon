#include <GL/gl.h>
#include "game.h"

int game_mx, game_my, game_mstate[3];
int game_win_width, game_win_height;
float game_win_aspect;

int game_init(void)
{
	glClearColor(0.3, 0.3, 0.3, 1);
	return 0;
}

void game_shutdown(void)
{
}

void game_display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	game_swap_buffers();
}

void game_reshape(int x, int y)
{
	game_win_width = x;
	game_win_height = y;
	game_win_aspect = (float) x / y;
	glViewport(0, 0, x, y);
}

void game_keyboard(int key, int press)
{
	if(!press) return;

	switch(key) {
	case 27:
		game_quit();
		break;
	}
}

void game_mouse(int bn, int st, int x, int y)
{
	game_mx = x;
	game_my = y;
	if(bn < 3) {
		game_mstate[bn] = st;
	}
}

void game_motion(int x, int y)
{
	game_mx = x;
	game_my = y;
}
