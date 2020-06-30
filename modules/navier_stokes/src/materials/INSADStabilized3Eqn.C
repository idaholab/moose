//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Navier-Stokes includes
#include "INSADStabilized3Eqn.h"

registerMooseObject("NavierStokesApp", INSADStabilized3Eqn);

InputParameters
INSADStabilized3Eqn::validParams()
{
  InputParameters params = INSADTauMaterialTempl<INSAD3Eqn>::validParams();
  params.addClassDescription("This is the material class used to compute the stabilization "
                             "parameter tau for momentum and tau_energy for the energy equation.");
  return params;
}

INSADStabilized3Eqn::INSADStabilized3Eqn(const InputParameters & parameters)
  : INSADTauMaterialTempl<INSAD3Eqn>(parameters), _tau_energy(declareADProperty<Real>("tau_energy"))
{
}

void
INSADStabilized3Eqn::computeQpProperties()
{
  INSADTauMaterialTempl<INSAD3Eqn>::computeQpProperties();

  auto dissipation_coefficient = _k[_qp] / (_rho[_qp] * _cp[_qp]);
  auto transient_part = _has_transient ? 4. / (_dt * _dt) : 0.;
  _tau_energy[_qp] = _alpha / std::sqrt(transient_part +
                                        (2. * _velocity[_qp].norm() / _hmax) *
                                            (2. * _velocity[_qp].norm() / _hmax) +
                                        9. * (4. * dissipation_coefficient / (_hmax * _hmax)) *
                                            (4. * dissipation_coefficient / (_hmax * _hmax)));
}
