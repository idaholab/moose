//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSFV2PSlipVelocityFunctorMaterialTempl.h"
#include "INSFVVelocityVariable.h"
#include "FEProblemBase.h"
#include "Function.h"
#include "NS.h"
#include "NavierStokesMethods.h"

#include "libmesh/utility.h"

#include <limits>
#include "HeatTransferUtils.h"
#include "NewtonInversion.h"

namespace
{
/**
 * Re_p times the derivative of schillerNaumannDragFunction with respect to Re_p, written as the
 * product because \f$ Re_p \, f'(Re_p) \f$ is finite at the origin while \f$ f' \f$ alone is not.
 */
template <typename T>
T
schillerNaumannDragDerivative(const T & Re_p)
{
  using std::pow;
  mooseAssert(MetaPhysicL::raw_value(Re_p) >= 0,
              "The particle Reynolds number is formed from a magnitude");
  return 0.15 * 0.687 * pow(Re_p, 0.687);
}
/**
 * Solves Manninen's force balance together with the drag correlation for the slip speed:
 * \f$ s \, f(R s) = s_0 \f$, where \f$ s_0 \f$ is the slip speed in the Stokes limit and
 * \f$ R \f$ converts a speed into a particle Reynolds number. The left hand side is zero at
 * \f$ s = 0 \f$ and strictly increasing, so the root is unique, and since \f$ f \ge 1 \f$ it
 * lies in \f$ [0, s_0] \f$. Templated so that both slip closures can call it.
 */
template <typename T>
T
solveSlipSpeed(const T & stokes_speed, const T & reynolds_per_speed)
{
  using std::sqrt;
  // f(0) = 1, so a vanishing acceleration gives a vanishing slip and the drag never enters
  if (MetaPhysicL::raw_value(stokes_speed) <= 0.0 ||
      MetaPhysicL::raw_value(reynolds_per_speed) <= 0.0)
    return stokes_speed;

  // Solve in Reynolds number rather than in speed: multiplying s f(R s) = s0 through by R gives
  // Re f(Re) = B, with B = R s0 formed entirely from inputs, so the branch of f is chosen before
  // iterating rather than re-tested on every pass.
  const T driving_group = reynolds_per_speed * stokes_speed;

  // Below this the drag correction 0.15 Re^0.687 is 3e-13, smaller than the relative tolerance the
  // loop would converge to, so the Stokes answer is already the converged one.
  constexpr Real negligible_driving_group = 1e-17;
  if (MetaPhysicL::raw_value(driving_group) < negligible_driving_group)
    return stokes_speed;

  // In Newton's regime f = 0.0183 Re, so the balance becomes 0.0183 Re^2 = B and is exact. The
  // branches of dragFunction change over at Re = 1000, which is this value of B.
  constexpr Real newton_regime_driving_group = 0.0183 * 1000.0 * 1000.0;
  if (MetaPhysicL::raw_value(driving_group) >= newton_regime_driving_group)
    return sqrt(driving_group / 0.0183) / reynolds_per_speed;

  // g(Re) = Re f(Re) = Re + 0.15 Re^1.687 is increasing and convex on [0, inf), so the tangent lies
  // below it and every Newton step from a positive iterate lands at or beyond the root, from where
  // the iterates decrease monotonically onto it. Plain Newton therefore converges without leaving
  // [0, B] and never forms a fractional power of a negative number. The iteration starts from one
  // Picard step off the Stokes guess Re = B.
  // Near round-off, so that the slip speed carries no iteration error, and reached in a handful of
  // iterations from this starting point
  constexpr Real rel_tol = 1e-12;
  const T reynolds =
      FluidPropertiesUtils::NewtonSolve(
          driving_group,
          driving_group,
          MetaPhysicL::raw_value(driving_group / NS::schillerNaumannDragFunction(driving_group)),
          rel_tol,
          [](const T & /*B*/, const T & re, T & g, T & dg_dB, T & dg_dre)
          {
            // d/dRe [Re f(Re)] = f(Re) + Re f'(Re), the second term written as the finite product
            const T drag = NS::schillerNaumannDragFunction(re);
            g = re * drag;
            dg_dB = 0;
            dg_dre = drag + schillerNaumannDragDerivative(re);
          },
          "WCNSFV2PSlipVelocityFunctorMaterial")
          .first;

  return reynolds / reynolds_per_speed;
}
/**
 * The linear drag function of a distorted fluid particle, the deformed-bubble regime in which the
 * drag coefficient grows with size rather than falling with Reynolds number. Substituted into the
 * closure it gives Ishii's terminal velocity for bubbly flow, independent of the particle size; the
 * derivation is on the WCNSFV2PSlipVelocityFunctorMaterial documentation page.
 *
 * Unlike dragFunction this one is linear in the slip speed, so it is returned per unit speed: the
 * caller multiplies by \f$ \left|u_s\right| \f$.
 *
 * @param particle_diameter diameter of the particles of the dispersed phase
 * @param rho_c continuous phase density
 * @param mu_c continuous phase dynamic viscosity
 * @param delta_rho magnitude of the density difference between the phases
 * @param sigma surface tension between the phases
 * @param gravity_magnitude magnitude of the gravity vector
 */
template <typename T>
inline T
distortedDragFunctionPerSpeed(const T & particle_diameter,
                              const T & rho_c,
                              const T & mu_c,
                              const T & delta_rho,
                              const T & sigma,
                              const Real gravity_magnitude)
{
  using std::sqrt;
  return Utility::pow<2>(particle_diameter) * rho_c / (36.0 * mu_c) *
         sqrt(gravity_magnitude * delta_rho / sigma);
}
}

