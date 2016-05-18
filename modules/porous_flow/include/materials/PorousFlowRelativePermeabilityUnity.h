/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWRELATIVEPERMEABILITYUNITY_H
#define POROUSFLOWRELATIVEPERMEABILITYUNITY_H

#include "DerivativeMaterialInterface.h"
#include "Material.h"
#include "PorousFlowDictator.h"

class PorousFlowRelativePermeabilityUnity;

template<>
InputParameters validParams<PorousFlowRelativePermeabilityUnity>();

/**
 * Base class for relative permeability materials
 * This class simply sets relative permeability = 1 at the nodes
 */
class PorousFlowRelativePermeabilityUnity : public DerivativeMaterialInterface<Material>
{
public:
  PorousFlowRelativePermeabilityUnity(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// Phase number of fluid that this relative permeability relates to
  const unsigned int _phase_num;

  /// The PorousFlowDictator UserObject
  const PorousFlowDictator & _dictator;

  /// Name of (dummy) saturation primary variable
  VariableName _saturation_variable_name;

  /// Saturation material property
  const MaterialProperty<std::vector<Real> > & _saturation_nodal;

  /// Relative permeability material property
  MaterialProperty<Real> & _relative_permeability;

  /// Derivative of relative permeability wrt phase saturation
  MaterialProperty<Real> & _drelative_permeability_ds;
};

#endif //POROUSFLOWRELATIVEPERMEABILITYUNITY_H
