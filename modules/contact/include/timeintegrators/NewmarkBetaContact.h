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
  const NumericVector<Number> & _contact_residual;
  const TagID _noncontact_tag_id;
  const NumericVector<Number> & _noncontact_residual;

  /// The fraction of the current noncontact forces to use in the current residual evaluation. The
  /// remaining fraction will come from the previous time-steps noncontact forces
  const Real _implicit_u_fraction;

  /// The fraction of the current noncontact forces to use in the current velocity/u_dot
  /// evaluation. The remaining fraction will come from the previous time-steps noncontact forces
  const Real _implicit_udot_fraction;

  NumericVector<Number> & _noncontact_old;
};
