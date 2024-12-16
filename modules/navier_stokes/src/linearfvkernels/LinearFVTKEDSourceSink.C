//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVTKEDSourceSink.h"
#include "Assembly.h"
#include "SubProblem.h"
#include "NavierStokesMethods.h"

registerMooseObject("MooseApp", LinearFVTKEDSourceSink);

InputParameters
LinearFVTKEDSourceSink::validParams()
{
  InputParameters params = LinearFVElementalKernel::validParams();
  params.addClassDescription("Elemental kernel to compute the production and destruction "
                             " terms of turbulent kinetic energy (TKE).");
  params.addClassDescription("Elemental kernel to compute the production and destruction "
                             " terms of turbulent kinetic energy dissipation (TKED).");
  params.addRequiredParam<MooseFunctorName>("u", "The velocity in the x direction.");
  params.addParam<MooseFunctorName>("v", "The velocity in the y direction.");
  params.addParam<MooseFunctorName>("w", "The velocity in the z direction.");
  params.addRequiredParam<MooseFunctorName>("k", "Coupled turbulent kinetic energy.");
  params.deprecateParam("k", NS::TKE, "01/01/2025");
  params.addRequiredParam<MooseFunctorName>(NS::density, "fluid density");
  params.addRequiredParam<MooseFunctorName>(NS::mu, "Dynamic viscosity.");
  params.addRequiredParam<MooseFunctorName>(NS::mu_t, "Turbulent viscosity.");

  params.addParam<std::vector<BoundaryName>>(
      "walls", {}, "Boundaries that correspond to solid walls.");
  params.addParam<bool>(
      "linearized_model",
      true,
      "Boolean to determine if the problem should be used in a linear or nonlinear solve");
  MooseEnum wall_treatment("eq_newton eq_incremental eq_linearized neq", "neq");
  params.addParam<MooseEnum>("wall_treatment",
                             wall_treatment,
                             "The method used for computing the wall functions "
                             "'eq_newton', 'eq_incremental', 'eq_linearized', 'neq'");

  params.addParam<Real>("C1_eps", 1.44, "First epsilon coefficient");
  params.addParam<Real>("C2_eps", 1.92, "Second epsilon coefficient");
  params.addParam<Real>("C_mu", 0.09, "Coupled turbulent kinetic energy closure.");
  params.addParam<Real>("C_pl", 10.0, "Production limiter constant multiplier.");

  return params;
}

LinearFVTKEDSourceSink::LinearFVTKEDSourceSink(const InputParameters & params)
  : LinearFVElementalKernel(params),
    _dim(_subproblem.mesh().dimension()),
    _u_var(getFunctor<Real>("u")),
    _v_var(params.isParamValid("v") ? &(getFunctor<Real>("v")) : nullptr),
    _w_var(params.isParamValid("w") ? &(getFunctor<Real>("w")) : nullptr),
    _k(getFunctor<Real>(NS::TKE)),
    _rho(getFunctor<Real>(NS::density)),
    _mu(getFunctor<Real>(NS::mu)),
    _mu_t(getFunctor<Real>(NS::mu_t)),
    _wall_boundary_names(getParam<std::vector<BoundaryName>>("walls")),
    _linearized_model(getParam<bool>("linearized_model")),
    _wall_treatment(getParam<MooseEnum>("wall_treatment").getEnum<NS::WallTreatmentEnum>()),
    _C1_eps(getParam<Real>("C1_eps")),
    _C2_eps(getParam<Real>("C2_eps")),
    _C_mu(getParam<Real>("C_mu")),
    _C_pl(getParam<Real>("C_pl"))
{
  if (_dim >= 2 && !_v_var)
    paramError("v", "In two or more dimensions, the v velocity must be supplied!");

  if (_dim >= 3 && !_w_var)
    paramError("w", "In three or more dimensions, the w velocity must be supplied!");
}

void
LinearFVTKEDSourceSink::initialSetup()
{
  LinearFVElementalKernel::initialSetup();
  NS::getWallBoundedElements(
      _wall_boundary_names, _fe_problem, _subproblem, blockIDs(), _wall_bounded);
  NS::getWallDistance(_wall_boundary_names, _fe_problem, _subproblem, blockIDs(), _dist);
  NS::getElementFaceArgs(_wall_boundary_names, _fe_problem, _subproblem, blockIDs(), _face_infos);
}

