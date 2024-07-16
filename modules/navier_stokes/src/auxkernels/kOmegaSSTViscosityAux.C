//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "kOmegaSSTViscosityAux.h"
#include "NavierStokesMethods.h"
#include "NS.h"
#include "NonlinearSystemBase.h"
#include "libmesh/nonlinear_solver.h"

registerMooseObject("NavierStokesApp", kOmegaSSTViscosityAux);

InputParameters
kOmegaSSTViscosityAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Calculates the turbulent viscosity according to the k-epsilon model.");
  params.addRequiredParam<MooseFunctorName>("u", "The velocity in the x direction.");
  params.addParam<MooseFunctorName>("v", "The velocity in the y direction.");
  params.addParam<MooseFunctorName>("w", "The velocity in the z direction.");
  params.addRequiredParam<MooseFunctorName>(NS::TKE, "Coupled turbulent kinetic energy.");
  params.addRequiredParam<MooseFunctorName>(
      NS::TKESD, "Coupled turbulent kinetic energy specific dissipation rate.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "Density");
  params.addRequiredParam<MooseFunctorName>(NS::mu, "Dynamic viscosity.");
  params.addRequiredParam<MooseFunctorName>("F2", "The F2 blending function.");
  params.addParam<std::vector<BoundaryName>>(
      "walls", {}, "Boundaries that correspond to solid walls.");
  params.addParam<bool>("bulk_wall_treatment", false, "Activate bulk wall treatment.");
  MooseEnum wall_treatment("eq_newton eq_incremental eq_linearized neq", "eq_newton");
  params.addParam<MooseEnum>("wall_treatment",
                             wall_treatment,
                             "The method used for computing the wall functions."
                             "'eq_newton', 'eq_incremental', 'eq_linearized', 'neq'");
  params.addParam<bool>(
      "low_Re_modification", false, "Activate low Reynolds number modifications.");
  params.addParam<MooseFunctorName>("F1", "The F1 blending function.");
  return params;
}

kOmegaSSTViscosityAux::kOmegaSSTViscosityAux(const InputParameters & params)
  : AuxKernel(params),
    _dim(_subproblem.mesh().dimension()),
    _u_var(getFunctor<ADReal>("u")),
    _v_var(params.isParamValid("v") ? &(getFunctor<ADReal>("v")) : nullptr),
    _w_var(params.isParamValid("w") ? &(getFunctor<ADReal>("w")) : nullptr),
    _k(getFunctor<ADReal>(NS::TKE)),
    _omega(getFunctor<ADReal>(NS::TKESD)),
    _rho(getFunctor<ADReal>(NS::density)),
    _mu(getFunctor<ADReal>(NS::mu)),
    _F2(getFunctor<ADReal>("F2")),
    _wall_boundary_names(getParam<std::vector<BoundaryName>>("walls")),
    _bulk_wall_treatment(getParam<bool>("bulk_wall_treatment")),
    _wall_treatment(getParam<MooseEnum>("wall_treatment")),
    _bool_low_Re_modification(getParam<bool>("low_Re_modification")),
    _F1(params.isParamValid("F1") ? &(getFunctor<ADReal>("F1")) : nullptr)
{
  if (_bool_low_Re_modification && (!_F1))
    paramError("F1",
               "The F1 blending function should be provided when low Reynolds number modifications "
               "are activated.");
}

void
kOmegaSSTViscosityAux::initialSetup()
{
  if (!_wall_boundary_names.empty())
  {
    NS::getWallBoundedElements(
        _wall_boundary_names, _c_fe_problem, _subproblem, blockIDs(), _wall_bounded);
    NS::getWallDistance(_wall_boundary_names, _c_fe_problem, _subproblem, blockIDs(), _dist);
    NS::getElementFaceArgs(
        _wall_boundary_names, _c_fe_problem, _subproblem, blockIDs(), _face_infos);
  }
}

