//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVMomentumViscousSourceRZ.h"

#include "NS.h"
#include "SystemBase.h"

registerMooseObject("NavierStokesApp", INSFVMomentumViscousSourceRZ);

InputParameters
INSFVMomentumViscousSourceRZ::validParams()
{
  InputParameters params = INSFVElementalKernel::validParams();
  params.addClassDescription(
      "Adds the -\\mu u_r / r^2 viscous source term that appears in the cylindrical form of the "
      "Navier-Stokes momentum equations (axisymmetric, no swirl).");
  params.addRequiredParam<MooseFunctorName>(
      NS::mu, "The dynamic viscosity that multiplies the viscous source term.");
  return params;
}

INSFVMomentumViscousSourceRZ::INSFVMomentumViscousSourceRZ(const InputParameters & params)
  : INSFVElementalKernel(params),
    _mu(getFunctor<ADReal>(NS::mu)),
    _coord_system(getBlockCoordSystem()),
    _rz_radial_coord(_subproblem.getAxisymmetricRadialCoord())
{
  if (_coord_system != Moose::COORD_RZ)
    mooseError(name(), " is only valid on blocks that use an RZ coordinate system.");

  if (_index != _rz_radial_coord)
    paramError("momentum_component",
               "INSFVMomentumViscousSourceRZ only applies to the radial momentum component in an "
               "axisymmetric coordinate system.");
}

ADReal
INSFVMomentumViscousSourceRZ::computeCoefficient(const Moose::ElemArg & elem_arg,
                                                 const Moose::StateArg & state) const
{
  const auto radius = _q_point[_qp](_rz_radial_coord);
  mooseAssert(radius > 0.0, "Axisymmetric radial coordinate should be positive inside a cell.");
  return -_mu(elem_arg, state) / (radius * radius);
}

ADReal
INSFVMomentumViscousSourceRZ::computeSegregatedContribution()
{
  const auto elem_arg = makeElemArg(_current_elem);
  const auto state = determineState();
  return computeCoefficient(elem_arg, state) * _u_functor(elem_arg, state);
}

void
INSFVMomentumViscousSourceRZ::gatherRCData(const Elem & elem)
{
  const auto elem_arg = makeElemArg(&elem);
  const auto state = determineState();
  const auto coefficient = computeCoefficient(elem_arg, state);
  const auto volume_coefficient = coefficient * _assembly.elementVolume(&elem);
  _rc_uo.addToA(&elem, _index, volume_coefficient);

  const auto dof_number = elem.dof_number(_sys.number(), _var.number(), 0);
  addResidualAndJacobian(volume_coefficient * _u_functor(elem_arg, state), dof_number);
}
