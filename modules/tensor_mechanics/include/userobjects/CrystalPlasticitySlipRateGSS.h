//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CrystalPlasticitySlipRate.h"
#include "RankTwoTensor.h"

/**
 * Phenomenological constitutive model slip rate userobject class.
 */
class CrystalPlasticitySlipRateGSS : public CrystalPlasticitySlipRate
{
public:
  static InputParameters validParams();

  CrystalPlasticitySlipRateGSS(const InputParameters & parameters);

  virtual bool calcSlipRate(unsigned int qp, Real dt, std::vector<Real> & val) const;
  virtual bool calcSlipRateDerivative(unsigned int qp, Real /*dt*/, std::vector<Real> & val) const;
  virtual void calcFlowDirection(unsigned int qp,
                                 std::vector<RankTwoTensor> & flow_direction) const;

protected:
  virtual void readFileFlowRateParams();
  virtual void getFlowRateParams();

  const MaterialProperty<std::vector<Real>> & _mat_prop_state_var;

  const MaterialProperty<RankTwoTensor> & _pk2;

  DenseVector<Real> _a0;
  DenseVector<Real> _xm;

  const MaterialProperty<std::vector<RankTwoTensor>> & _flow_direction;
};
