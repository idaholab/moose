/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWMATERIALVECTORBASE_H
#define POROUSFLOWMATERIALVECTORBASE_H

#include "PorousFlowMaterial.h"
#include "DerivativeMaterialInterface.h"

class PorousFlowMaterialVectorBase;

template <>
InputParameters validParams<PorousFlowMaterialVectorBase>();

/**
 * Base class for all PorousFlow vector materials
 */
class PorousFlowMaterialVectorBase : public DerivativeMaterialInterface<PorousFlowMaterial>
{
public:
  PorousFlowMaterialVectorBase(const InputParameters & parameters);

protected:
  /// Number of phases
  const unsigned int _num_phases;

  /// Number of fluid components
  const unsigned int _num_components;

  /// Number of PorousFlow variables
  const unsigned int _num_var;
};

#endif // POROUSFLOWMATERIALVECTORBASE_H
