#include "game.h"

static int ginit(void);
static void gdestroy(void);
static int gstart(void);
static void gstop(void);
static void gdisplay(void);
static void greshape(int x, int y);
static void gkeyb(int key, int press);
static void gmouse(int bn, int press, int x, int y);
static void gmotion(int x, int y);

struct game_screen scr_game = {
	"game",
	ginit, gdestroy,
	gstart, gstop,
	gdisplay, greshape,
	gkeyb, gmouse, gmotion
};


static int ginit(void)
{
	return 0;
}

static void gdestroy(void)
{
}

static int gstart(void)
{
	return 0;
}

static void gstop(void)
{
}

static void gdisplay(void)
{
}

static void greshape(int x, int y)
{
}

static void gkeyb(int key, int press)
{
}

static void gmouse(int bn, int press, int x, int y)
{
}

static void gmotion(int x, int y)
{
}
