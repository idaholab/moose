/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
