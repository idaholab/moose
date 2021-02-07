//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVMixingLengthScalarDiffusion.h"

registerMooseObject("NavierStokesApp", INSFVMixingLengthScalarDiffusion);

InputParameters
INSFVMixingLengthScalarDiffusion::validParams()
{
  InputParameters params = FVFluxKernel::validParams();
  params.addRequiredCoupledVar("u", "The velocity in the x direction.");
  params.addCoupledVar("v", "The velocity in the y direction.");
  params.addCoupledVar("w", "The velocity in the z direction.");
  params.addRequiredCoupledVar("mixing_length", "The turbulent mixing length.");
  params.set<unsigned short>("ghost_layers") = 2;
  return params;
}

INSFVMixingLengthScalarDiffusion::INSFVMixingLengthScalarDiffusion(const InputParameters & params)
  : FVFluxKernel(params),
    _dim(_subproblem.mesh().dimension()),
    _u_var(dynamic_cast<const INSFVVelocityVariable *>(
        &_subproblem.getVariable(_tid, params.get<std::vector<VariableName>>("u").front()))),
    _v_var(params.isParamValid("v")
               ? dynamic_cast<const INSFVVelocityVariable *>(&_subproblem.getVariable(
                     _tid, params.get<std::vector<VariableName>>("v").front()))
               : nullptr),
    _w_var(params.isParamValid("w")
               ? dynamic_cast<const INSFVVelocityVariable *>(&_subproblem.getVariable(
                     _tid, params.get<std::vector<VariableName>>("w").front()))
               : nullptr),
    _mixing_len(coupledValue("mixing_length"))
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
INSFVMixingLengthScalarDiffusion::computeQpResidual()
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

  // Compute the eddy diffusivitiy
  Real mixing_len = _mixing_len[_qp];
  ADReal eddy_diff = velocity_gradient * mixing_len * mixing_len;

  // Compute the diffusive flux of the scalar variable
  auto dudn = gradUDotNormal();
  return -1 * eddy_diff * dudn;

#else
  return 0;

#endif
}
