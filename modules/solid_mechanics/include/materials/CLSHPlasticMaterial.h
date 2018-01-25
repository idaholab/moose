//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef CLSHPLASTICMATERIAL_H
#define CLSHPLASTICMATERIAL_H

#include "SolidModel.h"

class CLSHPlasticMaterial;

template <>
InputParameters validParams<CLSHPlasticMaterial>();

/**
 * Plastic material
 */
class CLSHPlasticMaterial : public SolidModel
{
public:
  CLSHPlasticMaterial(const InputParameters & parameters);

protected:
};

#endif // CLSHPLASTICMATERIAL_H
