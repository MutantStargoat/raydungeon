#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "opengl.h"
#include "cgmath/cgmath.h"
#include "game.h"
#include "sdr.h"
#include "level.h"
#include "util.h"

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

static struct level *lvl;

static float proj_mat[16], view_mat[16];

static float cam_theta, cam_phi = 20, cam_dist = 10;
static cgm_vec3 cam_pan;

static unsigned int sdr;

static int ginit(void)
{
	return 0;
}

static void gdestroy(void)
{
}

static int gstart(void)
{
	lvl = malloc_nf(sizeof *lvl);
	init_level(lvl);
	if(load_level(lvl, "data/test.lvl") == -1) {
		return -1;
	}
	cam_pan.x = -(lvl->xsz / 2.0f) * lvl->scale;
	cam_pan.y = 0;
	cam_pan.z = -(lvl->ysz / 2.0f) * lvl->scale;

	glEnable(GL_DEPTH_TEST);

	if(lvl->sdf_src) {
		add_shader_footer(GL_FRAGMENT_SHADER, lvl->sdf_src);
	} else {
		add_shader_footer(GL_FRAGMENT_SHADER, "float eval_sdf(in vec3 p) { return 10000.0; }\n");
	}
	if(!(sdr = create_program_load("sdr/raydungeon.v.glsl", "sdr/raydungeon.p.glsl"))) {
		return -1;
	}
	clear_shader_footer(GL_FRAGMENT_SHADER);
	glUseProgram(sdr);
	return 0;
}

static void gstop(void)
{
	destroy_level(lvl);
	free(lvl);

	free_program(sdr);
}

static void gdisplay(void)
{
	int i, j;
	float x, y;
	struct level_cell *cell;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	cgm_mtranslation(view_mat, 0, 0, -cam_dist);
	cgm_mprerotate_x(view_mat, cam_phi);
	cgm_mprerotate_y(view_mat, cam_theta);
	cgm_mpretranslate(view_mat, cam_pan.x, cam_pan.y, cam_pan.z);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(view_mat);

	glUseProgram(sdr);

	glBegin(GL_TRIANGLES);
	glTexCoord2f(0, 0); glVertex2f(-1, -1);
	glTexCoord2f(2, 0); glVertex2f(3, -1);
	glTexCoord2f(0, 2); glVertex2f(-1, 3);
	glEnd();

	glUseProgram(0);


	cell = lvl->cells;
	glBegin(GL_QUADS);
	glColor3f(1, 1, 1);
	glNormal3f(0, 1, 0);
	for(i=0; i<lvl->ysz; i++) {
		y = (float)i * lvl->scale;
		for(j=0; j<lvl->xsz; j++) {
			x = (float)j * lvl->scale;
			if(cell->type) {
				glVertex3f(x - 0.48, -1, y - 0.48);
				glVertex3f(x + 0.48, -1, y - 0.48);
				glVertex3f(x + 0.48, -1, y + 0.48);
				glVertex3f(x - 0.48, -1, y + 0.48);
			}
			cell++;
		}
	}
	glEnd();
}

static void greshape(int x, int y)
{
	cgm_mperspective(proj_mat, cgm_deg_to_rad(60), win_aspect, 0.5, 40.0);
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(proj_mat);
}

static void gkeyb(int key, int press)
{
	if(press) {
		switch(key) {
		case '`':
			if(!fullscr) {
				game_grabmouse(-1);	/* toggle */
			}
			break;

		default:
			break;
		}
	}
}

static void gmouse(int bn, int press, int x, int y)
{
}

static void gmotion(int x, int y)
{
	int dx = x - mouse_x;
	int dy = y - mouse_y;

	if(!(dx | dy)) return;

	if(mouse_state[0]) {
		cam_theta += dx * 0.02;
		cam_phi += dy * 0.02;
		if(cam_phi < -M_PI/2) cam_phi = -M_PI/2;
		if(cam_phi > M_PI/2) cam_phi = M_PI/2;
	}
	if(mouse_state[1]) {
		float up[3], right[3];

		up[0] = -sin(cam_theta) * sin(cam_phi);
		up[1] = -cos(cam_phi);
		up[2] = cos(cam_theta) * sin(cam_phi);
		right[0] = cos(cam_theta);
		right[1] = 0;
		right[2] = sin(cam_theta);

		cam_pan.x += (right[0] * dx + up[0] * dy) * 0.01;
		cam_pan.y += up[1] * dy * 0.01;
		cam_pan.z += (right[2] * dx + up[2] * dy) * 0.01;
	}
	if(mouse_state[2]) {
		cam_dist += dy * 0.01;
		if(cam_dist < 0) cam_dist = 0;
	}
}
