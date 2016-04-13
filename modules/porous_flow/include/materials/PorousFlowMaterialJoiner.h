/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef PORFLOWMATERIALJOINER_H
#define PORFLOWMATERIALJOINER_H

#include "DerivativeMaterialInterface.h"
#include "Material.h"

#include "PorousFlowDictator.h"

//Forward Declarations
class PorousFlowMaterialJoiner;

template<>
InputParameters validParams<PorousFlowMaterialJoiner>();

/**
 * Material designed to form a std::vector of property
 * from the individual phase properties.  Old values are not included.
 */
class PorousFlowMaterialJoiner : public DerivativeMaterialInterface<Material>
{
public:
  PorousFlowMaterialJoiner(const InputParameters & parameters);

protected:

  virtual void computeQpProperties();

  /// The variable names UserObject for the Porous-Flow variables
  const PorousFlowDictator & _dictator_UO;
  /// Name of (dummy) pressure variable
  const VariableName _pressure_variable_name;
  /// Name of (dummy) saturation variable
  const VariableName _saturation_variable_name;
  /// Name of (dummy) temperature variable
  const VariableName _temperature_variable_name;
  /// Name of (dummy) mass fraction variable
  const VariableName _mass_fraction_variable_name;
  /// Bool to choose quadpoints instead of nodes (default is false = nodes)
  const bool _use_qps;
  /// Number of phases
  const unsigned int _num_phases;
  /// Name of material property to be joined
  const std::string _pf_prop;
  /// Derivatives of nodal porepressure variable wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<Real> > > & _dporepressure_nodal_dvar;
  /// Derivatives of nodal saturation variable wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<Real> > > & _dsaturation_nodal_dvar;
  /// Derivatives of nodal temperature variable wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<Real> > > & _dtemperature_nodal_dvar;
  /// Derivatives of quadpoint porepressure variable wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<Real> > > & _dporepressure_qp_dvar;
  /// Derivatives of quadpoint saturation variable wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<Real> > > & _dsaturation_qp_dvar;
  /// Derivatives of quadpoint temperature variable wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<Real> > > & _dtemperature_qp_dvar;
  /// computed property of the phase
  MaterialProperty<std::vector<Real> > & _property;
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

#endif //PORFLOWMATERIALJOINER_H
