//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NewmarkBeta.h"
#include <set>

#include "libmesh/numeric_vector.h"

class NewmarkBetaContact;
/**
 * Contact-implicit Newmark scheme
 */
class NewmarkBetaContact : public NewmarkBeta
{
public:
  static InputParameters validParams();

  NewmarkBetaContact(const InputParameters & parameters);
  void postResidual(NumericVector<Number> & residual) override;
  void init() override;
  void postStep() override;
  void computeTimeDerivatives() override;
  void computeADTimeDerivatives(ADReal &, const dof_id_type &, ADReal &) const override;

private:
  template <typename T>
  void changeNonContactTags(T & warehouse);

  std::set<std::string> _disp;
  const TagID _contact_tag_id;
  const NumericVector<Number> * _contact_residual;
  const TagID _noncontact_tag_id;
  const NumericVector<Number> * _noncontact_residual;
  /// Tag for computing the mass matrix
  const TagID _system_time_tag;
  /// Diagonal of the lumped mass matrix (and its inversion)
  NumericVector<Real> * _mass_matrix_diag;

  /// Vector of 1's to help with creating the lumped mass matrix
  NumericVector<Real> * _ones;

  /// The fraction of the current noncontact forces to use in the current residual evaluation. The
  /// remaining fraction will come from the previous time-steps noncontact forces
  const Real _implicit_u_fraction;

  NumericVector<Number> * _noncontact_old;
  NumericVector<Number> * _u_dotdot_contact;

  NumericVector<Number> * _u_dotdot_internal;
  NumericVector<Number> * _u_dotdot_internal_old;

  virtual void computeContactAccelerations();
  virtual void addTimeIntegratorVectors(const bool is_nonlinear_system = true) override;

  /// Helper function that actually does the math for computing the time derivative
  template <typename T,
            typename T2,
            typename T3,
            typename T4,
            typename T5,
            typename T6,
            typename T7,
            typename T8>
  void computeTimeDerivativeHelper(T & u_dot,
                                   const T2 & u_old,
                                   const T3 & u_dot_old,
                                   T4 & u_dotdot,
                                   const T5 & u_dotdot_old,
                                   const T6 & u_dotdot_contact,
                                   T7 & u_dotdot_internal,
                                   const T8 & u_dotdot_internal_old);
};

template <typename T,
          typename T2,
          typename T3,
          typename T4,
          typename T5,
          typename T6,
          typename T7,
          typename T8>
void
NewmarkBetaContact::computeTimeDerivativeHelper(T & u_dot,
                                                const T2 & u_old,
                                                const T3 & u_dot_old,
                                                T4 & u_dotdot,
                                                const T5 & /*u_dotdot_old*/,
                                                const T6 & u_dotdot_contact,
                                                T7 & u_dotdot_internal,
                                                const T8 & u_dotdot_internal_old)
{
  // Compute internal accelerations (i.e. non-contact accelerations)
  //  MathUtils::addScaled(2.0 / (_dt * _dt), u_dotdot, u_dotdot_internal);
  //  MathUtils::addScaled(-2.0 / (_dt * _dt), u_old, u_dotdot_internal);
  //  MathUtils::addScaled(-1.0, u_dotdot_contact, u_dotdot_internal);
  //  u_dotdot_internal /= 2.0 * _beta;

  // compute second derivative
  // according to Newmark-Beta-Contact method
  // u_dotdot = first_term - second_term - third_term
  //       first_term = (u - u_old) / beta / dt ^ 2
  //      second_term = u_dot_old / beta / dt
  //       third_term = _u_dotdot_internal_old * (1 / 2 / beta - 1)
  u_dotdot -= u_old;
  u_dotdot *= 1.0 / _beta / _dt / _dt;
  MathUtils::addScaled(-1.0 / _beta / _dt, u_dot_old, u_dotdot);
  MathUtils::addScaled(-0.5 / _beta + 1.0, u_dotdot_internal_old, u_dotdot);
  MathUtils::addScaled(-1.0 / 2.0 / _beta, u_dotdot_contact, u_dotdot);

  u_dotdot_internal = u_dotdot;
  u_dotdot += u_dotdot_contact;

  // compute first derivative
  // according to Newmark-Beta-Contact method
  // u_dot = first_term + second_term + third_term
  //       first_term = u_dot_old
  //      second_term = u_dotdot_internal_old * (1 - gamma) * dt
  //       third_term = u_dotdot_internal * gamma * dt
  //        four_term = u_dotdot_contact * dt
  u_dot = u_dot_old;
  MathUtils::addScaled((1.0 - _gamma) * _dt, u_dotdot_internal_old, u_dot);
  MathUtils::addScaled(_gamma * _dt, u_dotdot_internal, u_dot);
  MathUtils::addScaled(_dt, u_dotdot_contact, u_dot);
}
