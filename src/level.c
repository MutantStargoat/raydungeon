#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "level.h"
#include "darray.h"
#include "assfile.h"
#include "util.h"

static void parse_cell(struct level *lvl, int x, int y, char c);

void init_level(struct level *lvl)
{
	memset(lvl, 0, sizeof *lvl);

	lvl->scale = 1.0f;
	lvl->rects = darr_alloc(0, sizeof *lvl->rects);
}

void destroy_level(struct level *lvl)
{
	darr_free(lvl->rects);
}

static int rect_area(struct level_rect *rect)
{
	return rect->w * rect->h;
}

static int grow_rect(struct level *lvl, struct level_cell *orgcell, struct level_rect *rect)
{
	int i, j, x, y, area_a, area_b;
	struct level_rect ra, rb;
	struct level_cell *cell;

	ra = rb = *rect;

	/* try expanding horizontally first */
	cell = orgcell + 1;
	x = ra.x + 1;
	while(x++ < lvl->xsz && cell->type == CELL_SOLID) {
		ra.w++;
		cell++;
	}
	/* then for each line below, expand as long as we have at least ra.w space */
	cell = orgcell + lvl->xsz;
	y = ra.y + 1;
	while(y++ < lvl->ysz) {
		int rowsz = 0;
		x = ra.x;
		while(x++ < lvl->xsz && cell->type == CELL_SOLID) {
			rowsz++;
			cell++;
		}
		if(rowsz < ra.w) break;
		ra.h++;
		cell += lvl->xsz - rowsz;
	}

	/* then try the same thing, but vertical first */
	cell = orgcell + lvl->xsz;
	y = rb.y + 1;
	while(y++ < lvl->ysz && cell->type == CELL_SOLID) {
		rb.h++;
		cell += lvl->xsz;
	}
	/* then for each next column, expand as long as we have at least rb.h space */
	x = rb.x + 1;
	while(x < lvl->xsz) {
		int colsz = 0;
		y = rb.y;
		cell = lvl->cells + y * lvl->xsz + x;
		while(y++ < lvl->ysz && cell->type == CELL_SOLID) {
			colsz++;
			cell += lvl->xsz;
		}
		if(colsz < rb.h) break;
		rb.w++;
		x++;
	}

	/* return the rect with the largest area */
	area_a = rect_area(&ra);
	area_b = rect_area(&rb);

	*rect = area_a > area_b ? ra : rb;

	cell = orgcell;
	for(i=0; i<rect->h; i++) {
		for(j=0; j<rect->w; j++) {
			cell[j].visited = 1;
		}
		cell += lvl->xsz;
	}
	return 0;
}

void lvl_gen_rects(struct level *lvl)
{
	int i, j;
	struct level_cell *cell;
	struct level_rect rect;

	darr_clear(lvl->rects);
	cell = lvl->cells;
	for(i=0; i<lvl->ysz; i++) {
		for(j=0; j<lvl->xsz; j++) {
			cell[j].visited = 0;
		}
		cell += lvl->xsz;
	}

	cell = lvl->cells;
	for(i=0; i<lvl->ysz; i++) {
		for(j=0; j<lvl->xsz; j++) {
			if(cell->type == CELL_SOLID && !cell->visited) {
				rect.x = j;
				rect.y = i;
				rect.w = rect.h = 1;
				if(grow_rect(lvl, cell, &rect) != -1) {
					rect.dbgcol[0] = (rand() & 0x7f) + 0x7f;
					rect.dbgcol[1] = (rand() & 0x7f) + 0x7f;
					rect.dbgcol[2] = (rand() & 0x7f) + 0x7f;

					darr_push(lvl->rects, &rect);
				}
			}
			cell++;
		}
	}
}

int save_level(const struct level *lvl, const char *fname)
{
	FILE *fp;
	int i, j, num;
	struct level_cell *cell;
	struct level_rect *rect;

	if(!(fp = fopen(fname, "wb"))) {
		fprintf(stderr, "save_level: failed to open: %s: %s\n", fname, strerror(errno));
		return -1;
	}

	fprintf(fp, "RDLEVEL\n");
	fprintf(fp, "size = %dx%d\n", lvl->xsz, lvl->ysz);
	fprintf(fp, "scale = %f\n", lvl->scale);

	fputs("\nMAPSTART\n", fp);
	cell = lvl->cells;
	for(i=0; i<lvl->ysz; i++) {
		for(j=0; j<lvl->xsz; j++) {
			switch(cell->type & 0xff) {
			case CELL_OPEN:
				if(lvl->sx == j && lvl->sy == i) {
					fputc('S', fp);
				} else if((cell->type & CELL_WALK) == 0) {
					fputc('.', fp);
				} else {
					fputc(' ', fp);
				}
				break;

			case CELL_SOLID:
			default:
				fputc('#', fp);
			}
			cell++;
		}
		fputc('\n', fp);
	}
	fputs("MAPEND\n", fp);

	if(!darr_empty(lvl->rects)) {
		num = darr_size(lvl->rects);
		fprintf(fp, "\nRECTSTART %d\n", num);
		rect = lvl->rects;
		for(i=0; i<num; i++) {
			fprintf(fp, "rect %d %d %d %d\n", rect->x, rect->y, rect->w, rect->h);
			rect++;
		}
		fprintf(fp, "RECTEND\n\n");


		fprintf(fp, "SDRSTART\n");
		fprintf(fp, "float eval_sdf_gen(in vec3 p)\n");
		fprintf(fp, "{\n");
		rect = lvl->rects;
		for(i=0; i<num; i++) {
			float cx = (rect->x - 0.5f + rect->w * 0.5f) * lvl->scale;
			float cy = (rect->y - 0.5f + rect->h * 0.5f) * lvl->scale;
			float rx = (rect->w + 0.1f) * lvl->scale * 0.5f;
			float ry = (rect->h + 0.1f) * lvl->scale * 0.5f;

			if(i == 0) {
				fprintf(fp, "\tfloat d = boxdist(p - vec3(%f, 0.0, %f), vec3(%f, 1.0, %f));\n",
						cx, cy, rx, ry);
			} else {
				fprintf(fp, "\td = min(d, boxdist(p - vec3(%f, 0.0, %f), vec3(%f, 1.0, %f)));\n",
						cx, cy, rx, ry);
			}
			rect++;
		}
		fprintf(fp, "\treturn d;\n}\n");
		fprintf(fp, "SDREND\n\n");
	}

	fclose(fp);
	return 0;
}

