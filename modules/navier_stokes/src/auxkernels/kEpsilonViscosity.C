//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "kEpsilonViscosity.h"
#include "NavierStokesMethods.h"

registerMooseObject("NavierStokesApp", kEpsilonViscosity);

InputParameters
kEpsilonViscosity::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Calculates the turbulent viscosity according to the k-epsilon model.");
  params.addRequiredCoupledVar("u", "The velocity in the x direction.");
  params.addCoupledVar("v", "The velocity in the y direction.");
  params.addCoupledVar("w", "The velocity in the z direction.");
  params.addRequiredParam<MooseFunctorName>("k", "Coupled turbulent kinetic energy.");
  params.addRequiredParam<MooseFunctorName>("epsilon",
                                            "Coupled turbulent kinetic energy dissipation rate.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "Density");
  params.addRequiredParam<MooseFunctorName>("mu", "Dynamic viscosity.");
  params.addRequiredParam<MooseFunctorName>("C_mu", "Coupled turbulent kinetic energy closure.");
  params.addRequiredParam<std::vector<BoundaryName>>("walls",
                                                     "Boundaries that correspond to solid walls.");
  params.addParam<bool>(
      "linearized_yplus",
      false,
      "Boolean to indicate if yplus must be estimate locally for the blending functions.");

  params.addParam<unsigned int>(
      "n_iters_activate", 1, "Relaxation iterations after which the k-epsilon model is activated.");

  params.addParam<Real>("max_mixing_length",
                        10.0,
                        "Maximum mixing legth allowed for the domain - adjust if seeking for "
                        "realizable k-epsilon answer.");
  params.addParam<bool>(
      "wall_treatement", true, "Activate wall treatement by adding wall functions.");
  params.addParam<bool>(
      "non_equilibrium_treatement",
      false,
      "Use non-equilibrium wall treatement (faster than standard wall treatement)");
  return params;
}

kEpsilonViscosity::kEpsilonViscosity(const InputParameters & params)
  : AuxKernel(params),
    _dim(_subproblem.mesh().dimension()),
    _u_var(dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("u", 0))),
    _v_var(params.isParamValid("v")
               ? dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("v", 0))
               : nullptr),
    _w_var(params.isParamValid("w")
               ? dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("w", 0))
               : nullptr),
    _k(getFunctor<ADReal>("k")),
    _epsilon(getFunctor<ADReal>("epsilon")),
    _rho(getFunctor<ADReal>(NS::density)),
    _mu(getFunctor<ADReal>("mu")),
    _C_mu(getFunctor<ADReal>("C_mu")),
    _wall_boundary_names(getParam<std::vector<BoundaryName>>("walls")),
    _linearized_yplus(getParam<bool>("linearized_yplus")),
    _n_kernel_iters(0),
    _n_iters_activate(getParam<unsigned int>("n_iters_activate")),
    _max_mixing_length(getParam<Real>("max_mixing_length")),
    _wall_treatement(getParam<bool>("wall_treatement")),
    _non_equilibrium_treatement(getParam<bool>("non_equilibrium_treatement"))
{
}

