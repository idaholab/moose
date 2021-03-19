//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVMixingLengthReynoldsStress.h"
#include "INSFVVelocityVariable.h"

registerMooseObject("NavierStokesApp", INSFVMixingLengthReynoldsStress);

InputParameters
INSFVMixingLengthReynoldsStress::validParams()
{
  InputParameters params = FVFluxKernel::validParams();
  params.addClassDescription(
      "Computes the force due to the Reynolds stress term in the incompressible"
      " Reynolds-averaged Navier-Stokes equations.");
  params.addRequiredCoupledVar("u", "The velocity in the x direction.");
  params.addCoupledVar("v", "The velocity in the y direction.");
  params.addCoupledVar("w", "The velocity in the z direction.");
  params.addParam<Real>("rho", "fluid density");
  params.addRequiredCoupledVar("mixing_length", "Turbulent eddy mixing length.");
  MooseEnum momentum_component("x=0 y=1 z=2", "x");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this kernel applies to.");
  params.set<unsigned short>("ghost_layers") = 2;
  return params;
}

INSFVMixingLengthReynoldsStress::INSFVMixingLengthReynoldsStress(const InputParameters & params)
  : FVFluxKernel(params),
    _dim(_subproblem.mesh().dimension()),
    _axis_index(getParam<MooseEnum>("momentum_component")),
    _u_var(dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("u", 0))),
    _v_var(params.isParamValid("v")
               ? dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("v", 0))
               : nullptr),
    _w_var(params.isParamValid("w")
               ? dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("w", 0))
               : nullptr),
    _rho(getParam<Real>("rho")),
    _mixing_len(coupledValue("mixing_length")),
    _mixing_len_neighbor(coupledNeighborValue("mixing_length"))
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("INSFV is not supported by local AD indexing. In order to use INSFV, please run the "
             "configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#endif

  if (!_u_var)
    paramError("u", "the u velocity must be an INSFVVelocityVariable.");

  if (_dim >= 2 && !_v_var)
    paramError("v",
               "In two or more dimensions, the v velocity must be supplied and it must be an "
               "INSFVVelocityVariable.");

  if (_dim >= 3 && !_w_var)
    paramError("w",
               "In three-dimensions, the w velocity must be supplied and it must be an "
               "INSFVVelocityVariable.");
}

ADReal
INSFVMixingLengthReynoldsStress::computeQpResidual()
{
#ifdef MOOSE_GLOBAL_AD_INDEXING
  constexpr Real offset = 1e-15; // prevents explosion of sqrt(x) derivative to infinity

  // Compute the normalized velocity gradient.
  const auto & grad_u = _u_var->adGradSln(*_face_info);
  ADReal velocity_gradient = grad_u(0) * grad_u(0);
  if (_dim >= 2)
  {
    auto grad_v = _v_var->adGradSln(*_face_info);
    velocity_gradient += grad_u(1) * grad_u(1) + grad_v(0) * grad_v(0) + grad_v(1) * grad_v(1);
    if (_dim >= 3)
    {
      auto grad_w = _w_var->adGradSln(*_face_info);
      velocity_gradient += grad_u(2) * grad_u(2) + grad_v(2) * grad_v(2) + grad_w(0) * grad_w(0) +
                           grad_w(1) * grad_w(1) + grad_w(2) * grad_w(2);
    }
  }
  velocity_gradient = std::sqrt(velocity_gradient + offset);

  // Interpolate the mixing length to the face
  ADReal mixing_len;
  interpolate(Moose::FV::InterpMethod::Average,
              mixing_len,
              _mixing_len[_qp],
              _mixing_len_neighbor[_qp],
              *_face_info,
              true);

  // Compute the eddy diffusivitiy
  ADReal eddy_diff = velocity_gradient * mixing_len * mixing_len;

  // Compute the dot product of the strain rate tensor and the normal vector
  // aka (grad_v + grad_v^T) * n_hat
  ADReal norm_strain_rate = gradUDotNormal();
  norm_strain_rate += _u_var->adGradSln(*_face_info)(_axis_index) * _normal(0);
  norm_strain_rate += _dim >= 2 ? _v_var->adGradSln(*_face_info)(_axis_index) * _normal(1) : 0;
  norm_strain_rate += _dim >= 3 ? _w_var->adGradSln(*_face_info)(_axis_index) * _normal(2) : 0;

  // Return the turbulent stress contribution to the momentum equation
  return -1 * _rho * eddy_diff * norm_strain_rate;

#else
  return 0;

#endif
}