static char *clean_line(char *s)
{
	char *end;

	while(*s && isspace(*s)) s++;

	if(!(end = strchr(s, '#'))) {
		end = s + strlen(s) - 1;
	}
	while(end >= s && isspace(*end)) *end-- = 0;

	return *s ? s : 0;
}

enum {
	ST_HEADER,
	ST_ASSIGN,
	ST_MAP,
	ST_SDR
};

int load_level(struct level *lvl, const char *fname)
{
	ass_file *fp;
	char buf[512];
	char *line, *val, *endp;
	float fnum;
	int state = 0;
	int i, nrow;
	struct level_rect rect;
	char *sdr = 0;

	if(!(fp = ass_fopen(fname, "rb"))) {
		fprintf(stderr, "load_level: failed to open: %s\n", fname);
		return -1;
	}

	while(ass_fgets(buf, sizeof buf, fp)) {
		switch(state) {
		case ST_HEADER:
			if(memcmp(buf, "RDLEVEL", 7) != 0) {
				fprintf(stderr, "load_level: invalid level file: %s\n", fname);
				ass_fclose(fp);
				return -1;
			}
			state = ST_ASSIGN;
			break;

		case ST_ASSIGN:
			if(!(line = clean_line(buf)) || *line == '#') {
				continue;
			}

			if((val = strchr(line, '='))) {
				*val++ = 0;
				clean_line(line);
				if(!(val = clean_line(val))) {
					fprintf(stderr, "load_level: ignoring invalid value for %s: %s\n", line, val);
					continue;
				}
				if(strcmp(line, "size") == 0) {
					int x, y;
					if(sscanf(val, "%dx%d", &x, &y) != 2) {
						fprintf(stderr, "load_level: ignoring invalid size: %s\n", val);
						continue;
					}
					lvl->xsz = x;
					lvl->ysz = y;
				} else if(strcmp(line, "scale") == 0) {
					fnum = strtod(val, &endp);
					if(endp == val) {
						fprintf(stderr, "load_level: ignoring invalid scale: %s\n", val);
						continue;
					}
					lvl->scale = fnum;
				} else {
					fprintf(stderr, "load_level: ignoring unknown option: %s\n", line);
					continue;
				}

			} else if(strcmp(line, "MAPSTART") == 0) {
				if(!lvl->xsz || !lvl->ysz) {
					fprintf(stderr, "load_level: missing size before level data\n");
					ass_fclose(fp);
					return -1;
				}

				lvl->cells = calloc_nf(lvl->xsz * lvl->ysz, sizeof *lvl->cells);
				nrow = 0;
				state = ST_MAP;

			} else if(strcmp(line, "SDRSTART") == 0) {
				if(sdr) {
					darr_clear(sdr);
				} else {
					sdr = darr_alloc(0, 1);
				}
				state = ST_SDR;

			} else if(sscanf(line, "rect %d %d %d %d", &rect.x, &rect.y, &rect.w, &rect.h) == 4) {
				rect.dbgcol[0] = (rand() & 0x7f) + 0x7f;
				rect.dbgcol[1] = (rand() & 0x7f) + 0x7f;
				rect.dbgcol[2] = (rand() & 0x7f) + 0x7f;
				darr_push(lvl->rects, &rect);

			}
			break;

		case ST_MAP:
			if(memcmp(buf, "MAPEND", 6) == 0) {
				state = ST_ASSIGN;
				break;
			}
			for(i=0; i<lvl->xsz; i++) {
				if(!buf[i]) break;
				parse_cell(lvl, i, nrow, buf[i]);
			}
			if(++nrow >= lvl->ysz) {
				state = ST_ASSIGN;
				break;
			}
			break;

		case ST_SDR:
			if(memcmp(buf, "SDREND", 6) == 0) {
				state = ST_ASSIGN;
				break;
			}
			for(i=0; buf[i]; i++) {
				darr_strpush(sdr, buf[i]);
			}
			break;
		}
	}

	if(sdr) {
		lvl->sdf_src = darr_finalize(sdr);
	}

	ass_fclose(fp);
	return 0;
}

static void parse_cell(struct level *lvl, int x, int y, char c)
{
	struct level_cell *cell = lvl->cells + y * lvl->xsz + x;

	switch(c) {
	case ' ':
		cell->type = CELL_OPEN | CELL_WALK;
		break;

	case 'S':
		cell->type = CELL_OPEN | CELL_WALK;
		lvl->sx = x;
		lvl->sy = y;
		break;

	case 't':
	case '.':
		cell->type = CELL_OPEN;
		break;

	default:
		break;
	}
}
