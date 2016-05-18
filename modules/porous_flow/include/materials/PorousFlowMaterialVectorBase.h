/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWMATERIALVECTORBASE_H
#define POROUSFLOWMATERIALVECTORBASE_H

#include "Material.h"
#include "DerivativeMaterialInterface.h"
#include "PorousFlowDictator.h"

class PorousFlowMaterialVectorBase;

template<>
InputParameters validParams<PorousFlowMaterialVectorBase>();

/**
 * Base class for all PorousFlow materials that combine phase-dependent properties
 * such as fluid properties, relative permeability etc. This base class provides the
 * number of phases and number of fluid components (amongst other things) to all
 * PorousFlow materials that provide the vectors of properties expected by the kernels.
 */
class PorousFlowMaterialVectorBase : public DerivativeMaterialInterface<Material>
{
public:
  PorousFlowMaterialVectorBase(const InputParameters & parameters);

protected:
  /// The PorousFlow Dictator UserObject
  const PorousFlowDictator & _dictator;

  /// Number of phases
  const unsigned int _num_phases;

  /// Number of fluid components
  const unsigned int _num_components;

  /// Number of PorousFlow variables
  const unsigned int _num_var;
};

#endif //POROUSFLOWMATERIALVECTORBASE_H
