/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWPERMEABILITYCONST_H
#define POROUSFLOWPERMEABILITYCONST_H

#include "PorousFlowPermeabilityUnity.h"

//Forward Declarations
class PorousFlowPermeabilityConst;

template<>
InputParameters validParams<PorousFlowPermeabilityConst>();

/**
 * Material designed to provide the permeability tensor
 * which is assumed constant
 */
class PorousFlowPermeabilityConst : public PorousFlowPermeabilityUnity
{
public:
  PorousFlowPermeabilityConst(const InputParameters & parameters);

protected:
  /// constant value of permeability tensor
  const RealTensorValue _input_permeability;

  void computeQpProperties();
};

#endif //POROUSFLOWPERMEABILITYCONST_H
