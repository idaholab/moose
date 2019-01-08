//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "ADDensity.h"

registerADMooseObject("MiscApp", ADDensity);

defineADValidParams(
    ADDensity, ADMaterial, params.addClassDescription("Creates density AD material property");
    params.addCoupledVar("disp_r", "The r displacement");
    params.addCoupledVar("disp_x", "The x displacement");
    params.addCoupledVar("disp_y", "The y displacement");
    params.addCoupledVar("disp_z", "The z displacement");
    params.addCoupledVar(
        "displacements",
        "The displacements appropriate for the simulation geometry and coordinate system");
    params.addRequiredParam<Real>("density", "Density"););

template <ComputeStage compute_stage>
ADDensity<compute_stage>::ADDensity(const InputParameters & parameters)
  : ADMaterial<compute_stage>(parameters),
    _is_coupled(true),
    _disp_r(isCoupled("displacements")
                ? adCoupledValue("displacements", 0)
                : (isCoupled("disp_r") ? adCoupledValue("disp_r") : adZero())),
    _orig_density(adGetParam<Real>("density")),
    _density(adDeclareADProperty<Real>("density"))
{
  const bool is_r_coupled = isCoupled("disp_r");
  const bool is_x_coupled = isCoupled("disp_x");
  const bool is_y_coupled = isCoupled("disp_y");
  const bool is_z_coupled = isCoupled("disp_z");

  // new parameter scheme
  if (isCoupled("displacements"))
  {
    // get coordinate system
    _coord_system = getBlockCoordSystem();

    // get coupled gradients
    const unsigned int ndisp = coupledComponents("displacements");
    _grad_disp.resize(ndisp);
    for (unsigned int i = 0; i < ndisp; ++i)
      _grad_disp[i] = &adCoupledGradient("displacements", i);

    // fill remaining components with zero
    _grad_disp.resize(3, &adGradZero());
  }

  // old deprecated parameters
  else if (is_x_coupled || is_r_coupled)
  {
    // guess(!) coordinate system
    if (is_r_coupled)
    {
      if (is_z_coupled)
        _coord_system = Moose::COORD_RZ;
      else
        _coord_system = Moose::COORD_RSPHERICAL;
    }
    else
      _coord_system = Moose::COORD_XYZ;

    // couple gradients
    _grad_disp = {is_x_coupled ? &adCoupledGradient("disp_x")
                               : (is_r_coupled ? &adCoupledGradient("disp_r") : &adGradZero()),
                  is_y_coupled ? &adCoupledGradient("disp_y")
                               : (is_z_coupled ? &adCoupledGradient("disp_z") : &adGradZero()),
                  _coord_system != Moose::COORD_RZ && is_z_coupled ? &adCoupledGradient("disp_z")
                                                                   : &adGradZero()};
  }

  // no coupling
  else
  {
    _is_coupled = false;
    // TODO: We should deprecate this case and have the user use a GenericConstantMaterial for this
  }
}

template <ComputeStage compute_stage>
void
ADDensity<compute_stage>::initQpStatefulProperties()
{
  _density[_qp] = _orig_density;
}

template <>
void
ADDensity<RESIDUAL>::computeQpProperties()
{
  if (_is_coupled)
  {
    // rho * V = rho0 * V0
    // rho = rho0 * V0 / V
    // rho = rho0 / det(F)
    // rho = rho0 / det(grad(u) + 1)

    const auto Axx = (*_grad_disp[0])[_qp](0) + 1.0;
    const auto Axy = (*_grad_disp[0])[_qp](1);
    const auto Axz = (*_grad_disp[0])[_qp](2);
    const auto Ayx = (*_grad_disp[1])[_qp](0);
    auto Ayy = (*_grad_disp[1])[_qp](1) + 1.0;
    const auto Ayz = (*_grad_disp[1])[_qp](2);
    const auto Azx = (*_grad_disp[2])[_qp](0);
    const auto Azy = (*_grad_disp[2])[_qp](1);
    auto Azz = (*_grad_disp[2])[_qp](2) + 1.0;

    switch (_coord_system)
    {
      case Moose::COORD_XYZ:
        Azz = (*_grad_disp[2])[_qp](2) + 1.0;
        break;

      case Moose::COORD_RZ:
        if (_q_point[_qp](0) != 0.0)
          Azz = _disp_r[_qp] / _q_point[_qp](0) + 1.0;
        break;

      case Moose::COORD_RSPHERICAL:
        if (_q_point[_qp](0) != 0.0)
          Ayy = Azz = _disp_r[_qp] / _q_point[_qp](0) + 1.0;
        break;
    }

    const auto detF = Axx * Ayy * Azz + Axy * Ayz * Azx + Axz * Ayx * Azy - Azx * Ayy * Axz -
                      Azy * Ayz * Axx - Azz * Ayx * Axy;
    _density[_qp] = _orig_density / detF;
  }
  else
    _density[_qp] = _orig_density;
}

template <>
void
ADDensity<JACOBIAN>::computeQpProperties()
{
}
