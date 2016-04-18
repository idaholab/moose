/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef PORFLOWMATERIALJOINEROLD_H
#define PORFLOWMATERIALJOINEROLD_H

#include "DerivativeMaterialInterface.h"
#include "Material.h"

#include "PorousFlowDictator.h"

//Forward Declarations
class PorousFlowMaterialJoinerOld;

template<>
InputParameters validParams<PorousFlowMaterialJoinerOld>();

/**
 * Material designed to form a std::vector of property
 * from the individual phase densities
 */
class PorousFlowMaterialJoinerOld : public DerivativeMaterialInterface<Material>
{
public:
  PorousFlowMaterialJoinerOld(const InputParameters & parameters);

protected:

  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();

  /// The variable names UserObject for the Porous-Flow variables
  const PorousFlowDictator & _dictator_UO;
  /// Name of (dummy) pressure variable
  VariableName _pressure_variable_name;
  /// Name of (dummy) saturation variable
  VariableName _saturation_variable_name;
  /// Name of (dummy) temperature variable
  VariableName _temperature_variable_name;
  /// Name of (dummy) mass fraction variable
  VariableName _mass_fraction_variable_name;
  /// Number of phases
  const unsigned int _num_phases;
  /// Name of material property to be joined
  const std::string _pf_prop;
  /// Derivatives of porepressure variable wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<Real> > > & _dporepressure_dvar;
  /// Derivatives of saturation variable wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<Real> > > & _dsaturation_dvar;
  /// Derivatives of temperature variable wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<Real> > > & _dtemperature_dvar;
  /// computed property of the phase
  MaterialProperty<std::vector<Real> > & _property;
  /// old value of property of the phase
  MaterialProperty<std::vector<Real> > & _property_old;
  /// d(property)/d(PorousFlow variable)
  MaterialProperty<std::vector<std::vector<Real> > > & _dproperty_dvar;
  /// property of each phase
  std::vector<const MaterialProperty<Real> *> _phase_property;
  /// d(property of each phase)/d(pressure)
  std::vector<const MaterialProperty<Real> *> _dphase_property_dp;
  /// d(property of each phase)/d(saturation)
  std::vector<const MaterialProperty<Real> *> _dphase_property_ds;
  /// d(property of each phase)/d(temperature)
  std::vector<const MaterialProperty<Real> *> _dphase_property_dt;
};

#endif //PORFLOWMATERIALJOINEROLD_H
