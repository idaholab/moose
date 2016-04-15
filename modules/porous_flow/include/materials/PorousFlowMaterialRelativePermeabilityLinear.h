/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef POROUSFLOWMATERIALRELATIVEPERMEABILITYLINEAR_H
#define POROUSFLOWMATERIALRELATIVEPERMEABILITYLINEAR_H

#include "DerivativeMaterialInterface.h"
#include "Material.h"

#include "PorousFlowDictator.h"

//Forward Declarations
class PorousFlowMaterialRelativePermeabilityLinear;

template<>
InputParameters validParams<PorousFlowMaterialRelativePermeabilityLinear>();

/**
 * Linear relative permeability (equal to the phase saturation)
 */
class PorousFlowMaterialRelativePermeabilityLinear : public DerivativeMaterialInterface<Material>
{
public:
  PorousFlowMaterialRelativePermeabilityLinear(const InputParameters & parameters);

protected:

  virtual void computeQpProperties();

  /// The variable names UserObject for the PorousFlow variables
  const PorousFlowDictator & _dictator_UO;
  /// Phase number of fluid that this relative permeability relates to
  const unsigned int _phase_num;
  /// Name of (dummy) saturation primary variable
  VariableName _saturation_variable_name;
  /// Relative permeability material property
  MaterialProperty<Real> & _relative_permeability;
  /// Derivative of relative permeability wrt phase saturation
  MaterialProperty<Real> & _drelative_permeability_ds;
  /// Saturation material property
  const MaterialProperty<std::vector<Real> > & _saturation;
};

#endif //POROUSFLOWMATERIALRELATIVEPERMEABILITYLINEAR_H
