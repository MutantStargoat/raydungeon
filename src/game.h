#ifndef GAME_H_
#define GAME_H_

enum {
	GKEY_ESC	= 27,
	GKEY_DEL	= 127,
	GKEY_F1		= 256,
	GKEY_F2, GKEY_F3, GKEY_F4, GKEY_F5, GKEY_F6, GKEY_F7,
	GKEY_F8, GKEY_F9, GKEY_F10, GKEY_F11, GKEY_F12,
	GKEY_UP, GKEY_DOWN, GKEY_LEFT, GKEY_RIGHT,
	GKEY_PGUP, GKEY_PGDOWN,
	GKEY_HOME, GKEY_END,
	GKEY_INS
};

extern int game_mx, game_my, game_mstate[3];
extern int game_win_width, game_win_height;
extern float game_win_aspect;

int game_init(void);
void game_shutdown(void);

void game_display(void);
void game_reshape(int x, int y);
void game_keyboard(int key, int press);
void game_mouse(int bn, int st, int x, int y);
void game_motion(int x, int y);

/* defined in main.c */
void game_swap_buffers(void);
void game_quit(void);

#endif	/* GAME_H_ */
