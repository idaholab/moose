//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVRZViscousSource.h"

#include "MooseLinearVariableFV.h"
#include "NS.h"

#include "libmesh/utility.h"

registerMooseObject("NavierStokesApp", LinearFVRZViscousSource);

InputParameters
LinearFVRZViscousSource::validParams()
{
  InputParameters params = LinearFVElementalKernel::validParams();
  params.addClassDescription(
      "Adds the axisymmetric viscous source term mu * u_r / r^2 to the FV momentum equations.");
  params.addRequiredParam<MooseFunctorName>(NS::mu, "Dynamic viscosity functor.");
  MooseEnum component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "momentum_component", component, "Momentum component this kernel contributes to.");
  params.addParam<SolverVariableName>("u", "The velocity in the x direction.");
  params.addParam<SolverVariableName>("v", "The velocity in the y direction.");
  params.addParam<SolverVariableName>("w", "The velocity in the z direction.");
  params.addParam<bool>(
      "use_deviatoric_terms",
      false,
      "Include the deviatoric correction (-2/3 div(u)) in the axisymmetric term.");
  return params;
}

LinearFVRZViscousSource::LinearFVRZViscousSource(const InputParameters & params)
  : LinearFVElementalKernel(params),
    _mu(getFunctor<Real>(NS::mu)),
    _component(getParam<MooseEnum>("momentum_component")),
    _rz_radial_coord(_subproblem.getAxisymmetricRadialCoord()),
    _dim(_subproblem.mesh().dimension()),
    _use_deviatoric_terms(getParam<bool>("use_deviatoric_terms")),
    _coord_type(getBlockCoordSystem()),
    _stress_multiplier(_use_deviatoric_terms ? 2.0 : 1.0),
    _velocity_vars{nullptr, nullptr, nullptr}
{
  if (_coord_type != Moose::CoordinateSystemType::COORD_RZ)
    paramError("block", "LinearFVRZViscousSource is only valid on RZ coordinate systems.");

  if (_component != _rz_radial_coord)
    paramError("momentum_component", "LinearFVRZViscousSource must act on the radial component.");

  if (_use_deviatoric_terms)
    _var.computeCellGradients();

  const auto get_velocity_var =
      [this](const std::string & param_name) -> const MooseLinearVariableFVReal *
  {
    auto & var = _fe_problem.getVariable(_tid, getParam<SolverVariableName>(param_name));
    auto ptr = dynamic_cast<const MooseLinearVariableFVReal *>(&var);
    if (!ptr)
      paramError(param_name, "The supplied variable must be a MooseLinearVariableFVReal.");
    return ptr;
  };

  if (isParamValid("u"))
    _velocity_vars[0] = get_velocity_var("u");
  if (isParamValid("v"))
    _velocity_vars[1] = get_velocity_var("v");
  if (isParamValid("w"))
    _velocity_vars[2] = get_velocity_var("w");

  if (_use_deviatoric_terms)
  {
    if (!_velocity_vars[0])
      paramError("u", "The x-velocity must be provided when using deviatoric terms.");
    if (_dim > 1 && !_velocity_vars[1])
      paramError("v", "The y-velocity must be provided when using deviatoric terms.");
    if (_dim > 2 && !_velocity_vars[2])
      paramError("w", "The z-velocity must be provided when using deviatoric terms.");

    for (const auto dir : make_range(_dim))
      const_cast<MooseLinearVariableFVReal *>(_velocity_vars[dir])->computeCellGradients();
  }
}

Real
LinearFVRZViscousSource::computeMatrixContribution()
{
  if (_coord_type != Moose::CoordinateSystemType::COORD_RZ)
    return 0.0;

  const Real r = _current_elem_info->centroid()(_rz_radial_coord);
  mooseAssert(r > 0, "Axisymmetric control volumes should not sit on the axis (r = 0).");

  const auto elem_arg = makeElemArg(_current_elem_info->elem());
  const auto state = determineState();

  const Real mu = _mu(elem_arg, state);
  return mu * _stress_multiplier * _current_elem_volume / (r * r);
}

Real
LinearFVRZViscousSource::computeRightHandSideContribution()
{
  if (_coord_type != Moose::CoordinateSystemType::COORD_RZ || !_use_deviatoric_terms)
    return 0.0;

  Real divergence = 0.0;
  const auto elem_info = _current_elem_info;

  for (const auto dir : make_range(_dim))
    divergence += velocityVar(dir).gradSln(*elem_info)(dir);

  const Real r = elem_info->centroid()(_rz_radial_coord);
  mooseAssert(r > 0, "Axisymmetric control volumes should not sit on the axis (r = 0).");

  const auto state = determineState();
  const Real radial_value = velocityVar(_rz_radial_coord).getElemValue(*elem_info, state);
  divergence += radial_value / r;

  const auto elem_arg = makeElemArg(elem_info->elem());
  const Real mu = _mu(elem_arg, state);

  return (2.0 / 3.0) * mu * divergence * _current_elem_volume / r;
}

const MooseLinearVariableFVReal &
LinearFVRZViscousSource::velocityVar(unsigned int dir) const
{
  mooseAssert(dir < _velocity_vars.size() && _velocity_vars[dir],
              "Velocity variable for requested direction is not available.");
  return *_velocity_vars[dir];
}
