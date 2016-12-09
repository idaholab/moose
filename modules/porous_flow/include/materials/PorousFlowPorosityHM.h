/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWPOROSITYHM_H
#define POROUSFLOWPOROSITYHM_H

#include "PorousFlowPorosityBase.h"

//Forward Declarations
class PorousFlowPorosityHM;

template<>
InputParameters validParams<PorousFlowPorosityHM>();

/**
 * Material designed to provide the porosity in hydro-mechanical simulations
 * biot + (phi0 - biot)*exp(-vol_strain + (biot-1)effective_porepressure/solid_bulk)
 */
class PorousFlowPorosityHM : public PorousFlowPorosityBase
{
public:
  PorousFlowPorosityHM(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// porosity at zero strain and zero porepressure
  const VariableValue & _phi0;

  /// biot coefficient
  const Real _biot;

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

  /// effective porepressure
  const MaterialProperty<Real> & _pf;

  /// d(effective porepressure)/(d porflow variable)
  const MaterialProperty<std::vector<Real> > & _dpf_dvar;
};

#endif //POROUSFLOWPOROSITYHM_H
