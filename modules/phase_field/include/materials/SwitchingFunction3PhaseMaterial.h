/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SWITCHINGFUNCTION3PHASEMATERIAL_H
#define SWITCHINGFUNCTION3PHASEMATERIAL_H

#include "DerivativeParsedMaterialHelper.h"
#include "ExpressionBuilder.h"

// Forward Declarations
class SwitchingFunction3PhaseMaterial;

template <>
InputParameters validParams<SwitchingFunction3PhaseMaterial>();

/**
 * Material class to provide switching functions that prevent formation of a
 * third phase at a two-phase interface. See Folch and Plapp, Phys. Rev. E, v. 72,
 * 011602 (2005).
 */
class SwitchingFunction3PhaseMaterial : public DerivativeParsedMaterialHelper,
                                        public ExpressionBuilder
{
public:
  SwitchingFunction3PhaseMaterial(const InputParameters & parameters);

protected:
  /// Coupled variable values for order parameters.
  EBTerm _eta_i;
  EBTerm _eta_j;
  EBTerm _eta_k;
};

#endif // SWITCHINGFUNCTION3PHASEMATERIAL_H
