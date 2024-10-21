//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RANSYPlusAux.h"
#include "NonlinearSystemBase.h"
#include "NavierStokesMethods.h"
#include "libmesh/nonlinear_solver.h"

registerMooseObject("NavierStokesApp", RANSYPlusAux);

InputParameters
RANSYPlusAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Calculates non-dimensional wall distance (y+) value.");
  params.addRequiredCoupledVar("u", "The velocity in the x direction.");
  params.addCoupledVar("v", "The velocity in the y direction.");
  params.addCoupledVar("w", "The velocity in the z direction.");
  params.addParam<MooseFunctorName>("k", "Turbulent kinetic energy functor.");
  params.deprecateParam("k", NS::TKE, "01/01/2025");
  params.addRequiredParam<MooseFunctorName>(NS::density, "Fluid density.");
  params.addRequiredParam<MooseFunctorName>(NS::mu, "Dynamic viscosity.");
  params.addParam<std::vector<BoundaryName>>(
      "walls", {}, "Boundaries that correspond to solid walls.");
  MooseEnum wall_treatment("eq_newton eq_incremental eq_linearized neq", "neq");
  params.addParam<MooseEnum>("wall_treatment",
                             wall_treatment,
                             "The method used for computing the y_plus in the wall functions "
                             "'eq_newton', 'eq_incremental', 'eq_linearized', 'neq'");
  params.addParam<Real>("C_mu", 0.09, "Coupled turbulent kinetic energy closure coefficient.");

  return params;
}

RANSYPlusAux::RANSYPlusAux(const InputParameters & params)
  : AuxKernel(params),
    _dim(_subproblem.mesh().dimension()),
    _u_var(getFunctor<ADReal>("u")),
    _v_var(params.isParamValid("v") ? &(getFunctor<ADReal>("v")) : nullptr),
    _w_var(params.isParamValid("w") ? &(getFunctor<ADReal>("w")) : nullptr),
    _k(params.isParamValid(NS::TKE) ? &(getFunctor<ADReal>(NS::TKE)) : nullptr),
    _rho(getFunctor<ADReal>(NS::density)),
    _mu(getFunctor<ADReal>(NS::mu)),
    _wall_boundary_names(getParam<std::vector<BoundaryName>>("walls")),
    _wall_treatment(getParam<MooseEnum>("wall_treatment").getEnum<NS::WallTreatmentEnum>()),
    _C_mu(getParam<Real>("C_mu"))
{
  if (_dim >= 2 && !_v_var)
    paramError("v", "In two or more dimensions, the v velocity must be supplied!");

  if (_dim >= 3 && !_w_var)
    paramError("w", "In three or more dimensions, the w velocity must be supplied!");

  if (_wall_treatment == NS::WallTreatmentEnum::NEQ && !_k)
    paramError(NS::TKE, "In the non-equilibrium wall treatment the TKE must be supplied!");
}

void
RANSYPlusAux::initialSetup()
{
  // Getting wall treatment maps
  NS::getWallBoundedElements(
      _wall_boundary_names, _c_fe_problem, _subproblem, blockIDs(), _wall_bounded);
  NS::getWallDistance(_wall_boundary_names, _c_fe_problem, _subproblem, blockIDs(), _dist);
  NS::getElementFaceArgs(_wall_boundary_names, _c_fe_problem, _subproblem, blockIDs(), _face_infos);
}

Real
RANSYPlusAux::computeValue()
{

  if (_wall_bounded.find(_current_elem) != _wall_bounded.end())
  {
    const auto state = determineState();
    const auto elem_arg = makeElemArg(_current_elem);
    const auto rho = _rho(elem_arg, state);
    const auto mu = _mu(elem_arg, state);
    ADReal y_plus;
    std::vector<Real> y_plus_vec;

    ADRealVectorValue velocity(_u_var(elem_arg, state));
    if (_v_var)
      velocity(1) = (*_v_var)(elem_arg, state);
    if (_w_var)
      velocity(2) = (*_w_var)(elem_arg, state);

    const auto & face_info_vec = libmesh_map_find(_face_infos, _current_elem);
    const auto & distance_vec = libmesh_map_find(_dist, _current_elem);

    for (unsigned int i = 0; i < distance_vec.size(); i++)
    {
      const auto parallel_speed = NS::computeSpeed(
          velocity - velocity * face_info_vec[i]->normal() * face_info_vec[i]->normal());
      const auto distance = distance_vec[i];

      if (_wall_treatment == NS::WallTreatmentEnum::NEQ)
        // Non-equilibrium / Non-iterative
        y_plus = std::pow(_C_mu, 0.25) * distance * std::sqrt((*_k)(elem_arg, state)) * rho / mu;
      else
        // Equilibrium / Iterative
        y_plus = NS::findyPlus(mu, rho, std::max(parallel_speed, 1e-10), distance);

      y_plus_vec.push_back(raw_value(y_plus));
    }
    // Return average of y+ for cells with multiple wall faces
    return std::accumulate(y_plus_vec.begin(), y_plus_vec.end(), 0.0) / y_plus_vec.size();
  }
  else
    return 0.;
}
