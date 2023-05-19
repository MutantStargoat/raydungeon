#ifndef LEVEL_H_
#define LEVEL_H_

enum { DIR_N, DIR_E, DIR_S, DIR_W };

enum { CELL_SOLID, CELL_OPEN };
#define CELL_WALK	0x0100

struct level_cell {
	int type;
	unsigned int wallflags[4];
	int visited;
};

struct level_rect {
	int x, y, w, h;
	unsigned char dbgcol[4];
};

struct level {
	int xsz, ysz;
	float scale;
	struct level_cell *cells;
	int sx, sy;

	struct level_rect *rects;	/* darr, empty spaces */
};

void init_level(struct level *lvl);
void destroy_level(struct level *lvl);

void lvl_gen_rects(struct level *lvl);

int save_level(const struct level *lvl, const char *fname);
int load_level(struct level *lvl, const char *fname);

#endif	/* LEVEL_H_ */
