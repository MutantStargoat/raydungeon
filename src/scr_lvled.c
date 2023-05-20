#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/gl.h>
#include "cgmath/cgmath.h"
#include "game.h"
#include "level.h"
#include "util.h"
#include "darray.h"

static int init(void);
static void destroy(void);
static int start(void);
static void stop(void);
static void display(void);
static void reshape(int x, int y);
static void keyb(int key, int press);
static void mouse(int bn, int press, int x, int y);
static void motion(int x, int y);

struct game_screen scr_lvled = {
	"lvled",
	init, destroy,
	start, stop,
	display, reshape,
	keyb, mouse, motion
};

static struct level *lvl;

static float cam_theta, cam_phi = 20, cam_dist = 10;
static float cam_pan[3];

static int init(void)
{
	return 0;
}

static void destroy(void)
{
}

static int start(void)
{
	lvl = malloc_nf(sizeof *lvl);
	init_level(lvl);
	if(load_level(lvl, "data/test.lvl") == -1) {
		return -1;
	}
	cam_pan[0] = -(lvl->xsz / 2.0f) * lvl->scale;
	cam_pan[2] = -(lvl->ysz / 2.0f) * lvl->scale;
	return 0;
}

static void stop(void)
{
	destroy_level(lvl);
	free(lvl);
}

static void display(void)
{
	int i, j;
	float x, y, rad;
	struct level_cell *cell;
	struct level_rect *rect;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, 0, -cam_dist);
	glRotatef(cam_phi, 1, 0, 0);
	glRotatef(cam_theta, 0, 1, 0);
	glTranslatef(cam_pan[0], cam_pan[1], cam_pan[2]);

	cell = lvl->cells;
	glBegin(GL_QUADS);
	glColor3f(0.4, 0.4, 0.4);
	glNormal3f(0, 1, 0);
	rad = lvl->scale * 0.48;
	for(i=0; i<lvl->ysz; i++) {
		y = (float)i * lvl->scale;
		for(j=0; j<lvl->xsz; j++) {
			x = (float)j * lvl->scale;
			if(cell->type) {
				glVertex3f(x - rad, -1, y - rad);
				glVertex3f(x + rad, -1, y - rad);
				glVertex3f(x + rad, -1, y + rad);
				glVertex3f(x - rad, -1, y + rad);
			}
			cell++;
		}
	}
	glEnd();

	glLineWidth(2);
	rect = lvl->rects;
	for(i=0; i<darr_size(lvl->rects); i++) {
		float x = (rect->x - 0.55) * lvl->scale;
		float y = (rect->y - 0.55) * lvl->scale;
		float w = (rect->w + 0.1) * lvl->scale;
		float h = (rect->h + 0.1) * lvl->scale;

		glBegin(GL_LINE_LOOP);
		glColor3ubv(rect->dbgcol);
		glVertex3f(x, 0, y);
		glVertex3f(x + w, 0, y);
		glVertex3f(x + w, 0, y + h);
		glVertex3f(x, 0, y + h);
		glEnd();
		rect++;
	}
	glLineWidth(1);
}

static void reshape(int x, int y)
{
	float proj_mat[16];

	cgm_mperspective(proj_mat, cgm_deg_to_rad(60), win_aspect, 0.5, 500.0);
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(proj_mat);
}

static void keyb(int key, int press)
{
	if(!press) return;

	switch(key) {
	case 'g':
		lvl_gen_rects(lvl);
		break;

	case 's':
		printf("saving level\n");
		save_level(lvl, "data/test.lvl");
		break;

	default:
		break;
	}
}

static void mouse(int bn, int press, int x, int y)
{
}

static void motion(int x, int y)
{
	int dx = x - mouse_x;
	int dy = y - mouse_y;

	if(!(dx | dy)) return;

	if(mouse_state[0]) {
		cam_theta += dx * 0.5;
		cam_phi += dy * 0.5;
		if(cam_phi < -90) cam_phi = -90;
		if(cam_phi > 90) cam_phi = 90;
	}
	if(mouse_state[1]) {
		float up[3], right[3];
		float theta = cam_theta * M_PI / 180.0f;
		float phi = cam_phi * M_PI / 180.0f;

		up[0] = -sin(theta) * sin(phi);
		up[1] = -cos(phi);
		up[2] = cos(theta) * sin(phi);
		right[0] = cos(theta);
		right[1] = 0;
		right[2] = sin(theta);

		cam_pan[0] += (right[0] * dx + up[0] * dy) * 0.01;
		cam_pan[1] += up[1] * dy * 0.01;
		cam_pan[2] += (right[2] * dx + up[2] * dy) * 0.01;
	}
	if(mouse_state[2]) {
		cam_dist += dy * 0.1;
		if(cam_dist < 0) cam_dist = 0;
	}
}
