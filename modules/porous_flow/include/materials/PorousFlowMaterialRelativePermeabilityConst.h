/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef POROUSFLOWMATERIALRELATIVEPERMEABILITYCONST_H
#define POROUSFLOWMATERIALRELATIVEPERMEABILITYCONST_H

#include "DerivativeMaterialInterface.h"
#include "Material.h"

#include "PorousFlowDictator.h"

//Forward Declarations
class PorousFlowMaterialRelativePermeabilityConst;

template<>
InputParameters validParams<PorousFlowMaterialRelativePermeabilityConst>();

/**
 * Material designed to provide the porosity
 * which is assumed constant
 */
class PorousFlowMaterialRelativePermeabilityConst : public DerivativeMaterialInterface<Material>
{
public:
  PorousFlowMaterialRelativePermeabilityConst(const InputParameters & parameters);

protected:


  /// The variable names UserObject for the Porous-Flow variables
  const PorousFlowDictator & _dictator_UO;

  /// relative permeability
  MaterialProperty<std::vector<Real> > & _relative_permeability;


  /// d(relperm)/d(PorousFlow variable)
  MaterialProperty<std::vector<std::vector<Real> > > & _drelative_permeability_dvar;

  virtual void computeQpProperties();
};

#endif //POROUSFLOWMATERIALRELATIVEPERMEABILITYCONST_H
