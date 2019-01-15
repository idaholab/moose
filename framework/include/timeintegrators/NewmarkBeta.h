//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NEWMARKBETA_H
#define NEWMARKBETA_H

#include "TimeIntegrator.h"

class NewmarkBeta;

template <>
InputParameters validParams<NewmarkBeta>();

/**
 * Newmark-Beta time integration method
 */
class NewmarkBeta : public TimeIntegrator
{
public:
  NewmarkBeta(const InputParameters & parameters);
  virtual ~NewmarkBeta();

  virtual int order() override { return 1; }
  virtual void computeTimeDerivatives() override;
  virtual void computeADTimeDerivatives(DualReal & ad_u_dot, const dof_id_type & dof) override;
  virtual void postResidual(NumericVector<Number> & residual) override;

protected:
  /// Newmark time integration parameter-beta
  Real _beta;

  /// Newmark time integration parameter-gamma
  Real _gamma;

  /// solution vector for \f$ {du^dotdot}\over{du} \f$
  Real & _du_dotdot_du;
};
#endif /* NEWMARKBETA_H */
