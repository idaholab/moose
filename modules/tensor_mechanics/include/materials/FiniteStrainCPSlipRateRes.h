//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FiniteStrainCrystalPlasticity.h"

class FiniteStrainCPSlipRateRes : public FiniteStrainCrystalPlasticity
{
public:
  static InputParameters validParams();

  FiniteStrainCPSlipRateRes(const InputParameters & parameters);

protected:
  /**
   * This function solves internal variables.
   */
  virtual void solveStatevar();

  /**
   * This function sets variable for internal variable solve.
   */
  virtual void preSolveStress();

  /**
   * This function solves for stress, updates plastic deformation gradient.
   */
  virtual void solveStress();

  /**
   * This function calculates residual and jacobian of slip rate.
   */
  virtual void calcResidJacobSlipRate();

  /**
   * This function calculates residual of slip rate.
   */
  virtual void calcResidualSlipRate();

  /**
   * This function calculates jacobian of slip rate.
   */
  virtual void calcJacobianSlipRate();

  /**
   * This function updates the slip system resistances.
   */
  virtual void getSlipIncrements();

  /**
   * This function calculates partial derivative of resolved shear stress with respect to split
   * rate.
   */
  virtual void calcDtauDsliprate();

  /**
   * This function calculates partial derivative of slip system resistances with respect to split
   * rate.
   */
  virtual void calcDgssDsliprate();

  /**
   * This function calculates and updates the residual of slip rate.
   */
  void calcUpdate();

  /**
   * This function calculates the residual norm.
   */
  virtual Real calcResidNorm();

  /**
   * This function performs the line search update.
   */
  bool lineSearchUpdateSlipRate(const Real, const DenseVector<Real> &);

  /**
   * This function calculates the dot product of residual and update
   */
  Real calcResidDotProdUpdate(const DenseVector<Real> &);

  DenseVector<Real> _resid;
  DenseVector<Real> _slip_rate;
  DenseVector<Real> _dsliprate_dgss;
  DenseMatrix<Real> _jacob;
  DenseMatrix<Real> _dsliprate_dsliprate;
};
