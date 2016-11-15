/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWPOROSITYTHM_H
#define POROUSFLOWPOROSITYTHM_H

#include "PorousFlowPorosityBase.h"

//Forward Declarations
class PorousFlowPorosityTHM;

template<>
InputParameters validParams<PorousFlowPorosityTHM>();

/**
 * Material designed to provide the porosity in thermo-hydro-mechanical simulations
 * biot + (phi0 - biot)*exp(-vol_strain + coeff * effective_pressure + thermal_exp_coeff * temperature)
 */
class PorousFlowPorosityTHM : public PorousFlowPorosityBase
{
public:
  PorousFlowPorosityTHM(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;

  virtual void computeQpProperties() override;

  /// porosity at zero strain and zero porepressure and zero temperature
  const VariableValue & _phi0;

  /// biot coefficient
  const Real _biot;

  /// thermal expansion coefficient of the solid porous skeleton
  const Real _exp_coeff;

  /// drained bulk modulus of the porous skeleton
  const Real _solid_bulk;

  /// short-hand number (biot-1)/solid_bulk
  const Real _coeff;

  /// number of displacement variables
  const unsigned int _ndisp;

  /// variable number of the displacements variables
  std::vector<unsigned int> _disp_var_num;

  /// strain
  const MaterialProperty<Real> & _vol_strain_qp;

  /// d(strain)/(dvar)
  const MaterialProperty<std::vector<RealGradient> > & _dvol_strain_qp_dvar;

  /// effective nodal porepressure
  const MaterialProperty<Real> & _pf_nodal;

  /// d(effective nodal porepressure)/(d porflow variable)
  const MaterialProperty<std::vector<Real> > & _dpf_nodal_dvar;

  /// effective qp porepressure
  const MaterialProperty<Real> & _pf_qp;

  /// d(effective qp porepressure)/(d porflow variable)
  const MaterialProperty<std::vector<Real> > & _dpf_qp_dvar;

  /// nodal temperature
  const MaterialProperty<Real> & _temperature_nodal;

  /// d(nodal temperature)/(d porflow variable)
  const MaterialProperty<std::vector<Real> > & _dtemperature_nodal_dvar;

  /// qp temperature
  const MaterialProperty<Real> & _temperature_qp;

  /// d(qp temperature)/(d porflow variable)
  const MaterialProperty<std::vector<Real> > & _dtemperature_qp_dvar;
};

#endif //POROUSFLOWPOROSITYTHM_H
