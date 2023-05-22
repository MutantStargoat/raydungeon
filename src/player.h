#ifndef PLAYER_H_
#define PLAYER_H_

#include "cgmath/cgmath.h"

struct player {
	cgm_vec3 pos;
	float theta, phi;

	cgm_vec2 mouselook;
	cgm_vec3 move;

	float matrix[16], inv_matrix[16];
};

void init_player(struct player *p);

void update_player_mouse(struct player *p);
void update_player(struct player *p);

void calc_player_matrix(struct player *p);

#endif	/* PLAYER_H_ */
