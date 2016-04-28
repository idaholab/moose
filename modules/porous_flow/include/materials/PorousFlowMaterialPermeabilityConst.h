/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWMATERIALPERMEABILITYCONST_H
#define POROUSFLOWMATERIALPERMEABILITYCONST_H

#include "DerivativeMaterialInterface.h"
#include "Material.h"

#include "PorousFlowDictator.h"

//Forward Declarations
class PorousFlowMaterialPermeabilityConst;

template<>
InputParameters validParams<PorousFlowMaterialPermeabilityConst>();

/**
 * Material designed to provide the permeability tensor
 * which is assumed constant
 */
class PorousFlowMaterialPermeabilityConst : public DerivativeMaterialInterface<Material>
{
public:
  PorousFlowMaterialPermeabilityConst(const InputParameters & parameters);

protected:
  /// constant value of permeability tensor
  const RealTensorValue _input_permeability;

  /// The variable names UserObject for the Porous-Flow variables
  const PorousFlowDictator & _PorousFlow_name_UO;

  /// permeability
  MaterialProperty<RealTensorValue> & _permeability;

  /// d(permeability)/d(PorousFlow variable) which are all zero in this case
  MaterialProperty<std::vector<RealTensorValue> > & _dpermeability_dvar;

  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();
};

#endif //POROUSFLOWMATERIALPERMEABILITYCONST_H
