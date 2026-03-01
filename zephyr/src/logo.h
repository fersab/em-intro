#ifndef LOGO_EFFECT_H
#define LOGO_EFFECT_H

#include <stdbool.h>

// load logo from flash data and build particle arrays
void logo_init(void);

// advance logo animation timer
void logo_update(float dt);

// render logo: particles during convergence, sprite + shake after
void logo_draw(void);

// true when convergence + wave animation has finished
bool logo_is_done(void);

#endif