registerMooseObject("NavierStokesApp", WCNSFV2PSlipVelocityFunctorMaterial);
registerMooseObject("NavierStokesApp", ADWCNSFV2PSlipVelocityFunctorMaterial);

template <bool is_ad>
InputParameters
WCNSFV2PSlipVelocityFunctorMaterialTempl<is_ad>::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addClassDescription(
      "Computes the slip velocity, and optionally the diffusion (drift) velocity derived from it, "
      "for the two-phase mixture model.");
  params.addRequiredParam<SolverVariableName>("u", "The velocity in the x direction.");
  params.addParam<SolverVariableName>("v", "The velocity in the y direction.");
  params.addParam<SolverVariableName>("w", "The velocity in the z direction.");
  params.addRequiredParam<MooseFunctorName>(
      NS::density,
      "Mixture density. This is the density that appears in the buoyancy factor "
      "(rho_d - rho_m) / rho_d of the algebraic slip closure.");
  params.addRequiredParam<MooseFunctorName>(NS::dispersed_phase_density,
                                            "Dispersed phase density.");
  params.addRequiredParam<MooseFunctorName>(
      NS::mu,
      "Continuous phase dynamic viscosity. Both the particle relaxation time and the particle "
      "Reynolds number are formed from it, which is the single particle form of the closure. "
      "A volume averaged mixture viscosity here would invert the swarm correction.");
  params.addParam<RealVectorValue>(
      "gravity", RealVectorValue(0, 0, 0), "Gravity acceleration vector");
  params.addParam<Real>("force_value", 0.0, "Coefficient to multiply by the body force term");
  params.addParam<FunctionName>("force_function", "0", "A function that describes the body force");
  params.addParam<PostprocessorName>(
      "force_postprocessor", 0, "A postprocessor whose value is multiplied by the body force");
  params.addParam<RealVectorValue>(
      "force_direction", RealVectorValue(1, 0, 0), "Direction of the body force");
  params.addParam<MooseFunctorName>(
      "linear_coef_name",
      "Prescribed linear drag function. Supply this to impose the drag directly. Leave it unset "
      "and set 'use_dispersed_phase_drag_model' to have the Schiller and Naumann correlation "
      "solved together with the force balance instead.");
  params.addParam<bool>(
      "use_dispersed_phase_drag_model",
      false,
      "Whether to evaluate the drag from the Schiller and Naumann correlation, solved "
      "self-consistently with the slip velocity so that the particle Reynolds number is formed "
      "from the slip velocity as its definition requires.");
  params.addParam<MooseFunctorName>(
      NS::continuous_phase_density,
      "Continuous phase density, used to form the particle Reynolds number.");
  MooseEnum drag_model("schiller-naumann distorted-particle automatic ishii-zuber",
                       "schiller-naumann");
  params.addParam<MooseEnum>(
      "drag_model",
      drag_model,
      "Drag law closing the force balance when 'use_dispersed_phase_drag_model' is set. "
      "'schiller-naumann' is the rigid sphere law and is accurate while the particle stays "
      "spherical. 'distorted-particle' is the deformed bubble law, whose terminal velocity is "
      "independent of the particle size and reproduces Ishii's drift velocity correlation. "
      "'automatic' takes whichever of the two resists more, which selects the regime by itself. "
      "'ishii-zuber' is the multi-bubble relation of Ishii and Zuber, which predicts the relative "
      "velocity directly instead of supplying a drag to close the balance; it carries its own "
      "concentration dependence and therefore ignores 'swarm_exponent'.");
  params.addParam<MooseFunctorName>(
      "friction_pressure_gradient",
      "0",
      "Frictional pressure gradient of the two phase flow, which carries the effect of wall "
      "friction on the relative velocity. Only used by the 'ishii-zuber' drag model. The default "
      "of zero leaves the balance gravity dominated.");
  params.addParam<MooseFunctorName>(
      "single_particle_friction_pressure_gradient",
      "0",
      "Frictional pressure gradient of the corresponding single particle system. Only used by the "
      "'ishii-zuber' drag model, where it affects both the single particle terminal velocity and "
      "the concentration dependence.");
  params.addParam<MooseFunctorName>(
      "surface_tension",
      "Surface tension between the phases. Required by the distorted particle drag.");
  params.addParam<Real>(
      "swarm_exponent",
      0.0,
      "Exponent p of the hindrance factor (1 - alpha)^p multiplying the slip velocity. A particle "
      "in a swarm meets more resistance than an isolated one, and 'schiller-naumann' and "
      "'distorted-particle' are both single particle laws, so they need this correction. "
      "Ignored by 'ishii-zuber', which carries its own concentration dependence.");

  params.addParam<MooseFunctorName>(
      "particle_diameter", 1.0, "Diameter of particles in the dispersed phase.");
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this material applies to.");
  params.addRequiredParam<MooseFunctorName>("slip_velocity_name", "the name of the slip velocity");
  params.addParam<MooseFunctorName>(
      "drift_velocity_name",
      "Name of the diffusion (drift) velocity property to declare, the velocity of the dispersed "
      "phase relative to the centre of mass of the mixture. Declared only when supplied.");
  params.addParam<MooseFunctorName>(
      "fraction_dispersed",
      0.0,
      "Volume fraction of the dispersed phase. It converts the slip velocity into the drift "
      "velocity, it sets the swarm hindrance factor, and it supplies the whole concentration "
      "dependence of the 'ishii-zuber' drag model.");
  params.addParam<unsigned short>("ghost_layers",
                                  3,
                                  "The number of layers of elements to ghost. With Rhie-Chow and "
                                  "the velocity gradient calculation below, we need 3");
  params.addRelationshipManager(
      "ElementSideNeighborLayers",
      Moose::RelationshipManagerType::GEOMETRIC | Moose::RelationshipManagerType::ALGEBRAIC |
          Moose::RelationshipManagerType::COUPLING,
      [](const InputParameters & obj_params, InputParameters & rm_params)
      {
        rm_params.set<unsigned short>("layers") = obj_params.get<unsigned short>("ghost_layers");
      });
  return params;
}