Real
kOmegaSSTViscosityAux::computeValue()
{
  // Convenient Arguments
  const Elem & elem = *_current_elem;
  const auto current_argument = makeElemArg(_current_elem);
  const Moose::StateArg state =
      Moose::StateArg(1, Moose::SolutionIterationType::Nonlinear); // determineState();
  const auto rho = _rho(makeElemArg(_current_elem), state);
  const auto mu = _mu(makeElemArg(_current_elem), state);
  const auto nu = mu / rho;

  // Determine if the element is wall bounded
  // and if bulk wall treatment needs to be activated
  const bool wall_bounded = _wall_bounded.find(_current_elem) != _wall_bounded.end();
  Real mu_t;

  // Computing wall value for near-wall elements if bulk wall treatement is activated
  // bulk_wall_treatement should only be used for very coarse mesh problems
  if (wall_bounded && _bulk_wall_treatment)
  {
    // Computing wall value for turbulent dynamic visocisity
    const auto & elem_distances = _dist[&elem];
    const auto min_wall_distance_iterator =
        (std::min_element(elem_distances.begin(), elem_distances.end()));
    const auto min_wall_dist = *min_wall_distance_iterator;
    const size_t minIndex = std::distance(elem_distances.begin(), min_wall_distance_iterator);
    const auto loc_normal = _face_infos[&elem][minIndex]->normal();

    // Getting y_plus
    ADRealVectorValue velocity(_u_var(current_argument, state));
    if (_v_var)
      velocity(1) = (*_v_var)(current_argument, state);
    if (_w_var)
      velocity(2) = (*_w_var)(current_argument, state);

    // Compute the velocity and direction of the velocity component that is parallel to the wall
    const ADReal parallel_speed = (velocity - velocity * loc_normal * loc_normal).norm();

    // Switch for determining the near wall quantities
    // wall_treatment can be: "eq_newton eq_incremental eq_linearized neq"
    ADReal y_plus;
    ADReal mut_log; // turbulent log-layer viscosity
    ADReal mu_wall; // total wall viscosity to obtain the shear stress at the wall

    if (_wall_treatment == "eq_newton")
    {
      // Full Newton-Raphson solve to find the wall quantities from the law of the wall
      const auto u_tau = NS::findUStar(mu, rho, parallel_speed, min_wall_dist);
      y_plus = min_wall_dist * u_tau * rho / mu;
      mu_wall = rho * Utility::pow<2>(u_tau) * min_wall_dist / parallel_speed;
      mut_log = mu_wall - mu;
    }
    else if (_wall_treatment == "eq_incremental")
    {
      // Incremental solve on y_plus to get the near-wall quantities
      y_plus = NS::findyPlus(mu, rho, std::max(parallel_speed, 1e-10), min_wall_dist);
      mu_wall = mu * (NS::von_karman_constant * y_plus /
                      std::log(std::max(NS::E_turb_constant * y_plus, 1 + 1e-4)));
      mut_log = mu_wall - mu;
    }
    else if (_wall_treatment == "eq_linearized")
    {
      // Linearized approximation to the wall function to find the near-wall quantities faster
      const ADReal a_c = 1 / NS::von_karman_constant;
      const ADReal b_c = 1 / NS::von_karman_constant *
                         (std::log(NS::E_turb_constant * std::max(min_wall_dist, 1.0) / mu) + 1.0);
      const ADReal c_c = parallel_speed;

      const auto u_tau = (-b_c + std::sqrt(std::pow(b_c, 2) + 4.0 * a_c * c_c)) / (2.0 * a_c);
      y_plus = min_wall_dist * u_tau * rho / mu;
      mu_wall = rho * Utility::pow<2>(u_tau) * min_wall_dist / parallel_speed;
      mut_log = mu_wall - mu;
    }
    else if (_wall_treatment == "neq")
    {
      // Assign non-equilibrium wall function value
      y_plus =
          std::pow(_C_mu, 0.25) * min_wall_dist * std::sqrt(_k(current_argument, state)) * rho / mu;
      mu_wall = mu * (NS::von_karman_constant * y_plus /
                      std::log(std::max(NS::E_turb_constant * y_plus, 1 + 1e-4)));
      mut_log = mu_wall - mu;
    }
    else
      mooseAssert(false, "Should not reach here");

    if (y_plus <= 5.0)
      // sub-laminar layer
      mu_t = 0.0;
    else if (y_plus >= 30.0)
      // log-layer
      mu_t = std::max(mut_log.value(), NS::mu_t_low_limit);
    else
    {
      // buffer layer
      const auto blending_function = (y_plus - 5.0) / 25.0;
      // the blending depends on the mut_log at y+=30
      const auto mut_log = mu * (NS::von_karman_constant * 30.0 /
                                     std::log(std::max(NS::E_turb_constant * 30.0, 1 + 1e-4)) -
                                 1.0);
      mu_t = std::max(raw_value(blending_function * mut_log), NS::mu_t_low_limit);
    }
  }
  else
  {

    // Useful variables
    const auto rho = _rho(current_argument, state);
    const auto TKE = _k(current_argument, state);
    const auto omega_capped = std::max(_omega(current_argument, state), 1e-10);

    // ***** Computing shear strain rate ***** //
    // *************************************** //
    // Computing wall scaling
    const auto & grad_u = _u_var.gradient(current_argument, state);
    const auto Sij_xx = 2.0 * grad_u(0);
    ADReal Sij_xy = 0.0;
    ADReal Sij_xz = 0.0;
    ADReal Sij_yy = 0.0;
    ADReal Sij_yz = 0.0;
    ADReal Sij_zz = 0.0;

    const auto grad_xx = grad_u(0);
    ADReal grad_xy = 0.0;
    ADReal grad_xz = 0.0;
    ADReal grad_yx = 0.0;
    ADReal grad_yy = 0.0;
    ADReal grad_yz = 0.0;
    ADReal grad_zx = 0.0;
    ADReal grad_zy = 0.0;
    ADReal grad_zz = 0.0;

    auto trace = Sij_xx / 3.0;

    if (_dim >= 2)
    {
      const auto & grad_v = (*_v_var).gradient(current_argument, state);
      Sij_xy = grad_u(1) + grad_v(0);
      Sij_yy = 2.0 * grad_v(1);

      grad_xy = grad_u(1);
      grad_yx = grad_v(0);
      grad_yy = grad_v(1);

      trace += Sij_yy / 3.0;

      if (_dim >= 3)
      {
        const auto & grad_w = (*_w_var).gradient(current_argument, state);

        Sij_xz = grad_u(2) + grad_w(0);
        Sij_yz = grad_v(2) + grad_w(1);
        Sij_zz = 2.0 * grad_w(2);

        grad_xz = grad_u(2);
        grad_yz = grad_v(2);
        grad_zx = grad_w(0);
        grad_zy = grad_w(1);
        grad_zz = grad_w(2);

        trace += Sij_zz / 3.0;
      }
    }

    const auto symmetric_strain_tensor_norm =
        std::sqrt((Sij_xx - trace) * grad_xx + Sij_xy * grad_xy + Sij_xz * grad_xz +
                  Sij_xy * grad_yx + (Sij_yy - trace) * grad_yy + Sij_yz * grad_yz +
                  Sij_xz * grad_zx + Sij_yz * grad_zy + (Sij_zz - trace) * grad_zz);
    // *************************************** //

    // Low-Re modification
    ADReal alpha_star(1.0);
    if (_bool_low_Re_modification)
    {
      const auto F1 = (*_F1)(current_argument, state);
      const auto beta = F1 * _beta_i_1 + (1.0 - F1) * _beta_i_2;
      const auto Re_shear = rho * TKE / (_mu(current_argument, state) * omega_capped);
      const auto alpha_1_star = ((beta / 3.0) + Re_shear / _Re_k) / (1.0 + Re_shear / _Re_k);
      const auto alpha_star = F1 * alpha_1_star + (1.0 - F1) * _alpha_2_star;
    }

    // Limited time scale
    const auto T = std::min(alpha_star / omega_capped,
                            _a_1 / symmetric_strain_tensor_norm / _F2(current_argument, state));

    // Dynamic turbulent viscosity
    mu_t = (rho * TKE * T).value();
  }

  return mu_t;
}
