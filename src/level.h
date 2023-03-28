#ifndef LEVEL_H_
#define LEVEL_H_

enum { DIR_N, DIR_E, DIR_S, DIR_W };

enum { CELL_SOLID, CELL_OPEN };
#define CELL_WALK	0x0100

struct level_cell {
	int type;
	unsigned int wallflags[4];
};

struct level_span {
	int start, len;
};

struct level {
	int xsz, ysz;
	struct level_cell *cells;

	struct level_span *hspans, *vspans;	/* dynamic array */
};

void init_level(struct level *lvl);
void destroy_level(struct level *lvl);

int load_level(struct level *lvl, const char *fname);

#endif	/* LEVEL_H_ */
