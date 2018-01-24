//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef RATEDEPSMEARISOCRACKMODEL_H
#define RATEDEPSMEARISOCRACKMODEL_H

#include "RateDepSmearCrackModel.h"

class RateDepSmearIsoCrackModel;

template <>
InputParameters validParams<RateDepSmearIsoCrackModel>();

/**
 * In this class a rate dependent isotropic damage model is implemented
 */

class RateDepSmearIsoCrackModel : public RateDepSmearCrackModel
{
public:
  RateDepSmearIsoCrackModel(const InputParameters & parameters);

  virtual ~RateDepSmearIsoCrackModel();

protected:
  virtual void initQpStatefulProperties();
  virtual void initVariables();
  /**
   * This function calculates rate of damage based on energy
   */
  virtual Real damageRate();
  virtual void calcStateIncr();
  virtual void calcJacobian();
  virtual void postSolveStress();

  Real _crit_energy; ///Required parameter
  Real _kfail;       ///Used to avoid non-positive definiteness
  Real _upper_lim_damage;

  MaterialProperty<Real> & _energy;
  const MaterialProperty<Real> & _energy_old;

  Real _ddamage;
  Real _ddamagerate_drs;

  ColumnMajorMatrix _s0_diag, _s0_diag_pos, _s0_diag_neg, _s0_evec;
  ColumnMajorMatrix _dstrain_diag, _dstrain_diag_pos, _dstrain_diag_neg, _dstrain_evec;

private:
};

#endif // RATEDEPSMEARCRACKMODEL
