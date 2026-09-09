//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctorMaterial.h"
#include "MooseLinearVariableFV.h"

#include <algorithm>
#include <type_traits>

class Function;

/**
 * Computes the slip velocity of the dispersed phase relative to the continuous phase for the
 * two-phase mixture model, and optionally the diffusion (drift) velocity derived from it,
 * \f$ u_{Md} = (1 - c_d) u_s \f$ with \f$ c_d = \alpha \rho_d / \rho_m \f$. The two velocities, and
 * the closures that produce the slip, are described on the documentation page.
 *
 * One implementation serves both discretizations, in the manner of
 * NSFVMixtureFunctorMaterialTempl: the template parameter selects the scalar type the closure is
 * evaluated in, plain for the linear finite volume systems and automatically differentiated for
 * the nonlinear ones, and with it the velocity variable type.
 */
template <bool is_ad>
class WCNSFV2PSlipVelocityFunctorMaterialTempl : public FunctorMaterial
{
public:
  static InputParameters validParams();
  WCNSFV2PSlipVelocityFunctorMaterialTempl(const InputParameters & parameters);

  /// The velocity variables this closure reads. The linear finite volume instantiation holds the
  /// linear variable directly; the automatic differentiation instantiation takes the field base the
  /// nonlinear finite volume variables derive from.
  using VelocityVariable =
      std::conditional_t<is_ad, MooseVariableField<Real>, MooseLinearVariableFVReal>;

protected:
  /// Which drag law closes the force balance
  enum class DragModelEnum
  {
    /// Rigid sphere: Schiller and Naumann below the transition, Newton above
    SCHILLER_NAUMANN = 0,
    /// Distorted fluid particle: C_D grows with size, terminal velocity independent of it
    DISTORTED_PARTICLE = 1,
    /// Whichever of the two resists more, which selects the regime automatically
    AUTOMATIC = 2,
    /// The multi-bubble distorted particle relation of Ishii and Zuber (1979), as written in
    /// equation (57) of Hibiki and Ishii (2003); both papers are cited in full on
    /// ishiiZuberSlipSpeed below
    ISHII_ZUBER = 3
  };

  /// Whether the functors this object fetches are evaluated with derivatives, which the scalar
  /// type decides. FunctorMaterial reports itself as differentiated; the plain instantiation is
  /// not, and would otherwise be handed the derivative view of a variable.
  bool isADObject() const override { return is_ad; }

  /// The velocity variables return a differentiated value whichever discretization they belong
  /// to. The automatic differentiation instantiation keeps the derivatives; the plain one drops
  /// them here, which is where the two paths legitimately differ.
  template <typename T>
  static auto generic(const T & value)
  {
    if constexpr (is_ad)
      return value;
    else
      return MetaPhysicL::raw_value(value);
  }

  /// Retrieve a velocity variable and check that it is of the type this instantiation needs
  VelocityVariable & getVelocityVariable(const std::string & param_name);

  /**
   * The factor converting the slip velocity into the diffusion velocity, \f$ 1 - c_d \f$, written
   * as \f$ (\rho_m - \alpha \rho_d) / \rho_m \f$. The phase fraction is clamped into [0, 1], as
   * the mixture property material clamps it.
   */
  template <typename SpaceArg, typename StateArg>
  GenericReal<is_ad> diffusionVelocityFactor(const SpaceArg & r, const StateArg & t) const
  {
    const auto rho_m = _rho_mixture(r, t);
    if (MetaPhysicL::raw_value(rho_m) <= 0.0)
      return 1.0;
    const auto fd =
        std::min(std::max(_f_d(r, t), GenericReal<is_ad>(0.0)), GenericReal<is_ad>(1.0));
    return (rho_m - fd * _rho_d(r, t)) / rho_m;
  }

  /**
   * The factor converting the slip velocity into the volumetric drift, \f$ \alpha - c_d \f$, the
   * difference between the volume averaged and the mass averaged mixture velocity.
   */
  template <typename SpaceArg, typename StateArg>
  GenericReal<is_ad> volumetricDriftFactor(const SpaceArg & r, const StateArg & t) const
  {
    const auto rho_m = _rho_mixture(r, t);
    if (MetaPhysicL::raw_value(rho_m) <= 0.0)
      return 0.0;
    const auto fd =
        std::min(std::max(_f_d(r, t), GenericReal<is_ad>(0.0)), GenericReal<is_ad>(1.0));
    return fd * (rho_m - _rho_d(r, t)) / rho_m;
  }

