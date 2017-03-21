/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWFLUIDPROPERTIESBASE_H
#define POROUSFLOWFLUIDPROPERTIESBASE_H

#include "PorousFlowMaterialBase.h"
#include "PorousFlowDictator.h"

class PorousFlowFluidPropertiesBase;

template <>
InputParameters validParams<PorousFlowFluidPropertiesBase>();

/**
 * Base class for fluid properties materials. All PorousFlow fluid
 * materials must override computeQpProperties()
 */
class PorousFlowFluidPropertiesBase : public PorousFlowMaterialBase
{
public:
  PorousFlowFluidPropertiesBase(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Pore pressure at the nodes or quadpoints
  const MaterialProperty<std::vector<Real>> & _porepressure;

  /// Fluid temperature at the nodes or quadpoints
  const MaterialProperty<Real> & _temperature;

  /// Name of (dummy) pressure primary variable
  const VariableName _pressure_variable_name;

  /// Name of (dummy) temperature primary variable
  const VariableName _temperature_variable_name;

  /// Conversion from degrees Celsius to degrees Kelvin
  const Real _t_c2k;

  /// Universal gas constant
  const Real _R;
};

#endif // POROUSFLOWFLUIDPROPERTIESBASE_H