Real
kEpsilonViscosity::computeValue()
{

  // Boundary value
  const Elem & elem = *_current_elem;
  auto current_argument = makeElemArg(_current_elem);

  bool wall_bounded = false;
  Real min_wall_dist = 0.0;
  Point loc_normal;

  if (elem == (*(*_mesh.activeLocalElementsBegin())))
  {
    _n_kernel_iters += 1;
    if (_n_kernel_iters == (_n_iters_activate + 2)) //+2 since we need to consider assembly passes
    {
      _console << "Activating k-epsilon model." << std::endl;
    }
  }

  if (_n_kernel_iters < (_n_iters_activate + 2)) //+2 since we need to consider assembly passes
    return 100.0 +
           _mu(current_argument)
               .value(); /// TODO: need to compute this value dynamically based in the condition number of the problem

  for (unsigned int i_side = 0; i_side < elem.n_sides(); ++i_side)
  {
    const std::vector<BoundaryID> side_bnds =
        _subproblem.mesh().getBoundaryIDs(_current_elem, i_side);

    for (const BoundaryName & name : _wall_boundary_names)
    {
      BoundaryID wall_id = _subproblem.mesh().getBoundaryID(name);
      for (BoundaryID side_id : side_bnds)
      {
        if (side_id == wall_id)
        {
          const FaceInfo * const fi = _mesh.faceInfo(&elem, i_side);
          Real dist = std::abs((fi->elemCentroid() - fi->faceCentroid()) * fi->normal());

          if (dist > min_wall_dist)
          {
            min_wall_dist = dist;
            loc_normal = fi->normal();
          }
          wall_bounded = true;
        }
      }
    }
  }

  if (wall_bounded && _wall_treatement)
  {
    // return wall function

    constexpr Real karman_cte = 0.4187;
    constexpr Real E = 9.793;

    // Getting y_plus
    ADRealVectorValue velocity(_u_var->getElemValue(&elem));
    if (_v_var)
      velocity(1) = _v_var->getElemValue(&elem);
    if (_w_var)
      velocity(2) = _w_var->getElemValue(&elem);

    // Compute the velocity and direction of the velocity component that is parallel to the wall
    ADReal parallel_speed = (velocity - velocity * loc_normal * loc_normal).norm();

    ADReal y_plus;
    if (_non_equilibrium_treatement)
    {
      ADReal y_plus = _rho(current_argument) * std::pow(_C_mu(current_argument), 0.25) *
                      std::pow(_k(current_argument), 0.5) * min_wall_dist / _mu(current_argument);
    }
    else
    {
      ADReal u_star;
      if (_linearized_yplus)
      {
        const ADReal a_c = 1 / karman_cte;
        const ADReal b_c =
            1 / karman_cte * (std::log(E * min_wall_dist / _mu(current_argument)) + 1.0);
        const ADReal c_c = parallel_speed;
        u_star = (-b_c + std::sqrt(std::pow(b_c, 2) + 4.0 * a_c * c_c)) / (2.0 * a_c);
      }
      else
        u_star = NS::findUStar(
            _mu(current_argument), _rho(current_argument), parallel_speed, min_wall_dist);

      y_plus = min_wall_dist * u_star * _rho(current_argument) / _mu(current_argument);
    }

    if (y_plus <= 5.0) // sub-laminar layer
    {
      return 0.0 + _mu(current_argument).value(); // Formulation corresponding to Von Karman
                                                  // sublaminar layer description
      // TODO: Not sure how to model the sublaminar layer with upwind turbulent production
      // conditions OpenFOAM and Star-CCM+ treatements suck. Fluent does not provide enough details
    }
    else if (y_plus >=
             30.0) // log-layer (actaully potential layer regarding modern research - change later)
    {
      auto von_karman_value = (1 / karman_cte + std::log(E * y_plus));
      ADReal new_u_tau_squared;
      if (_non_equilibrium_treatement)
      {
        new_u_tau_squared = std::pow(_C_mu(current_argument), 0.25) *
                            std::pow(_k(current_argument), 0.5) * parallel_speed / von_karman_value;
      }
      else
      {
        new_u_tau_squared = std::pow(parallel_speed / von_karman_value, 2);
      }
      auto wall_val = new_u_tau_squared * _rho(current_argument) * min_wall_dist / parallel_speed;
      return wall_val.value() + _mu(current_argument).value();
    }
    else // my experimental blending function
    {
      auto von_karman_value = (1 / karman_cte + std::log(E * y_plus));
      ADReal new_u_tau_squared;
      if (_non_equilibrium_treatement)
      {
        new_u_tau_squared = std::pow(_C_mu(current_argument), 0.25) *
                            std::pow(_k(current_argument), 0.5) * parallel_speed / von_karman_value;
      }
      else
      {
        new_u_tau_squared = std::pow(parallel_speed / von_karman_value, 2);
      }
      auto blending_function = (y_plus - 5.0) / 25.0;
      auto wall_val =
          (new_u_tau_squared * _rho(current_argument) * min_wall_dist / parallel_speed) *
          blending_function;
      return wall_val.value() + _mu(current_argument).value();
      // Note: ideally the user should not include values in the buffer layer as intermittency does
      // not allow to define the wall function properly and bla bla However, we are allowing for
      // values in the buffer layer as otherwise it is cumbersome for the user
      // TODO: raise warning if some of the cell values lie in the buffer layer
    }
  }
  else
  {
    //  Return Bulk value
    constexpr Real protection_epsilon = 1e-11;
    // auto current_argument = makeElemArg(_current_elem);
    // auto Launder_Sharma_bulk_limit = 0.5 * _mu(current_argument);

    auto k_value = std::max(_k(current_argument), 1e-10);
    auto epsilon_value = std::max(_epsilon(current_argument), 1e-10);

    ADReal local_mixing_length = (_rho(current_argument) * _C_mu(current_argument) *
                                  std::pow(k_value, 2) / (epsilon_value + protection_epsilon))
                                     .value();

    bool mixing_length_cond = (_C_mu(current_argument) * std::pow(_k(current_argument), 1.5)) <
                              (_epsilon(current_argument) * _max_mixing_length);
    local_mixing_length = (mixing_length_cond) ? local_mixing_length : _max_mixing_length;

    return (local_mixing_length * std::pow(k_value, 0.5)).value() + _mu(current_argument).value();

    // return (_rho(current_argument) * _C_mu(current_argument) * std::pow(k_value, 2) /
    //         (epsilon_value + protection_epsilon))
    //            .value() +
    //        _mu(current_argument).value();
  }
}
