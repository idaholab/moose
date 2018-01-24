/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CRYSTALPLASTICITYSLIPRATEGSS_H
#define CRYSTALPLASTICITYSLIPRATEGSS_H

#include "CrystalPlasticitySlipRate.h"
#include "RankTwoTensor.h"

class CrystalPlasticitySlipRateGSS;

template <>
InputParameters validParams<CrystalPlasticitySlipRateGSS>();

/**
 * Phenomenological constitutive model slip rate userobject class.
 */
class CrystalPlasticitySlipRateGSS : public CrystalPlasticitySlipRate
{
public:
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

#endif // CRYSTALPLASTICITYSLIPRATEGSS_H
