/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWMATERIALFLUIDPROPERTIESBASE_H
#define POROUSFLOWMATERIALFLUIDPROPERTIESBASE_H

#include "DerivativeMaterialInterface.h"
#include "Material.h"
#include "PorousFlowDictator.h"

class PorousFlowMaterialFluidPropertiesBase;

template<>
InputParameters validParams<PorousFlowMaterialFluidPropertiesBase>();

/**
 * Base class for fluid properties
 */
class PorousFlowMaterialFluidPropertiesBase : public DerivativeMaterialInterface<Material>
{
public:
  PorousFlowMaterialFluidPropertiesBase(const InputParameters & parameters);

protected:
  /// Somehow should be able to make this pure virtual?
  virtual void computeQpProperties();

  /// Phase number of fluid that this relative permeability relates to
  const unsigned int _phase_num;
  /// Pore pressure at the nodes
  const MaterialProperty<std::vector<Real> > & _porepressure_nodal;
  /// Pore pressure at the qps
  const MaterialProperty<std::vector<Real> > & _porepressure_qp;
  /// Fluid temperature at the nodes
  const MaterialProperty<std::vector<Real> > & _temperature_nodal;
  /// Fluid temperature at the qps
  const MaterialProperty<std::vector<Real> > & _temperature_qp;
  /// The PorousFlowDictator UserObject
  const PorousFlowDictator & _dictator_UO;
  /// Name of (dummy) pressure primary variable
  const VariableName _pressure_variable_name;
  /// Name of (dummy) temperature primary variable
  const VariableName _temperature_variable_name;
  /// Conversion from degrees Celsius to degrees Kelvin
  Real _t_c2k;
  /// Universal gas constant
  Real _R;
};

#endif //POROUSFLOWMATERIALFLUIDPROPERTIESBASE_H
