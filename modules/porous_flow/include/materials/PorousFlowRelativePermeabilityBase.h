/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWRELATIVEPERMEABILITYBASE_H
#define POROUSFLOWRELATIVEPERMEABILITYBASE_H

#include "PorousFlowMaterialBase.h"

class PorousFlowRelativePermeabilityBase;

template<>
InputParameters validParams<PorousFlowRelativePermeabilityBase>();

/**
 * Base class for PorousFlow relative permeability materials. All materials
 * that derive from this class must override computeQpProperties()
 */
class PorousFlowRelativePermeabilityBase : public PorousFlowMaterialBase
{
public:
  PorousFlowRelativePermeabilityBase(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// Name of (dummy) saturation primary variable
  VariableName _saturation_variable_name;

  /// Saturation material property
  const MaterialProperty<std::vector<Real> > & _saturation_nodal;

  /// Relative permeability material property
  MaterialProperty<Real> & _relative_permeability;

  /// Derivative of relative permeability wrt phase saturation
  MaterialProperty<Real> & _drelative_permeability_ds;
};

#endif //POROUSFLOWRELATIVEPERMEABILITYBASE_H
