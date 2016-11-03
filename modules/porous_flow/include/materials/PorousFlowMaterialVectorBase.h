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
 * Base class for all PorousFlow vector materials
 */
class PorousFlowMaterialVectorBase : public DerivativeMaterialInterface<Material>
{
public:
  PorousFlowMaterialVectorBase(const InputParameters & parameters);

protected:
  /// The PorousFlow Dictator UserObject
  const PorousFlowDictator & _dictator;

  /// Nearest node number for each quadpoint
  const MaterialProperty<unsigned int> & _node_number;

  /// Number of phases
  const unsigned int _num_phases;

  /// Number of fluid components
  const unsigned int _num_components;

  /// Number of PorousFlow variables
  const unsigned int _num_var;
};

#endif //POROUSFLOWMATERIALVECTORBASE_H
