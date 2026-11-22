#include "Ident.h"

void Ident::initialise(Camera &camera) {
    frame = 0;
    active = true;
    redraw = true;
}

void Ident::step(Camera &camera){
    if (frame < 120){
        camera.orbitY(theta);
    }
    frame++;
    redraw = true;
}
