//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef CROSSTERMBARRIERFUNCTIONMATERIAL_H
#define CROSSTERMBARRIERFUNCTIONMATERIAL_H

#include "CrossTermBarrierFunctionBase.h"

// Forward Declarations
class CrossTermBarrierFunctionMaterial;

template <>
InputParameters validParams<CrossTermBarrierFunctionMaterial>();

/**
 * CrossTermBarrierFunctionMaterial adds free energy contribution on the interfaces
 * between arbitrary pairs of phases in a symmetric way.
 */
class CrossTermBarrierFunctionMaterial : public CrossTermBarrierFunctionBase
{
public:
  CrossTermBarrierFunctionMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();
};

#endif // CROSSTERMBARRIERFUNCTIONMATERIAL_H
