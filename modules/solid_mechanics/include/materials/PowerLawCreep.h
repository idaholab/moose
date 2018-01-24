//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
