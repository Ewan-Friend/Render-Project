#include "Ident.h"
#include "Globals.h"

void Ident::initialise(Camera &camera) {
    frame = 0;
    active = true;
    redraw = true;
}

void Ident::step(Camera &camera){
    //if (frame < 9999){
    //    camera.orbitY(theta);
    //}
    animateScene(frame);
    frame++;
    redraw = true;
}
