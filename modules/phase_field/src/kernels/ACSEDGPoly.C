//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ACSEDGPoly.h"
#include "Material.h"
#include "GrainTrackerInterface.h"

registerMooseObject("PhaseFieldApp", ACSEDGPoly);

InputParameters
ACSEDGPoly::validParams()
{
  InputParameters params = ACBulk<Real>::validParams();
  params.addClassDescription("Stored Energy contribution to grain growth");
  params.addRequiredCoupledVar("v", "Array of coupled variable names");
  params.addRequiredParam<unsigned int>("deformed_grain_num",
                                        "Number of OP representing deformed grains");
  params.addRequiredParam<UserObjectName>("grain_tracker",
                                          "The GrainTracker UserObject to get values from.");
  params.addRequiredParam<unsigned int>("op_index", "The index for the current order parameter");
  return params;
}

ACSEDGPoly::ACSEDGPoly(const InputParameters & parameters)
  : ACBulk<Real>(parameters),
    _op_num(coupledComponents("v")),
    _vals(coupledValues("v")),
    _vals_var(coupledIndices("v")),
    _beta(getMaterialProperty<Real>("beta")),
    _rho_eff(getMaterialProperty<Real>("rho_eff")),
    _Disloc_Den_i(getMaterialProperty<Real>("Disloc_Den_i")),
    _deformed_grain_num(getParam<unsigned int>("deformed_grain_num")),
    _grain_tracker(getUserObject<GrainTrackerInterface>("grain_tracker")),
    _op_index(getParam<unsigned int>("op_index"))
{
}

Real
ACSEDGPoly::computeDFDOP(PFFunctionType type)
{
  Real SumEtaj = 0.0;
  for (unsigned int i = 0; i < _op_num; ++i)
    SumEtaj += (*_vals[i])[_qp] * (*_vals[i])[_qp];

  // Add the current OP to the sum
  Real SumEtai2 = SumEtaj + _u[_qp] * _u[_qp];
  // Dislocation density in deformed grains
  Real rho_i = _Disloc_Den_i[_qp];
  // undeformed grains are dislocation-free
  const auto & op_to_grain = _grain_tracker.getVarToFeatureVector(_current_elem->id());
  const auto grn_index = op_to_grain[_op_index];
  if (grn_index >= _deformed_grain_num)
    rho_i = 0.0;

  // Calculate the contributions of the deformation energy to the residual and Jacobian
  Real drho_eff_detai = 2.0 * _u[_qp] * (rho_i - _rho_eff[_qp]) / SumEtai2;

  // Calculate the Stored Energy contribution to either the residual or Jacobian of the grain growth
  // free energy
  switch (type)
  {
    case Residual:
      return _beta[_qp] * drho_eff_detai;

    case Jacobian:
      return _beta[_qp] * _phi[_j][_qp] *
             (2.0 * SumEtai2 * ((rho_i - _rho_eff[_qp]) - _u[_qp] * drho_eff_detai) -
              4.0 * _u[_qp] * _u[_qp] * (rho_i - _rho_eff[_qp])) /
             (SumEtai2 * SumEtai2);
  }
  mooseError("Invalid type passed in");
}
