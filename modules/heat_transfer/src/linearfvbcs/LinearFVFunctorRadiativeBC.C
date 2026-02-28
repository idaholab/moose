//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVFunctorRadiativeBC.h"
#include "MathUtils.h"

registerMooseObject("HeatTransferApp", LinearFVFunctorRadiativeBC);

InputParameters
LinearFVFunctorRadiativeBC::validParams()
{
  InputParameters params = LinearFVAdvectionDiffusionFunctorRobinBCBase::validParams();
  params.addClassDescription(
      "Boundary condition for radiative heat flux in a linear finite volume system. "
      "The nonlinear radiative flux q = sigma * emissivity * (T^4 - Tinfinity^4) is "
      "Newton-linearized around the extrapolated boundary face temperature at each assembly "
      "step, yielding a Robin-type condition with second-order spatial accuracy. "
      "Convergence to the nonlinear solution is achieved through Picard iteration driven "
      "by transient stepping or a coupled nonlinear solve.");
  params.addRequiredParam<MooseFunctorName>(
      "emissivity", "Functor describing the surface emissivity for the radiative boundary condition");
  params.addRequiredParam<MooseFunctorName>(
      "Tinfinity", "Functor for the far-field temperature of the body in radiative heat transfer");
  params.addParam<Real>(
      "stefan_boltzmann_constant", 5.670374419e-8, "The Stefan-Boltzmann constant");
  params.addRequiredParam<MooseFunctorName>(
      "diffusion_coeff",
      "Functor for the thermal conductivity. Must match the diffusion_coeff used in "
      "LinearFVDiffusion, as it serves as the alpha coefficient in the Robin formulation.");
  return params;
}

LinearFVFunctorRadiativeBC::LinearFVFunctorRadiativeBC(const InputParameters & parameters)
  : LinearFVAdvectionDiffusionFunctorRobinBCBase(parameters),
    _emissivity(getFunctor<Real>("emissivity")),
    _tinf(getFunctor<Real>("Tinfinity")),
    _sigma(getParam<Real>("stefan_boltzmann_constant")),
    _diffusion_coeff(getFunctor<Real>("diffusion_coeff"))
{
  _var.computeCellGradients();
}

Real
LinearFVFunctorRadiativeBC::extrapolateFaceTemperature(Moose::StateArg state) const
{
  const auto & elem_info = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM)
                               ? _current_face_info->elemInfo()
                               : _current_face_info->neighborInfo();
  return _var.getElemValue(*elem_info, state) +
         _var.gradSln(*elem_info) * computeCellToFaceVector();
}

Real
LinearFVFunctorRadiativeBC::getAlpha(Moose::FaceArg face, Moose::StateArg state) const
{
  return _diffusion_coeff(face, state);
}

Real
LinearFVFunctorRadiativeBC::getBeta(Moose::FaceArg face, Moose::StateArg state) const
{
  const Real T_b_old = extrapolateFaceTemperature(state);
  return 4.0 * _sigma * _emissivity(face, state) * Utility::pow<3>(T_b_old);
}

Real
LinearFVFunctorRadiativeBC::getGamma(Moose::FaceArg face, Moose::StateArg state) const
{
  const Real T_b_old = extrapolateFaceTemperature(state);
  return _sigma * _emissivity(face, state) *
         (3.0 * Utility::pow<4>(T_b_old) + Utility::pow<4>(_tinf(face, state)));
}
