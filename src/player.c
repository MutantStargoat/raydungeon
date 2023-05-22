#include <string.h>
#include "player.h"
#include "options.h"

void init_player(struct player *p)
{
	memset(p, 0, sizeof *p);

	cgm_midentity(p->matrix);
	cgm_midentity(p->inv_matrix);
}

#define MOUSE_SPEED		(opt.mouse_speed * 0.0001)

void update_player_mouse(struct player *p)
{
	p->theta += p->mouselook.x * MOUSE_SPEED;
	p->phi += p->mouselook.y * MOUSE_SPEED;
	if(p->phi < -M_PI / 2.0) p->phi = -M_PI / 2.0;
	if(p->phi > M_PI / 2.0) p->phi = M_PI / 2.0;

	p->mouselook.x = p->mouselook.y = 0.0f;
}

void update_player(struct player *p)
{
	cgm_vec3 right, fwd, vel;

	vel.x = cos(p->theta) * p->move.x + sin(p->theta) * p->move.z;
	vel.y = 0;
	vel.z = -sin(p->theta) * p->move.x + cos(p->theta) * p->move.z;
	cgm_vcons(&p->move, 0, 0, 0);

	cgm_vadd(&p->pos, &vel);
}

void calc_player_matrix(struct player *p)
{
	cgm_midentity(p->matrix);
	cgm_mrotate_x(p->matrix, p->phi);
	cgm_mrotate_y(p->matrix, p->theta);
	cgm_mtranslate(p->matrix, p->pos.x, p->pos.y, p->pos.z);

	cgm_mcopy(p->inv_matrix, p->matrix);
	cgm_minverse(p->inv_matrix);
}
