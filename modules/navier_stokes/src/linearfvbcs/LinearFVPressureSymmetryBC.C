//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVPressureSymmetryBC.h"
#include "FaceCenteredMapFunctor.h"

registerMooseObject("NavierStokesApp", LinearFVPressureSymmetryBC);

InputParameters
LinearFVPressureSymmetryBC::validParams()
{
  InputParameters params = LinearFVAdvectionDiffusionBC::validParams();
  params.addClassDescription("Adds a symmetry boundary condition for pressure in a segregated "
                             "velocity and pressure solve.");
  params.addParam<MooseFunctorName>(
      "pressure_predictor_flux",
      "",
      "Optional total pressure-predictor source flux. When provided this supersedes the split "
      "HbyA_flux + additional_face_fluxes input path.");
  params.addParam<MooseFunctorName>(
      "constrained_pressure_normal_gradient",
      "",
      "Optional cached pressure normal gradient populated from the current Rhie-Chow predictor "
      "state. When provided this supersedes the algebraic symmetry-flux reconstruction.");
  params.addParam<bool>(
      "use_constrained_pressure_normal_gradient_only",
      false,
      "When true, require and use constrained_pressure_normal_gradient as the authoritative "
      "patch constraint instead of reconstructing the pressure boundary gradient algebraically.");
  params.addRequiredParam<MooseFunctorName>("HbyA_flux", "The total HbyA face flux value.");
  params.addParam<MooseFunctorName>(
      "Ainv",
      "",
      "Optional 1/A tensor serving as a diffusion coefficient. Required when a constrained "
      "pressure normal gradient is supplied.");
  params.addParam<std::vector<MooseFunctorName>>(
      "additional_face_fluxes",
      {},
      "Additional pressure-equation source-flux functors that should be canceled at the "
      "boundary together with HbyA.");
  return params;
}

LinearFVPressureSymmetryBC::LinearFVPressureSymmetryBC(const InputParameters & parameters)
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
    _HbyA_flux(getFunctor<Real>("HbyA_flux"))
{
  _Ainv = getParam<MooseFunctorName>("Ainv").empty()
              ? nullptr
              : &getFunctor<RealVectorValue>(getParam<MooseFunctorName>("Ainv"));
  if ((_constrained_pressure_normal_gradient || _use_constrained_pressure_normal_gradient_only) &&
      !_Ainv)
    paramError("Ainv",
               "A constrained_pressure_normal_gradient requires the Ainv functor so the "
               "pressure flux boundary contribution can be assembled consistently.");
  if (_use_constrained_pressure_normal_gradient_only && !_constrained_pressure_normal_gradient)
    paramError("use_constrained_pressure_normal_gradient_only",
               "requires constrained_pressure_normal_gradient to be provided.");

  for (const auto & flux_name : getParam<std::vector<MooseFunctorName>>("additional_face_fluxes"))
    _additional_face_fluxes.push_back(&getFunctor<Real>(flux_name));
}

Real
LinearFVPressureSymmetryBC::computeBoundaryNormalAinv() const
{
  mooseAssert(_Ainv, "Ainv must be available when evaluating a constrained pressure gradient.");

  const auto face_arg = makeCDFace(*_current_face_info);
  const auto normal = _current_face_info->normal();

  Real normal_ainv = 0.0;
  for (const auto i : make_range(LIBMESH_DIM))
    normal_ainv += (*_Ainv)(face_arg, determineState())(i) * normal(i) * normal(i);

  return normal_ainv;
}

Real
LinearFVPressureSymmetryBC::computeBoundaryValue() const
{
  refreshBoundaryConstraintCache();
  const auto elem_info = _current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                             ? _current_face_info->elemInfo()
                             : _current_face_info->neighborInfo();
  if (_constrained_pressure_normal_gradient)
    return _var.getElemValue(*elem_info, determineState()) +
           _cached_constrained_pressure_normal_gradient * computeCellToFaceDistance();

  return _var.getElemValue(*elem_info, determineState());
}

Real
LinearFVPressureSymmetryBC::computeBoundaryNormalGradient() const
{
  refreshBoundaryConstraintCache();
  if (_constrained_pressure_normal_gradient)
    return _cached_constrained_pressure_normal_gradient;

  return 0.0;
}

Real
LinearFVPressureSymmetryBC::computeBoundaryValueMatrixContribution() const
{
  return 1.0;
}

Real
LinearFVPressureSymmetryBC::computeBoundaryValueRHSContribution() const
{
  refreshBoundaryConstraintCache();
  if (_constrained_pressure_normal_gradient)
    return _cached_constrained_pressure_normal_gradient * computeCellToFaceDistance();

  return 0.0;
}

Real
LinearFVPressureSymmetryBC::computeBoundaryGradientMatrixContribution() const
{
  return 0.0;
}

Real
LinearFVPressureSymmetryBC::computeBoundaryGradientRHSContribution() const
{
  refreshBoundaryConstraintCache();
  if (_constrained_pressure_normal_gradient)
    return _cached_constrained_pressure_normal_gradient * _cached_boundary_normal_ainv;

  return -_cached_boundary_pressure_source_flux;
}

void
LinearFVPressureSymmetryBC::refreshBoundaryConstraintCache() const
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
  _cached_boundary_normal_ainv = _Ainv ? computeBoundaryNormalAinv() : 0.0;
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
