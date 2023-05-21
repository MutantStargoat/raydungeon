#include <string.h>
#include "input.h"

static const struct input_map def_inpmap[MAX_INPUTS] = {
	{INP_FWD, 'w', -1},
	{INP_BACK, 's', -1},
	{INP_TLEFT, 'q', -1},
	{INP_TRIGHT, 'e', -1},
	{INP_SLEFT, 'a', -1},
	{INP_SRIGHT, 'd', -1}
};

struct input_map inpmap[MAX_INPUTS];

unsigned int inpstate;


void init_input(void)
{
	memcpy(inpmap, def_inpmap, sizeof inpmap);
	inpstate = 0;
}
