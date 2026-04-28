//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVPressureFluxBC.h"
#include "FaceCenteredMapFunctor.h"

registerMooseObject("NavierStokesApp", LinearFVPressureFluxBC);

InputParameters
LinearFVPressureFluxBC::validParams()
{
  InputParameters params = LinearFVAdvectionDiffusionBC::validParams();
  params.addClassDescription(
      "Adds a fixed diffusive flux BC which can be used for the assembly of linear "
      "finite volume system and whose normal face gradient values are determined "
      "using the pressure-predictor face flux. This kernel is only designed to work with "
      "advection-diffusion problems.");
  params.addParam<MooseFunctorName>(
      "pressure_predictor_flux",
      "",
      "Optional total pressure-predictor source flux. When provided this supersedes the split "
      "HbyA_flux + additional_face_fluxes input path.");
  params.addParam<MooseFunctorName>(
      "constrained_pressure_normal_gradient",
      "",
      "Optional cached pressure normal gradient populated from the current Rhie-Chow predictor "
      "state. When provided this supersedes the algebraic wall-flux reconstruction.");
  params.addParam<bool>(
      "use_constrained_pressure_normal_gradient_only",
      false,
      "When true, require and use constrained_pressure_normal_gradient as the authoritative "
      "patch constraint instead of reconstructing the pressure boundary gradient algebraically.");
  params.addRequiredParam<MooseFunctorName>("HbyA_flux", "The total HbyA face flux value.");
  params.addParam<std::vector<MooseFunctorName>>(
      "additional_face_fluxes",
      {},
      "Additional pressure-equation source-flux functors that must be enforced together with "
      "the HbyA wall flux.");
  params.addRequiredParam<MooseFunctorName>(
      "Ainv", "The 1/A where A is the momentum system diagonal vector.");
  return params;
}

LinearFVPressureFluxBC::LinearFVPressureFluxBC(const InputParameters & parameters)
  : LinearFVAdvectionDiffusionBC(parameters),
    _use_constrained_pressure_normal_gradient_only(
        getParam<bool>("use_constrained_pressure_normal_gradient_only")),
    _pressure_predictor_flux(
        getParam<MooseFunctorName>("pressure_predictor_flux").empty()
            ? nullptr
            : &getFunctor<Real>(getParam<MooseFunctorName>("pressure_predictor_flux"))),
    _constrained_pressure_normal_gradient(
        getParam<MooseFunctorName>("constrained_pressure_normal_gradient").empty()
            ? nullptr
            : &getFunctor<Real>(getParam<MooseFunctorName>("constrained_pressure_normal_gradient"))),
    _HbyA_flux(getFunctor<Real>("HbyA_flux")),
    _Ainv(getFunctor<RealVectorValue>("Ainv"))
{
  if (_use_constrained_pressure_normal_gradient_only && !_constrained_pressure_normal_gradient)
    paramError("use_constrained_pressure_normal_gradient_only",
               "requires constrained_pressure_normal_gradient to be provided.");

  for (const auto & flux_name : getParam<std::vector<MooseFunctorName>>("additional_face_fluxes"))
    _additional_face_fluxes.push_back(&getFunctor<Real>(flux_name));
}

Real
LinearFVPressureFluxBC::computeBoundaryNormalAinv() const
{
  const auto face_arg = makeCDFace(*_current_face_info);
  const auto normal = _current_face_info->normal();

  Real normal_ainv = 0.0;
  for (const auto i : make_range(LIBMESH_DIM))
    normal_ainv += _Ainv(face_arg, determineState())(i) * normal(i) * normal(i);

  return normal_ainv;
}

Real
LinearFVPressureFluxBC::computeBoundaryPressureSourceFlux() const
{
  refreshBoundaryConstraintCache();
  return _cached_boundary_pressure_source_flux;
}

void
LinearFVPressureFluxBC::refreshBoundaryConstraintCache() const
{
  const auto state = determineState();
  if (_boundary_constraint_cache_valid && _cached_face_info == _current_face_info &&
      _cached_face_type == _current_face_type && _cached_state == state.state &&
      _cached_iteration_type == state.iteration_type)
    return;

  const auto face_arg = singleSidedFaceArg(_current_face_info);
  _cached_face_info = _current_face_info;
  _cached_face_type = _current_face_type;
  _cached_state = state.state;
  _cached_iteration_type = state.iteration_type;
  _cached_boundary_normal_ainv = computeBoundaryNormalAinv();
  _cached_constrained_pressure_normal_gradient =
      _constrained_pressure_normal_gradient
          ? (*_constrained_pressure_normal_gradient)(face_arg, state)
          : 0.0;

  if (_pressure_predictor_flux)
    _cached_boundary_pressure_source_flux = (*_pressure_predictor_flux)(face_arg, state);
  else
  {
    Real total_flux = _HbyA_flux(face_arg, state);
    for (const auto * flux : _additional_face_fluxes)
    {
      if (const auto * face_map_flux =
              dynamic_cast<
                  const FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> *>(
                  flux);
          face_map_flux && !face_map_flux->count(_current_face_info->id()))
        continue;

      total_flux += (*flux)(face_arg, state);
    }

    _cached_boundary_pressure_source_flux = total_flux;
  }

  _boundary_constraint_cache_valid = true;
}

Real
LinearFVPressureFluxBC::computeBoundaryValue() const
{
  refreshBoundaryConstraintCache();
  const auto elem_info = _current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                             ? _current_face_info->elemInfo()
                             : _current_face_info->neighborInfo();
  const Real distance = computeCellToFaceDistance();
  if (_constrained_pressure_normal_gradient)
    return _var.getElemValue(*elem_info, determineState()) +
           _cached_constrained_pressure_normal_gradient * distance;

  return _var.getElemValue(*elem_info, determineState()) -
         _cached_boundary_pressure_source_flux / std::max(_cached_boundary_normal_ainv, 1e-8) *
             distance;
}

Real
LinearFVPressureFluxBC::computeBoundaryNormalGradient() const
{
  refreshBoundaryConstraintCache();
  if (_constrained_pressure_normal_gradient)
    return _cached_constrained_pressure_normal_gradient;

  return -_cached_boundary_pressure_source_flux / std::max(_cached_boundary_normal_ainv, 1e-8);
}

Real
LinearFVPressureFluxBC::computeBoundaryValueMatrixContribution() const
{
  return 1.0;
}

Real
LinearFVPressureFluxBC::computeBoundaryValueRHSContribution() const
{
  refreshBoundaryConstraintCache();
  const Real distance = computeCellToFaceDistance();
  if (_constrained_pressure_normal_gradient)
    return _cached_constrained_pressure_normal_gradient * distance;

  return -_cached_boundary_pressure_source_flux / std::max(_cached_boundary_normal_ainv, 1e-8) *
         distance;
}

Real
LinearFVPressureFluxBC::computeBoundaryGradientMatrixContribution() const
{
  return 0.0;
}

Real
LinearFVPressureFluxBC::computeBoundaryGradientRHSContribution() const
{
  refreshBoundaryConstraintCache();
  if (_constrained_pressure_normal_gradient)
    return _cached_constrained_pressure_normal_gradient * _cached_boundary_normal_ainv;

  return -_cached_boundary_pressure_source_flux;
}
