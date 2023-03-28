#include "game.h"

static int map_init(void);
static void map_destroy(void);
static int map_start(void);
static void map_stop(void);
static void map_display(void);
static void map_reshape(int x, int y);
static void map_keyb(int key, int press);
static void map_mouse(int bn, int press, int x, int y);
static void map_motion(int x, int y);

struct game_screen scr_map = {
	"map",
	map_init, map_destroy,
	map_start, map_stop,
	map_display, map_reshape,
	map_keyb, map_mouse, map_motion
};

static int map_init(void)
{
	return 0;
}

static void map_destroy(void)
{
}

static int map_start(void)
{
	return 0;
}

static void map_stop(void)
{
}

static void map_display(void)
{
}

static void map_reshape(int x, int y)
{
}

static void map_keyb(int key, int press)
{
}

static void map_mouse(int bn, int press, int x, int y)
{
}

static void map_motion(int x, int y)
{
}
