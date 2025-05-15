//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MassDiffusionBaseGasMixDGKernel.h"

InputParameters
MassDiffusionBaseGasMixDGKernel::validParams()
{
  InputParameters params = ADDGKernel::validParams();

  params.addRequiredParam<MaterialPropertyName>("density", "Mixture density material property");
  params.addRequiredParam<MaterialPropertyName>("diffusion_coefficient",
                                                "Diffusion coefficient material property");
  params.addRequiredParam<MaterialPropertyName>("mass_fraction", "Mass fraction material property");

  params.addRequiredCoupledVar("A_linear", "Cross-sectional area");
  params.addRequiredParam<MaterialPropertyName>("direction",
                                                "Flow channel direction material property");

  params.addClassDescription("Adds mass diffusion for FlowChannelGasMix.");

  return params;
}

MassDiffusionBaseGasMixDGKernel::MassDiffusionBaseGasMixDGKernel(const InputParameters & parameters)
  : ADDGKernel(parameters),

    _rho_elem(getADMaterialProperty<Real>("density")),
    _rho_neig(getNeighborADMaterialProperty<Real>("density")),
    _D_elem(getADMaterialProperty<Real>("diffusion_coefficient")),
    _D_neig(getNeighborADMaterialProperty<Real>("diffusion_coefficient")),
    _mass_fraction_elem(getADMaterialProperty<Real>("mass_fraction")),
    _mass_fraction_neig(getNeighborADMaterialProperty<Real>("mass_fraction")),

    _A_linear(adCoupledValue("A_linear")),
    _dir(getMaterialProperty<RealVectorValue>("direction"))
{
}

ADReal
MassDiffusionBaseGasMixDGKernel::computeQpResidual(Moose::DGResidualType type)
{
  const ADReal flux = computeQpFlux();
  const Real flux_sign_elem = _current_side * 2 - 1.0;
  return type == Moose::Element ? flux_sign_elem * flux * _A_linear[_qp] * _test[_i][_qp]
                                : -flux_sign_elem * flux * _A_linear[_qp] * _test_neighbor[_i][_qp];
}

void
MassDiffusionBaseGasMixDGKernel::computePositionChanges(Real & dx, Real & dx_side) const
{
  const Point x_elem = _current_elem->vertex_average();
  const Point x_neig = _neighbor_elem->vertex_average();
  dx = (x_neig - x_elem) * _dir[_qp];
  dx_side = (_q_point[_qp] - x_elem) * _dir[_qp];
}

ADReal
MassDiffusionBaseGasMixDGKernel::linearlyInterpolate(const ADReal & y_elem,
                                                     const ADReal & y_neig,
                                                     Real dx,
                                                     Real dx_side) const
{
  const ADReal dydx = computeGradient(y_elem, y_neig, dx);

  return y_elem + dydx * dx_side;
}

ADReal
MassDiffusionBaseGasMixDGKernel::computeGradient(const ADReal & y_elem,
                                                 const ADReal & y_neig,
                                                 Real dx) const
{
  return (y_neig - y_elem) / dx;
}
