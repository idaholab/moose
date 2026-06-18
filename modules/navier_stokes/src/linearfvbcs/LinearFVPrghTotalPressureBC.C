//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVPrghTotalPressureBC.h"

#include "ElemInfo.h"
#include "FEProblemBase.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", LinearFVPrghTotalPressureBC);

InputParameters
LinearFVPrghTotalPressureBC::validParams()
{
  InputParameters params = LinearFVAdvectionDiffusionFunctorDirichletBC::validParams();
  params.addClassDescription(
      "Adds a total-pressure fixed-value boundary condition for a linear FV p_rgh solve. This "
      "imposes the supplied static/total pressure reference with the p_rgh hydrostatic offset; on "
      "backflow it also subtracts the incoming dynamic pressure.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "The density functor.");
  params.addParam<MooseFunctorName>(
      "face_flux",
      "corrected_face_phi",
      "The corrected face-flux functor used for the backflow dynamic-pressure switch.");
  params.addRequiredParam<SolverVariableName>("u", "The velocity in the x direction.");
  params.addParam<SolverVariableName>("v", "The velocity in the y direction.");
  params.addParam<SolverVariableName>("w", "The velocity in the z direction.");
  params.addRequiredParam<RealVectorValue>("gravity", "The gravitational acceleration vector.");
  params.addParam<Point>("reference_pressure_point",
                         Point(0.0, 0.0, 0.0),
                         "The point used to form gh for the p_rgh hydrostatic offset.");
  params.addParam<bool>(
      "use_normal_velocity_only",
      true,
      "Use only the extrapolated normal velocity in the dynamic-pressure correction.");
  return params;
}

LinearFVPrghTotalPressureBC::LinearFVPrghTotalPressureBC(const InputParameters & parameters)
  : LinearFVAdvectionDiffusionFunctorDirichletBC(parameters),
    _dim(_subproblem.mesh().dimension()),
    _u_var(dynamic_cast<const MooseLinearVariableFVReal *>(
        &_fv_problem.getVariable(_tid, getParam<SolverVariableName>("u")))),
    _v_var(parameters.isParamValid("v")
               ? dynamic_cast<const MooseLinearVariableFVReal *>(
                     &_fv_problem.getVariable(_tid, getParam<SolverVariableName>("v")))
               : nullptr),
    _w_var(parameters.isParamValid("w")
               ? dynamic_cast<const MooseLinearVariableFVReal *>(
                     &_fv_problem.getVariable(_tid, getParam<SolverVariableName>("w")))
               : nullptr),
    _density(getFunctor<Real>(NS::density)),
    _face_flux(getFunctor<Real>("face_flux")),
    _gravity(getParam<RealVectorValue>("gravity")),
    _reference_pressure_point(getParam<Point>("reference_pressure_point")),
    _use_normal_velocity_only(getParam<bool>("use_normal_velocity_only"))
{
  if (!_u_var)
    paramError("u", "the u velocity must be a MooseLinearVariableFVReal.");

  _velocity_vars.push_back(_u_var);

  if (_dim >= 2 && !_v_var)
    paramError("v",
               "In two or more dimensions, the v velocity must be supplied and it must be a "
               "MooseLinearVariableFVReal.");
  _velocity_vars.push_back(_v_var);

  if (_dim >= 3 && !_w_var)
    paramError("w",
               "In three dimensions, the w velocity must be supplied and it must be a "
               "MooseLinearVariableFVReal.");
  _velocity_vars.push_back(_w_var);
}

const ElemInfo &
LinearFVPrghTotalPressureBC::fluidElemInfo() const
{
  return _current_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR
             ? *_current_face_info->neighborInfo()
             : *_current_face_info->elemInfo();
}

RealVectorValue
LinearFVPrghTotalPressureBC::cellVelocity(const ElemInfo & elem_info,
                                          const Moose::StateArg & state) const
{
  RealVectorValue velocity;
  for (const auto dim_i : make_range(_dim))
    velocity(dim_i) = _velocity_vars[dim_i]->getElemValue(elem_info, state);

  return velocity;
}

bool
LinearFVPrghTotalPressureBC::isBackflow() const
{
  return outwardFaceFlux() < 0.0;
}

Real
LinearFVPrghTotalPressureBC::outwardFaceFlux() const
{
  const auto state = determineState();
  const Real boundary_normal_multiplier =
      _current_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR ? -1.0 : 1.0;
  return boundary_normal_multiplier *
         _face_flux(functorFaceArg(_face_flux, *_current_face_info), state);
}

Real
LinearFVPrghTotalPressureBC::dynamicPressureCorrection() const
{
  if (!isBackflow())
    return 0.0;

  const auto & elem_info = fluidElemInfo();
  const auto state = determineState();
  const RealVectorValue velocity = cellVelocity(elem_info, state);
  Real speed_squared = velocity.norm_sq();

  if (_use_normal_velocity_only)
  {
    const RealVectorValue normal = _current_face_info->normal();
    const Real normal_magnitude = normal.norm();
    if (normal_magnitude <= libMesh::TOLERANCE)
      speed_squared = 0.0;
    else
    {
      const Real normal_speed = velocity * (normal / normal_magnitude);
      speed_squared = normal_speed * normal_speed;
    }
  }

  const Real rho = _density(functorFaceArg(_density, *_current_face_info), state);
  return 0.5 * rho * speed_squared;
}

Real
LinearFVPrghTotalPressureBC::hydrostaticPressureOffset() const
{
  const auto state = determineState();
  const Real rho = _density(functorFaceArg(_density, *_current_face_info), state);
  const Real gh = _gravity * (_current_face_info->faceCentroid() - _reference_pressure_point);
  return rho * gh;
}

Real
LinearFVPrghTotalPressureBC::computeBoundaryValue() const
{
  const auto state = determineState();
  const Real reference_value = _functor(functorFaceArg(_functor, *_current_face_info), state);
  return reference_value - dynamicPressureCorrection() - hydrostaticPressureOffset();
}