  /// Dimension of the domain
  const unsigned int _dim;

  /// Velocity in the x direction
  VelocityVariable * const _u_var;
  /// Velocity in the y direction
  VelocityVariable * const _v_var;
  /// Velocity in the z direction
  VelocityVariable * const _w_var;

  /// Mixture density
  const Moose::Functor<GenericReal<is_ad>> & _rho_mixture;
  /// Dispersed phase density
  const Moose::Functor<GenericReal<is_ad>> & _rho_d;
  /// Continuous phase dynamic viscosity. Both the particle relaxation time and the particle
  /// Reynolds number are formed from it; see the note in validParams on why this is not the
  /// mixture viscosity
  const Moose::Functor<GenericReal<is_ad>> & _mu_c;

  /// Volume fraction of the dispersed phase, needed to convert the slip velocity into the
  /// diffusion velocity
  const Moose::Functor<GenericReal<is_ad>> & _f_d;

  /// Gravity acceleration vector
  const RealVectorValue _gravity;
  /// Body force scaling
  const Real _force_scale;
  /// Body force function
  const Function & _force_function;
  /// Body force postprocessor
  const PostprocessorValue & _force_postprocessor;
  /// Body force direction
  const RealVectorValue _force_direction;

  /// Prescribed linear drag function. Null when the drag is computed internally from the
  /// Schiller and Naumann correlation, see NS::solveSlipSpeed
  const Moose::Functor<GenericReal<is_ad>> * const _linear_friction;

  /// Continuous phase density, used to form the particle Reynolds number
  const Moose::Functor<GenericReal<is_ad>> * const _rho_c;

  /// Which drag law to close the force balance with
  const DragModelEnum _drag_model;

  /// Surface tension, needed by the distorted particle drag
  const Moose::Functor<GenericReal<is_ad>> * const _sigma;

  /// Exponent p of the hindrance factor (1 - alpha)^p applied to the slip velocity. Zero leaves
  /// the single-particle result untouched
  const Real _swarm_exponent;

  /// Frictional pressure gradients of the two phase and single particle systems, M_F and M_Finf,
  /// equations (41) and (24) of the Hibiki and Ishii paper cited on ishiiZuberSlipSpeed below.
  /// Only the 'ishii-zuber' drag model uses them.
  const Moose::Functor<GenericReal<is_ad>> & _friction_pressure_gradient;
  const Moose::Functor<GenericReal<is_ad>> & _single_particle_friction_pressure_gradient;

  /**
   * The relative velocity of the distorted particle multi-bubble system, equations (45) to (50) of
   *
   *   Hibiki, T. and Ishii, M., "One-dimensional drift-flux model and constitutive equations for
   *   relative motion between phases in various two-phase flow regimes", International Journal of
   *   Heat and Mass Transfer 46 (2003) 4935-4948,
   *
   * whose underlying drag correlation is that of Ishii and Zuber, AIChE Journal 25 (1979) 843-855.
   * Returns the magnitude of the slip.
   *
   * @param alpha volume fraction of the dispersed phase
   * @param buoyancy the driving term, |rho_c - rho_d| times the acceleration, standing for the
   *        Delta rho g_z of the reference
   * @param m_f the frictional pressure gradient of the two phase flow
   * @param m_f_inf the frictional pressure gradient of the single particle system
   * @param sigma surface tension
   * @param rho_c density of the continuous phase
   */
  static GenericReal<is_ad> ishiiZuberSlipSpeed(const GenericReal<is_ad> alpha,
                                                const GenericReal<is_ad> buoyancy,
                                                const GenericReal<is_ad> m_f,
                                                const GenericReal<is_ad> m_f_inf,
                                                const GenericReal<is_ad> sigma,
                                                const GenericReal<is_ad> rho_c);
  /// Particle diameter in the dispersed phase
  const Moose::Functor<GenericReal<is_ad>> & _particle_diameter;

  /// Index of the velocity component x|y|z
  const unsigned int _index;
};

typedef WCNSFV2PSlipVelocityFunctorMaterialTempl<false> LinearWCNSFV2PSlipVelocityFunctorMaterial;
typedef WCNSFV2PSlipVelocityFunctorMaterialTempl<true> WCNSFV2PSlipVelocityFunctorMaterial;