template <bool is_ad>
typename WCNSFV2PSlipVelocityFunctorMaterialTempl<is_ad>::VelocityVariable &
WCNSFV2PSlipVelocityFunctorMaterialTempl<is_ad>::getVelocityVariable(const std::string & param_name)
{
  auto * const var = dynamic_cast<VelocityVariable *>(
      &_fe_problem.getVariable(_tid, getParam<SolverVariableName>(param_name)));
  if (!var)
    paramError(param_name,
               is_ad ? "The velocity must be a finite volume field variable."
                     : "The velocity must be a MooseLinearVariableFVReal. Use "
                       "ADWCNSFV2PSlipVelocityFunctorMaterial for a nonlinear finite volume "
                       "velocity.");

  // The closure reads a gradient and, in a transient, a time derivative of the velocity. On the
  // nonlinear side both come from the functor interface; on the linear side the cell gradients
  // have to be requested, which the constructor does.
  if constexpr (is_ad)
    if (!dynamic_cast<const INSFVVelocityVariable *>(var) &&
        !dynamic_cast<const MooseLinearVariableFV<Real> *>(var))
      paramError(param_name,
                 "The velocity must be an INSFVVelocityVariable or a linear finite volume "
                 "variable.");

  return *var;
}

template <bool is_ad>
WCNSFV2PSlipVelocityFunctorMaterialTempl<is_ad>::WCNSFV2PSlipVelocityFunctorMaterialTempl(
    const InputParameters & params)
  : FunctorMaterial(params),
    _dim(_subproblem.mesh().dimension()),
    _u_var(&getVelocityVariable("u")),
    _v_var(_dim > 1 && isParamValid("v") ? &getVelocityVariable("v") : nullptr),
    _w_var(_dim > 2 && isParamValid("w") ? &getVelocityVariable("w") : nullptr),
    _rho_mixture(this->template getFunctor<GenericReal<is_ad>>(NS::density)),
    _rho_d(this->template getFunctor<GenericReal<is_ad>>(NS::dispersed_phase_density)),
    _mu_c(this->template getFunctor<GenericReal<is_ad>>(NS::mu)),
    _f_d(this->template getFunctor<GenericReal<is_ad>>("fraction_dispersed")),
    _gravity(getParam<RealVectorValue>("gravity")),
    _force_scale(getParam<Real>("force_value")),
    _force_function(getFunction("force_function")),
    _force_postprocessor(getPostprocessorValue("force_postprocessor")),
    _force_direction(getParam<RealVectorValue>("force_direction")),
    _linear_friction(isParamValid("linear_coef_name")
                         ? &this->template getFunctor<GenericReal<is_ad>>("linear_coef_name")
                         : nullptr),
    _rho_c(isParamValid(NS::continuous_phase_density)
               ? &this->template getFunctor<GenericReal<is_ad>>(NS::continuous_phase_density)
               : nullptr),
    _drag_model(getParam<MooseEnum>("drag_model").template getEnum<DragModelEnum>()),
    _sigma(isParamValid("surface_tension")
               ? &this->template getFunctor<GenericReal<is_ad>>("surface_tension")
               : nullptr),
    _swarm_exponent(getParam<Real>("swarm_exponent")),
    _friction_pressure_gradient(
        this->template getFunctor<GenericReal<is_ad>>("friction_pressure_gradient")),
    _single_particle_friction_pressure_gradient(this->template getFunctor<GenericReal<is_ad>>(
        "single_particle_friction_pressure_gradient")),
    _particle_diameter(this->template getFunctor<GenericReal<is_ad>>("particle_diameter")),
    _index(getParam<MooseEnum>("momentum_component"))
{
  const bool internal_drag = getParam<bool>("use_dispersed_phase_drag_model");
  if (internal_drag && _linear_friction)
    paramError("linear_coef_name",
               "A prescribed drag function cannot be combined with "
               "'use_dispersed_phase_drag_model', which computes the drag internally.");
  if (!internal_drag && !_linear_friction)
    paramError("linear_coef_name",
               "Either supply a drag function here, or set 'use_dispersed_phase_drag_model' to "
               "compute it from the Schiller and Naumann correlation.");
  if (internal_drag && !_rho_c)
    paramError("use_dispersed_phase_drag_model",
               "The continuous phase density 'rho_c' is required to form the particle Reynolds "
               "number of the drag correlation.");
  if (internal_drag && _drag_model != DragModelEnum::SCHILLER_NAUMANN)
  {
    if (!_sigma)
      paramError("surface_tension",
                 "The 'distorted-particle' and 'ishii-zuber' drag models are both set by the "
                 "balance of buoyancy against surface tension, so the surface tension is "
                 "required.");
    // Only the distorted particle law is scaled by the gravity magnitude itself. The Ishii and
    // Zuber relation drives its buoyancy from the full particle acceleration, so a case driven by
    // a body force rather than by gravity is legitimate for it.
    if (_gravity.norm() == 0 && _drag_model != DragModelEnum::ISHII_ZUBER)
      paramError("gravity",
                 "The 'distorted-particle' drag is scaled by g * delta_rho / sigma and is not "
                 "defined without gravity.");
  }

  // The phase fraction defaults to zero, which silently disables both of the mechanisms that
  // depend on it rather than failing, so require it wherever it actually carries the physics.
  if (!isParamSetByUser("fraction_dispersed"))
  {
    if (_swarm_exponent != 0.0)
      paramError("fraction_dispersed",
                 "A swarm exponent was supplied but the dispersed phase fraction was not. The "
                 "hindrance factor is (1 - alpha)^p, so without the phase fraction it is exactly "
                 "one and the swarm correction requested here would have no effect.");
    if (_drag_model == DragModelEnum::ISHII_ZUBER)
      paramError("fraction_dispersed",
                 "The 'ishii-zuber' drag model is a multi-particle correlation whose entire "
                 "concentration dependence comes from the dispersed phase fraction, so it must be "
                 "supplied. Without it the model returns the isolated particle result.");
  }

  if (_dim > 1 && !isParamValid("v"))
    paramError("v", "In two or more dimensions, the v velocity must be supplied.");
  if (_dim > 2 && !isParamValid("w"))
    paramError("w", "In three dimensions, the w velocity must be supplied.");

  // The advective part of the particle acceleration needs the velocity gradients
  // Cell gradients are a linear finite volume notion; the nonlinear variables build the
  // gradients the functor interface returns by another route
  if constexpr (!is_ad)
  {
    _u_var->computeCellGradients();
    if (_v_var)
      _v_var->computeCellGradients();
    if (_w_var)
      _w_var->computeCellGradients();
  }

  const auto & slip_velocity = this->template addFunctorProperty<GenericReal<is_ad>>(
      getParam<MooseFunctorName>("slip_velocity_name"),
      [this](const auto & r, const auto & t) -> GenericReal<is_ad>
      {
        // Guards the division below against a prescribed friction function evaluating to zero
        constexpr Real offset = 1e-15;

        // Pulled in so that the arithmetic below resolves for a plain double and for a
        // differentiated number alike
        using std::abs;
        using std::max;
        using std::min;
        using std::pow;
        using std::sqrt;

        GenericRealVectorValue<is_ad> term_advection(0, 0, 0);
        GenericRealVectorValue<is_ad> term_transient(0, 0, 0);
        const GenericRealVectorValue<is_ad> term_force(
            _force_scale * _force_postprocessor *
            _force_function.value(_t, _current_elem->vertex_average()) * _force_direction);

        // The time derivative is taken from the variable itself, which routes it through the
        // problem's time integrator, see MooseLinearVariableFV::evaluateDot. It needs a step of
        // non-zero length: the initial evaluation of a transient problem has no history to
        // difference, and the term is zero there.
        if (_subproblem.isTransient() && _dt > 0.0)
        {
          term_transient(0) = generic(_u_var->dot(r, t));
          if (_v_var)
            term_transient(1) = generic(_v_var->dot(r, t));
          if (_w_var)
            term_transient(2) = generic(_w_var->dot(r, t));
        }

        // Advective part of the material derivative, u . grad(u)
        const auto u_velocity = generic((*_u_var)(r, t));
        const auto u_grad = generic(_u_var->gradient(r, t));
        term_advection(0) += u_velocity * u_grad(0);
        if (_v_var)
        {
          const auto v_velocity = generic((*_v_var)(r, t));
          const auto v_grad = generic(_v_var->gradient(r, t));
          term_advection(0) += v_velocity * u_grad(1);
          term_advection(1) += u_velocity * v_grad(0) + v_velocity * v_grad(1);
          if (_w_var)
          {
            const auto w_velocity = generic((*_w_var)(r, t));
            const auto w_grad = generic(_w_var->gradient(r, t));
            term_advection(0) += w_velocity * u_grad(2);
            term_advection(1) += w_velocity * v_grad(2);
            term_advection(2) +=
                u_velocity * w_grad(0) + v_velocity * w_grad(1) + w_velocity * w_grad(2);
          }
        }

        // Manninen's algebraic slip closure, u_slip = tau_d / f_drag * (rho_d - rho_m)/rho_d * a,
        // with the particle relaxation time of Bilicki and Kestin. The buoyancy factor carries the
        // mixture density and the relaxation time the continuous phase viscosity, which is the
        // single particle form of the closure; every concentration effect is left to
        // swarm_exponent and to the ishii-zuber drag model.
        const GenericReal<is_ad> density_scaling =
            (_rho_d(r, t) - _rho_mixture(r, t)) / _rho_d(r, t);
        const GenericRealVectorValue<is_ad> acceleration_vec =
            -term_transient - term_advection + _gravity + term_force;

        const GenericReal<is_ad> relaxation_time =
            _rho_d(r, t) * Utility::pow<2>(_particle_diameter(r, t)) / (18.0 * _mu_c(r, t));

        // The slip in the Stokes limit, f_drag = 1, which is also the whole answer when the drag
        // function is prescribed rather than computed
        const GenericReal<is_ad> stokes_prefactor = relaxation_time * density_scaling;

        if (_linear_friction)
          return stokes_prefactor / ((*_linear_friction)(r, t) + offset) * acceleration_vec(_index);

        // Otherwise solve the force balance and the drag correlation together, so that the
        // particle Reynolds number is formed from the slip velocity as its definition requires
        // NS::computeSpeed rather than norm(): the derivative of a square root is unbounded at
        // zero, which a flow started from rest with no gravity reaches on its first residual
        // evaluation, and it substitutes a negligible magnitude there instead
        const GenericReal<is_ad> acceleration =
            NS::computeSpeed<GenericReal<is_ad>>(acceleration_vec);
        const GenericReal<is_ad> stokes_speed = abs(stokes_prefactor) * acceleration;
        const GenericReal<is_ad> rho_c = (*_rho_c)(r, t);
        const GenericReal<is_ad> mu_c = _mu_c(r, t);

        // The Ishii and Zuber multi-bubble relation is a correlation for the relative velocity
        // itself, not a drag law, so it replaces the force balance rather than closing it. It also
        // carries its own concentration dependence, which is what the swarm exponent stands in for
        // elsewhere, so no hindrance is applied on top of it.
        if (_drag_model == DragModelEnum::ISHII_ZUBER)
        {
          const GenericReal<is_ad> alpha =
              min(max(_f_d(r, t), GenericReal<is_ad>(0.0)), GenericReal<is_ad>(1.0));
          const GenericReal<is_ad> buoyancy = abs(rho_c - _rho_d(r, t)) * acceleration;
          const GenericReal<is_ad> speed =
              ishiiZuberSlipSpeed(alpha,
                                  buoyancy,
                                  _friction_pressure_gradient(r, t),
                                  _single_particle_friction_pressure_gradient(r, t),
                                  (*_sigma)(r, t),
                                  rho_c);
          // The correlation returns a magnitude. Its direction is that of the buoyancy, which is
          // the direction of the acceleration carrying the sign of rho_d - rho_m: a light dispersed
          // phase moves against the acceleration, a heavy one with it.
          if (MooseUtils::isZero(acceleration_vec))
            return 0.0;
          return speed * (MetaPhysicL::raw_value(density_scaling) < 0.0 ? -1.0 : 1.0) *
                 acceleration_vec(_index) / acceleration;
        }

        // Rigid sphere branch: s f(R s) = s0 with f monotone and at least unity
        GenericReal<is_ad> slip_speed = std::numeric_limits<Real>::max();
        if (_drag_model != DragModelEnum::DISTORTED_PARTICLE)
        {
          const GenericReal<is_ad> reynolds_per_speed = HeatTransferUtils::reynolds(
              rho_c, GenericReal<is_ad>(1.0), _particle_diameter(r, t), mu_c);
          slip_speed = solveSlipSpeed(stokes_speed, reynolds_per_speed);
        }

        // Distorted particle branch. Its drag function is linear in the slip speed, so the force
        // balance k s^2 = s0 has the closed form s = sqrt(s0 / k) and needs no iteration.
        if (_drag_model != DragModelEnum::SCHILLER_NAUMANN)
        {
          const GenericReal<is_ad> k =
              distortedDragFunctionPerSpeed(GenericReal<is_ad>(_particle_diameter(r, t)),
                                            GenericReal<is_ad>(rho_c),
                                            GenericReal<is_ad>(mu_c),
                                            GenericReal<is_ad>(abs(rho_c - _rho_d(r, t))),
                                            GenericReal<is_ad>((*_sigma)(r, t)),
                                            _gravity.norm());
          const GenericReal<is_ad> distorted_speed =
              (MetaPhysicL::raw_value(k) > 0.0) ? sqrt(stokes_speed / k) : stokes_speed;
          // A larger drag function gives a smaller slip, so taking the more resistant of the two
          // laws is the same as taking the smaller of the two speeds
          slip_speed = min(slip_speed, distorted_speed);
        }

        // Recover the drag function the chosen speed implies, and apply the closure with it. This
        // keeps a single expression for the returned component whichever branch was taken.
        const GenericReal<is_ad> f_drag = (slip_speed > 0.0) ? stokes_speed / slip_speed : 1.0;

        // Hindrance of the swarm. Applied to the speed rather than to the drag function because
        // the two branches respond differently to a factor on the drag, being respectively linear
        // and square-root in it, whereas a factor on the speed means the same thing for both.
        const GenericReal<is_ad> hindrance =
            (_swarm_exponent == 0.0) ? 1.0
                                     : pow(max(1.0 - min(max(_f_d(r, t), GenericReal<is_ad>(0.0)),
                                                         GenericReal<is_ad>(1.0)),
                                               0.0),
                                           _swarm_exponent);

        return stokes_prefactor / f_drag * hindrance * acceleration_vec(_index);
      });

  // The diffusion (drift) velocity, the velocity of the dispersed phase relative to the centre of
  // mass of the mixture. This, not the slip velocity, is the velocity that appears in the phase
  // conservation equations; see Manninen, Taivassalo and Kallio, "On the mixture model for
  // multiphase flow", VTT Publications 288 (1996).
  if (isParamValid("drift_velocity_name"))
    this->template addFunctorProperty<GenericReal<is_ad>>(
        getParam<MooseFunctorName>("drift_velocity_name"),
        [this, &slip_velocity](const auto & r, const auto & t) -> GenericReal<is_ad>
        { return this->diffusionVelocityFactor(r, t) * slip_velocity(r, t); });
}

