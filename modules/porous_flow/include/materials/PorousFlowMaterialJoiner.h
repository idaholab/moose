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
 * from the individual phase densities
 */
class PorousFlowMaterialJoiner : public DerivativeMaterialInterface<Material>
{
public:
  PorousFlowMaterialJoiner(const InputParameters & parameters);

protected:

  /// The variable names UserObject for the Porous-Flow variables
  const PorousFlowDictator & _porflow_name_UO;
  /// Bool to choose quadpoints instead of nodes (default is false = nodes)
  const bool _use_qps;

  const unsigned int _num_phases;

  const std::string _pf_prop;

  /// Derivatives of nodal porepressure variable wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<Real> > > & _dporepressure_dvar;
  /// Derivatives of nodal saturation variable wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<Real> > > & _dsaturation_dvar;
  /// Derivatives of nodal temperature variable wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<Real> > > & _dtemperature_dvar;
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

  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();
};

#endif //PORFLOWMATERIALJOINER_H
