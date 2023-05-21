#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "opengl.h"
#include "cgmath/cgmath.h"
#include "game.h"
#include "sdr.h"
#include "level.h"
#include "util.h"
#include "options.h"
#include "player.h"
#include "input.h"

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

static struct player player;
static float cam_dist;

static unsigned int sdr;

static int rbuf_width, rbuf_height;
static unsigned int fbo, rbuf_col, rbuf_zbuf;


static int ginit(void)
{
	return 0;
}

static void gdestroy(void)
{
	glDeleteRenderbuffers(1, &rbuf_col);
	glDeleteRenderbuffers(1, &rbuf_zbuf);
	glDeleteFramebuffers(1, &fbo);
}

static int gstart(void)
{
	init_player(&player);
	init_input();

	lvl = malloc_nf(sizeof *lvl);
	init_level(lvl);
	if(load_level(lvl, "data/test.lvl") == -1) {
		return -1;
	}
	player.pos.x = -lvl->sx * lvl->scale;
	player.pos.y = 0;
	player.pos.z = -lvl->sy * lvl->scale;

	glEnable(GL_DEPTH_TEST);

	if(lvl->sdf_src) {
		add_shader_footer(GL_FRAGMENT_SHADER, lvl->sdf_src);
	} else {
		add_shader_header(GL_FRAGMENT_SHADER, "#define TEST_SDF\n");
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

#define KB_MOVE_SPEED	0.5f

static void update(void)
{
	if(inpstate & INP_MOVE_BITS) {
		if(inpstate & INP_FWD_BIT) {
			player.vel.z -= KB_MOVE_SPEED;
		}
		if(inpstate & INP_BACK_BIT) {
			player.vel.z += KB_MOVE_SPEED;
		}
		if(inpstate & INP_SRIGHT_BIT) {
			player.vel.x += KB_MOVE_SPEED;
		}
		if(inpstate & INP_SLEFT_BIT) {
			player.vel.x -= KB_MOVE_SPEED;
		}
	}

	update_player(&player);
	calc_player_matrix(&player);
}

static void gdisplay(void)
{
	int i, j;
	float x, y;
	struct level_cell *cell;
	static long last_upd;
	static float upd_acc;

	upd_acc += (time_msec - last_upd) / 1000.0f;
	for(i=0; i<MAX_UPD; i++) {
		if(upd_acc < UPD_TSTEP) break;
		upd_acc -= UPD_TSTEP;
		update();
	}
	if(i > 0) {
		last_upd = time_msec;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glViewport(0, 0, rbuf_width, rbuf_height);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	cgm_mtranslation(view_mat, 0, 0, -cam_dist);
	cgm_mprerotate_x(view_mat, player.phi);
	cgm_mprerotate_y(view_mat, player.theta);
	cgm_mpretranslate(view_mat, player.pos.x, player.pos.y, player.pos.z);
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
				glVertex3f(x - 0.48 * lvl->scale, -1, y - 0.48 * lvl->scale);
				glVertex3f(x + 0.48 * lvl->scale, -1, y - 0.48 * lvl->scale);
				glVertex3f(x + 0.48 * lvl->scale, -1, y + 0.48 * lvl->scale);
				glVertex3f(x - 0.48 * lvl->scale, -1, y + 0.48 * lvl->scale);
			}
			cell++;
		}
	}
	glEnd();

	glViewport(0, 0, win_width, win_height);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, rbuf_width, rbuf_height, 0, 0, win_width, win_height,
			GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void greshape(int x, int y)
{
	cgm_mperspective(proj_mat, cgm_deg_to_rad(60), win_aspect, 0.5, 200.0);
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(proj_mat);

	rbuf_width = ((int)(x * opt.gfx.render_res) + 3) & 0xfffffc;
	rbuf_height = ((int)(y * opt.gfx.render_res) + 3) & 0xfffffc;
	printf("render buffer %dx%d\n", rbuf_width, rbuf_height);

	if(!fbo) {
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		glGenRenderbuffers(1, &rbuf_col);
		glBindRenderbuffer(GL_RENDERBUFFER, rbuf_col);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB, rbuf_width, rbuf_height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbuf_col);

		glGenRenderbuffers(1, &rbuf_zbuf);
		glBindRenderbuffer(GL_RENDERBUFFER, rbuf_zbuf);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, rbuf_width, rbuf_height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbuf_zbuf);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	} else {
		glBindRenderbuffer(GL_RENDERBUFFER, rbuf_col);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB, rbuf_width, rbuf_height);

		glBindRenderbuffer(GL_RENDERBUFFER, rbuf_zbuf);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, rbuf_width, rbuf_height);
	}
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
		player.theta += dx * 0.02;
		player.phi += dy * 0.02;
		if(player.phi < -M_PI/2) player.phi = -M_PI/2;
		if(player.phi > M_PI/2) player.phi = M_PI/2;
	}
	if(mouse_state[1]) {
		float up[3], right[3];

		up[0] = -sin(player.theta) * sin(player.phi);
		up[1] = -cos(player.phi);
		up[2] = cos(player.theta) * sin(player.phi);
		right[0] = cos(player.theta);
		right[1] = 0;
		right[2] = sin(player.theta);

		player.pos.x += (right[0] * dx + up[0] * dy) * 0.01;
		player.pos.y += up[1] * dy * 0.01;
		player.pos.z += (right[2] * dx + up[2] * dy) * 0.01;
	}
	if(mouse_state[2]) {
		cam_dist += dy * 0.01;
		if(cam_dist < 0) cam_dist = 0;
	}
}
