#ifndef INPUT_H_
#define INPUT_H_

/* game input actions */
enum {
	INP_FWD,
	INP_BACK,
	INP_TLEFT,
	INP_TRIGHT,
	INP_SLEFT,
	INP_SRIGHT,

	MAX_INPUTS
};

#define INP_FWD_BIT		(1 << INP_FWD)
#define INP_BACK_BIT	(1 << INP_BACK)
#define INP_TLEFT_BIT	(1 << INP_TLEFT)
#define INP_TRIGHT_BIT	(1 << INP_TRIGHT)
#define INP_SLEFT_BIT	(1 << INP_SLEFT)
#define INP_SRIGHT_BIT	(1 << INP_SRIGHT)

#define INP_MOVE_BITS	\
	(INP_FWD_BIT | INP_BACK_BIT | INP_TLEFT_BIT | INP_TRIGHT_BIT | \
	 INP_SLEFT_BIT | INP_SRIGHT_BIT)

struct input_map {
	int inp, key, mbn;
};
extern struct input_map inpmap[MAX_INPUTS];

extern unsigned int inpstate;

void init_input(void);

#endif	/* INPUT_H_ */
