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

  virtual void computeQpProperties();

  /// Phase number of fluid that this relative permeability relates to
  const unsigned int _phase_num;
  /// Pore pressure at the nodes
  const MaterialProperty<std::vector<Real> > & _porepressure;
  /// Pore pressure at the qps
  const MaterialProperty<std::vector<Real> > & _porepressure_qp;
  /// Fluid temperature at the nodes
  const MaterialProperty<std::vector<Real> > & _temperature;
  /// Fluid temperature at the qps
  const MaterialProperty<std::vector<Real> > & _temperature_qp;
  /// Name of (dummy) pressure primary variable
  VariableName _pressure_variable_name;
  /// Name of (dummy) temperature primary variable
  VariableName _temperature_variable_name;
  /// The PorousFlowDictator UserObject
  const PorousFlowDictator & _dictator_UO;
  /// Conversion from degrees Celsius to degrees Kelvin
  Real _t_c2k;
  /// Universal gas constant
  Real _R;
};

#endif //POROUSFLOWMATERIALFLUIDPROPERTIESBASE_H
