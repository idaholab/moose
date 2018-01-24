/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWPERMEABILITYCONST_H
#define POROUSFLOWPERMEABILITYCONST_H

#include "PorousFlowPermeabilityBase.h"

// Forward Declarations
class PorousFlowPermeabilityConst;

template <>
InputParameters validParams<PorousFlowPermeabilityConst>();

/**
 * Material designed to provide a constant permeability tensor
 */
class PorousFlowPermeabilityConst : public PorousFlowPermeabilityBase
{
public:
  PorousFlowPermeabilityConst(const InputParameters & parameters);

protected:
  void computeQpProperties() override;

  /// constant value of permeability tensor
  const RealTensorValue _input_permeability;
};

#endif // POROUSFLOWPERMEABILITYCONST_H
