/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef EULER2RGB_H
#define EULER2RGB_H

#include "Function.h"

Point
euler2RGB(unsigned int sd, Real phi1, Real PHI, Real phi2, unsigned int phase, unsigned int sym);

#endif // EULER2RGB_H
