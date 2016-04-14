/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef PORFLOWMATERIALRELATIVEPERMEABILITYCONST_H
#define PORFLOWMATERIALRELATIVEPERMEABILITYCONST_H

#include "DerivativeMaterialInterface.h"
#include "Material.h"

#include "PorFlowVarNames.h"

//Forward Declarations
class PorFlowMaterialRelativePermeabilityConst;

template<>
InputParameters validParams<PorFlowMaterialRelativePermeabilityConst>();

/**
 * Material designed to provide the porosity
 * which is assumed constant
 */
class PorFlowMaterialRelativePermeabilityConst : public DerivativeMaterialInterface<Material>
{
public:
  PorFlowMaterialRelativePermeabilityConst(const InputParameters & parameters);

protected:


  /// The variable names UserObject for the Porous-Flow variables
  const PorFlowVarNames & _porflow_name_UO;

  /// relative permeability
  MaterialProperty<std::vector<Real> > & _relative_permeability;


  /// d(relperm)/d(PorFlow variable)
  MaterialProperty<std::vector<std::vector<Real> > > & _drelative_permeability_dvar;

  virtual void computeQpProperties();
};

#endif //PORFLOWMATERIALRELATIVEPERMEABILITYCONST_H
