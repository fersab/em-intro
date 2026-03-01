#ifndef STARS_H
#define STARS_H

// pre-compute layer colors and randomize star positions
void stars_init(void);

// scroll stars left at layer-dependent speeds, wrap around
void stars_update(float dt);

// render all stars as single pixels
void stars_draw(void);

#endif
