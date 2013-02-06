#ifndef EULER2RGB_H
#define EULER2RGB_H

#include "Function.h"

Real Euler2RGB(unsigned int sd, Real phi1, Real PHI, Real phi2, unsigned int phase, unsigned int sym);

static Real a;
static Real SymOpsCubic;
static Real SymOpsHexagonal;
static Real SymOpsTetragonal;
static Real SymOpsTrigonal;
static Real SymOpsOrthorhombic;
static Real SymOpsMonoclinic;
static Real SymOpsTriclinic;

#endif //Euler2RGB_H
