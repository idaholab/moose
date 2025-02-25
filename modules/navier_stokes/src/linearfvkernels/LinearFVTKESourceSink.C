//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVTKESourceSink.h"
#include "Assembly.h"
#include "SubProblem.h"
#include "NavierStokesMethods.h"

registerMooseObject("MooseApp", LinearFVTKESourceSink);

InputParameters
LinearFVTKESourceSink::validParams()
{
  InputParameters params = LinearFVElementalKernel::validParams();
  params.addClassDescription("Elemental kernel to compute the production and destruction "
                             " terms of turbulent kinetic energy (TKE).");
  params.addRequiredParam<MooseFunctorName>("u", "The velocity in the x direction.");
  params.addParam<MooseFunctorName>("v", "The velocity in the y direction.");
  params.addParam<MooseFunctorName>("w", "The velocity in the z direction.");
  params.addRequiredParam<MooseFunctorName>(NS::TKED,
                                            "Coupled turbulent kinetic energy dissipation rate.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "Fluid density");
  params.addRequiredParam<MooseFunctorName>(NS::mu, "Dynamic viscosity.");
  params.addRequiredParam<MooseFunctorName>(NS::mu_t, "Turbulent viscosity.");

  params.addParam<std::vector<BoundaryName>>(
      "walls", {}, "Boundaries that correspond to solid walls.");
  params.addParam<bool>(
      "linearized_model",
      true,
      "Boolean to determine if the problem should be use in a linear or nonlinear solve.");
  MooseEnum wall_treatment("eq_newton eq_incremental eq_linearized neq", "neq");

  params.addParam<MooseEnum>("wall_treatment",
                             wall_treatment,
                             "The method used for computing the wall functions "
                             "'eq_newton', 'eq_incremental', 'eq_linearized', 'neq'");
  params.addParam<Real>("C_mu", 0.09, "Coupled turbulent kinetic energy closure.");
  params.addParam<Real>("C_pl", 10.0, "Production Limiter Constant Multiplier.");

  return params;
}

LinearFVTKESourceSink::LinearFVTKESourceSink(const InputParameters & params)
  : LinearFVElementalKernel(params),
    _dim(_subproblem.mesh().dimension()),
    _u_var(getFunctor<Real>("u")),
    _v_var(params.isParamValid("v") ? &(getFunctor<Real>("v")) : nullptr),
    _w_var(params.isParamValid("w") ? &(getFunctor<Real>("w")) : nullptr),
    _epsilon(getFunctor<Real>(NS::TKED)),
    _rho(getFunctor<Real>(NS::density)),
    _mu(getFunctor<Real>(NS::mu)),
    _mu_t(getFunctor<Real>(NS::mu_t)),
    _wall_boundary_names(getParam<std::vector<BoundaryName>>("walls")),
    _linearized_model(getParam<bool>("linearized_model")),
    _wall_treatment(getParam<MooseEnum>("wall_treatment").getEnum<NS::WallTreatmentEnum>()),
    _C_mu(getParam<Real>("C_mu")),
    _C_pl(getParam<Real>("C_pl"))
{
  if (_dim >= 2 && !_v_var)
    paramError("v", "In two or more dimensions, the v velocity must be supplied!");

  if (_dim >= 3 && !_w_var)
    paramError("w", "In three or more dimensions, the w velocity must be supplied!");
}

void
LinearFVTKESourceSink::initialSetup()
{
  LinearFVElementalKernel::initialSetup();
  NS::getWallBoundedElements(
      _wall_boundary_names, _fe_problem, _subproblem, blockIDs(), _wall_bounded);
  NS::getWallDistance(_wall_boundary_names, _fe_problem, _subproblem, blockIDs(), _dist);
  NS::getElementFaceArgs(_wall_boundary_names, _fe_problem, _subproblem, blockIDs(), _face_infos);
}

Real
LinearFVTKESourceSink::computeMatrixContribution()
{
  /*
  Matrix contribution:
  - Computes near-wall TKE destruction
  - Computes bulk TKE destruction
  */

  // Useful variables
  const auto state = determineState();
  const auto elem_arg = makeElemArg(_current_elem_info->elem());
  const Real rho = _rho(elem_arg, state);

  if (_wall_bounded.find(_current_elem_info->elem()) != _wall_bounded.end())
  {

    // Useful variables
    const Real mu = _mu(elem_arg, state);
    const Real TKE = _var.getElemValue(*_current_elem_info, state);

    // Assembly variables
    Real production = 0.0;
    Real destruction = 0.0;
    Real tot_weight = 0.0;
    std::vector<Real> y_plus_vec, velocity_grad_norm_vec;

    // Get near-wall element velocity vector
    RealVectorValue velocity(_u_var(elem_arg, state));
    if (_v_var)
      velocity(1) = (*_v_var)(elem_arg, state);
    if (_w_var)
      velocity(2) = (*_w_var)(elem_arg, state);

    // Get faceInfo and distance vector for between all walls and elements
    const auto & face_info_vec = libmesh_map_find(_face_infos, _current_elem_info->elem());
    const auto & distance_vec = libmesh_map_find(_dist, _current_elem_info->elem());
    mooseAssert(distance_vec.size(), "Should have found a distance vector");
    mooseAssert(distance_vec.size() == face_info_vec.size(),
                "Should be as many distance vectors as face info vectors");

    // Populate y+, near wall velocity gradient, and update weights
    for (unsigned int i = 0; i < distance_vec.size(); i++)
    {
      const auto parallel_speed = NS::computeSpeed<Real>(
          velocity - velocity * face_info_vec[i]->normal() * face_info_vec[i]->normal());
      const auto distance = distance_vec[i];

      Real y_plus;
      if (_wall_treatment == NS::WallTreatmentEnum::NEQ) // Non-equilibrium --> Non-iterative
        y_plus = distance * std::sqrt(std::sqrt(_C_mu) * TKE) * rho / mu;
      else // Equilibrium --> Iterative
        y_plus = NS::findyPlus<Real>(mu, rho, std::max(parallel_speed, 1e-10), distance);

      y_plus_vec.push_back(y_plus);
      const Real velocity_grad_norm = parallel_speed / distance;
      velocity_grad_norm_vec.push_back(velocity_grad_norm);
      tot_weight += 1.0;
    }

    // Compute near-wall production and destruction
    for (unsigned int i = 0; i < y_plus_vec.size(); i++)
    {
      const auto y_plus = y_plus_vec[i];

      const auto fi = face_info_vec[i];
      const bool defined_on_elem_side = _var.hasFaceSide(*fi, true);
      const Elem * const loc_elem = defined_on_elem_side ? &fi->elem() : fi->neighborPtr();
      const Moose::FaceArg facearg = {
          fi, Moose::FV::LimiterType::CentralDifference, false, false, loc_elem, nullptr};

      const Real wall_mut = _mu_t(facearg, state);
      const Real wall_mu = _mu(facearg, state);
      const auto tau_w = (wall_mut + wall_mu) * velocity_grad_norm_vec[i];
      const auto destruction_visc = 2.0 * wall_mu / Utility::pow<2>(distance_vec[i]) / tot_weight;
      const auto destruction_log = std::pow(_C_mu, 0.75) * rho * std::pow(TKE, 0.5) /
                                   (NS::von_karman_constant * distance_vec[i]) / tot_weight;

      if (y_plus < 11.25)
        destruction += destruction_visc;
      else
      {
        destruction += destruction_log;
        production += tau_w * std::pow(_C_mu, 0.25) / std::sqrt(TKE) /
                      (NS::von_karman_constant * distance_vec[i]) / tot_weight;
      }
    }

    // Assign terms to matrix to solve implicitly (they get multiplied by TKE)
    return (destruction - production) * _current_elem_volume;
  }
  else
  {
    // Compute destruction
    const auto destruction =
        rho * _epsilon(elem_arg, state) / _var.getElemValue(*_current_elem_info, state);

    // Assign to matrix to solve implicitly
    return destruction * _current_elem_volume;
  }
}

Real
LinearFVTKESourceSink::computeRightHandSideContribution()
{

  // Useful variables
  const auto state = determineState();
  const auto elem_arg = makeElemArg(_current_elem_info->elem());

  if (_wall_bounded.find(_current_elem_info->elem()) != _wall_bounded.end()) // Wall bounded
    return 0.0; // Do nothing
  else // Not wall bounded
  {
    // Compute TKE production
    const auto symmetric_strain_tensor_sq_norm =
        NS::computeShearStrainRateNormSquared<Real>(_u_var, _v_var, _w_var, elem_arg, state);

    auto production = _mu_t(elem_arg, state) * symmetric_strain_tensor_sq_norm;

    // k-Production limiter (needed for flows with stagnation zones)
    const Real production_limit = _C_pl * _rho(elem_arg, state) * _epsilon(elem_arg, state);

    // Apply production limiter
    production = std::min(production, production_limit);

    // Assign production to RHS
    return production * _current_elem_volume;
  }
}