Real
LinearFVTKEDSourceSink::computeMatrixContribution()
{ 
  if (_wall_bounded.find(_current_elem_info->elem()) != _wall_bounded.end())
    // TKED value for near wall element will be directly assigned for this cell
    return _current_elem_volume;
  else
  {
    // Convenient definitions
    const auto state = determineState();
    const auto elem_arg = makeElemArg(_current_elem_info->elem());
    const Real rho = _rho(elem_arg, state);
    const Real TKE = _k(elem_arg, state);

    // Convenient variables
    Real production = 0.0;
    Real destruction = 0.0;

    // Compute production of TKE
    const auto symmetric_strain_tensor_sq_norm = 
              NS::computeShearStrainRateNormSquared<Real>(_u_var, _v_var, _w_var, elem_arg, state);
    Real production_k = _mu_t(elem_arg, state) * symmetric_strain_tensor_sq_norm;

    // Limit TKE production (needed for flows with stagnation zones)
    const Real production_limit = _C_pl * rho * _var.getElemValue(*_current_elem_info, state);
    production_k = std::min(production_k, production_limit);

    // Compute production and destruction
    production = _C1_eps * production_k / TKE;
    destruction = _C2_eps * rho * _var.getElemValue(*_current_elem_info, state) / TKE;

    // Assign to matrix (term gets multiplied by TKED)
    return (destruction - production*0.0) * _current_elem_volume;
  }
}

Real
LinearFVTKEDSourceSink::computeRightHandSideContribution()
{
  if (_wall_bounded.find(_current_elem_info->elem()) != _wall_bounded.end())
  {
    // Convenient definitions
    const auto state = determineState();
    const auto elem_arg = makeElemArg(_current_elem_info->elem());
    const Real rho = _rho(elem_arg, state);
    const Real mu = _mu(elem_arg, state);
    const Real TKE = _k(elem_arg, state);

    // Convenient variables
    Real destruction = 0.0;
    std::vector<Real> y_plus_vec, velocity_grad_norm_vec;
    Real tot_weight = 0.0;

    // Get velocity vector
    RealVectorValue velocity(_u_var(elem_arg, state));
    if (_v_var)
      velocity(1) = (*_v_var)(elem_arg, state);
    if (_w_var)
      velocity(2) = (*_w_var)(elem_arg, state);

    // Get near wall faceInof and distances from cell center to every wall
    const auto & face_info_vec = libmesh_map_find(_face_infos, _current_elem_info->elem());
    const auto & distance_vec = libmesh_map_find(_dist, _current_elem_info->elem());
    mooseAssert(distance_vec.size(), "Should have found a distance vector");
    mooseAssert(distance_vec.size() == face_info_vec.size(),
                "Should be as many distance vectors as face info vectors");

    // Update y+ and wall face cell
    for (unsigned int i = 0; i < distance_vec.size(); i++)
    {
      const auto distance = distance_vec[i];
      mooseAssert(distance > 0, "Should be at a non-zero distance");

      Real y_plus;
      if (_wall_treatment == NS::WallTreatmentEnum::NEQ) // Non-equilibrium / Non-iterative
        y_plus = distance * std::sqrt(std::sqrt(_C_mu) * TKE) * rho / mu;
      else // Equilibrium / Iterative
      {
        const auto parallel_speed = NS::computeSpeed<Real>(
            velocity - velocity * face_info_vec[i]->normal() * face_info_vec[i]->normal());
        y_plus = NS::findyPlus<Real>(mu, rho, std::max(parallel_speed, 1e-10), distance);
      }

      y_plus_vec.push_back(y_plus);
      tot_weight += 1.0;
    }

    // Compute near wall epsilon value
    for (const auto i : index_range(y_plus_vec))
    {
      const auto y_plus = y_plus_vec[i];

      if (y_plus < 11.25)
      {
        const auto fi = face_info_vec[i];
        const bool defined_on_elem_side = _var.hasFaceSide(*fi, true);
        const Elem * const loc_elem = defined_on_elem_side ? &fi->elem() : fi->neighborPtr();
        const Moose::FaceArg facearg = {
            fi, Moose::FV::LimiterType::CentralDifference, false, false, loc_elem, nullptr};
        destruction += 2.0 * TKE * _mu(facearg, state) / rho /
                       Utility::pow<2>(distance_vec[i]) / tot_weight;
      }
      else
        destruction += std::pow(_C_mu, 0.75) * std::pow(TKE, 1.5) /
                       (NS::von_karman_constant * distance_vec[i]) / tot_weight;
    }

    // Assign the computed value of TKED for element near the wall
    return destruction * _current_elem_volume;
  }
  else
    // Do nothing
    // return 0.0;
  {
    // Convenient definitions
    const auto state = determineState();
    const auto elem_arg = makeElemArg(_current_elem_info->elem());
    const Real rho = _rho(elem_arg, state);
    const Real TKE = _k(elem_arg, state);

    // Convenient variables
    Real production = 0.0;

    // Compute production of TKE
    const auto symmetric_strain_tensor_sq_norm = 
              NS::computeShearStrainRateNormSquared<Real>(_u_var, _v_var, _w_var, elem_arg, state);
    Real production_k = _mu_t(elem_arg, state) * symmetric_strain_tensor_sq_norm;

    // Limit TKE production (needed for flows with stagnation zones)
    const Real production_limit = _C_pl * rho * _var.getElemValue(*_current_elem_info, state);
    production_k = std::min(production_k, production_limit);

    // Compute production and destruction
    production = _C1_eps * production_k / TKE * _var.getElemValue(*_current_elem_info, state);

    // Assign to matrix (term gets multiplied by TKED)
    return production * _current_elem_volume;
  }
}
