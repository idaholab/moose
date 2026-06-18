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
  NS::addLinearFVVelocityVariableParams(params);
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
    _velocity_vars(NS::getLinearFVVelocityVariables(*this, _fv_problem, _tid, _dim)),
    _density(getFunctor<Real>(NS::density)),
    _face_flux(getFunctor<Real>("face_flux")),
    _gravity(getParam<RealVectorValue>("gravity")),
    _reference_pressure_point(getParam<Point>("reference_pressure_point")),
    _use_normal_velocity_only(getParam<bool>("use_normal_velocity_only"))
{
}

const ElemInfo &
LinearFVPrghTotalPressureBC::fluidElemInfo() const
{
  return NS::linearFVBoundaryElemInfo(*_current_face_info, _current_face_type);
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
  return NS::linearFVBoundaryNormalMultiplier(_current_face_type) *
         _face_flux(functorFaceArg(_face_flux, *_current_face_info), state);
}

Real
LinearFVPrghTotalPressureBC::dynamicPressureCorrection() const
{
  if (!isBackflow())
    return 0.0;

  const auto & elem_info = fluidElemInfo();
  const auto state = determineState();
  const RealVectorValue velocity = NS::linearFVCellVelocity(_velocity_vars, _dim, elem_info, state);
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
