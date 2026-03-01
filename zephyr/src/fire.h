#ifndef FIRE_H
#define FIRE_H

#define FIRE_RATE (1.0f / 20.0f)

// build fire palette: black -> red -> yellow -> white
void fire_init(void);

// one step of fire simulation: seed bottom rows, average and propagate up
void fire_update(void);

// render fire buffer to framebuffer using the heat palette
void fire_draw(void);

#endif