template <bool is_ad>
GenericReal<is_ad>
WCNSFV2PSlipVelocityFunctorMaterialTempl<is_ad>::ishiiZuberSlipSpeed(
    const GenericReal<is_ad> alpha,
    const GenericReal<is_ad> buoyancy,
    const GenericReal<is_ad> m_f,
    const GenericReal<is_ad> m_f_inf,
    const GenericReal<is_ad> sigma,
    const GenericReal<is_ad> rho_c)
{
  const GenericReal<is_ad> one_minus = std::max(1.0 - alpha, GenericReal<is_ad>(0.0));
  const GenericReal<is_ad> denominator = buoyancy + m_f_inf;
  if (MetaPhysicL::raw_value(denominator) <= 0.0 || MetaPhysicL::raw_value(sigma) <= 0.0 ||
      MetaPhysicL::raw_value(rho_c) <= 0.0 || MetaPhysicL::raw_value(one_minus) <= 0.0)
    return 0.0;

  // This form is taken from Hibiki and Ishii (2003), cited in full in the header.
  //
  // The terminal velocity of an isolated distorted particle. The frictional pressure
  // gradient of the single particle system adds to the buoyancy rather than hindering it.
  const GenericReal<is_ad> v_r_inf =
      sqrt(2.0) * pow(denominator * sigma / Utility::pow<2>(rho_c), 0.25);

  // The concentration ratio, through which the frictional pressure gradient of the two phase
  // flow enters. With both gradients zero it is simply 1 - alpha.
  const GenericReal<is_ad> ratio =
      std::max((buoyancy * one_minus + m_f) / denominator, GenericReal<is_ad>(0.0));

  // The concentration function, with the viscosity ratio mu_f / mu_m = 1 - alpha
  const GenericReal<is_ad> f_alpha = one_minus * sqrt(ratio);

  // The multi-particle relative velocity. The constants are those of the Newton
  // regime drag ratio of Ishii and Zuber (1979), arranged so that the concentration factor is
  // unity at alpha = 0.
  const GenericReal<is_ad> concentration =
      18.67 * f_alpha / (1.0 + 17.67 * pow(f_alpha, 6.0 / 7.0));

  // The correlation gives the drift velocity; the relative velocity carries one power of
  // (1 - alpha) less
  return v_r_inf * sqrt(one_minus) * concentration;
}

template class WCNSFV2PSlipVelocityFunctorMaterialTempl<false>;
template class WCNSFV2PSlipVelocityFunctorMaterialTempl<true>;
