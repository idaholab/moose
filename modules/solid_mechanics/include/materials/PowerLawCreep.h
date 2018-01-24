/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef POWERLAWCREEP_H
#define POWERLAWCREEP_H

#include "SolidModel.h"

// Forward declarations
class PowerLawCreep;

template <>
InputParameters validParams<PowerLawCreep>();

/**
 * Power-law creep material
 * edot = A(sigma)**n * exp(-Q/(RT))
 */

class PowerLawCreep : public SolidModel
{
public:
  PowerLawCreep(const InputParameters & parameters);

protected:
};

#endif // POWERLAWCREEPMATERIAL_H
