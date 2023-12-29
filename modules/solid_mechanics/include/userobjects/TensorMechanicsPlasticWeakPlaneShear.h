//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TensorMechanicsPlasticModel.h"
#include "TensorMechanicsHardeningModel.h"

/**
 * Rate-independent associative weak-plane tensile failure
 * with hardening/softening.  The cone's tip is smoothed.
 */
class TensorMechanicsPlasticWeakPlaneShear : public TensorMechanicsPlasticModel
{
public:
  static InputParameters validParams();

  TensorMechanicsPlasticWeakPlaneShear(const InputParameters & parameters);

  virtual void activeConstraints(const std::vector<Real> & f,
                                 const RankTwoTensor & stress,
                                 Real intnl,
                                 const RankFourTensor & Eijkl,
                                 std::vector<bool> & act,
                                 RankTwoTensor & returned_stress) const override;

  virtual std::string modelName() const override;

protected:
  /// Hardening model for cohesion
  const TensorMechanicsHardeningModel & _cohesion;

  /// Hardening model for tan(phi)
  const TensorMechanicsHardeningModel & _tan_phi;

  /// Hardening model for tan(psi)
  const TensorMechanicsHardeningModel & _tan_psi;

  Real yieldFunction(const RankTwoTensor & stress, Real intnl) const override;

  RankTwoTensor dyieldFunction_dstress(const RankTwoTensor & stress, Real intnl) const override;

  Real dyieldFunction_dintnl(const RankTwoTensor & stress, Real intnl) const override;

  RankTwoTensor flowPotential(const RankTwoTensor & stress, Real intnl) const override;

  RankFourTensor dflowPotential_dstress(const RankTwoTensor & stress, Real intnl) const override;

  RankTwoTensor dflowPotential_dintnl(const RankTwoTensor & stress, Real intnl) const override;

  /**
   * The yield function is modified to
   * f = sqrt(s_xz^2 + s_yz^2 + a) + s_zz*_tan_phi - _cohesion
   * where "a" depends on the tip_scheme.  Currently _tip_scheme is
   * 'hyperbolic', where a = _small_smoother2
   * 'cap' where a = _small_smoother2 + (p(stress(2,2) - _cap_start))^2
   *       with the function p(x)=x(1-exp(-_cap_rate*x)) for x>0, and p=0 otherwise
   */
  MooseEnum _tip_scheme;

  /// smoothing parameter for the cone's tip - see doco for _tip_scheme
  Real _small_smoother2;

  /// smoothing parameter dictating when the 'cap' will start - see doco for _tip_scheme
  Real _cap_start;

  /// dictates how quickly the 'cap' degenerates to a hemisphere - see doco for _tip_scheme
  Real _cap_rate;

  /// Function that's used in dyieldFunction_dstress and flowPotential
  RankTwoTensor df_dsig(const RankTwoTensor & stress, Real _tan_phi_or_psi) const;

  /// returns the 'a' parameter - see doco for _tip_scheme
  virtual Real smooth(const RankTwoTensor & stress) const;

  /// returns the da/dstress(2,2) - see doco for _tip_scheme
  virtual Real dsmooth(const RankTwoTensor & stress) const;

  /// returns the d^2a/dstress(2,2)^2 - see doco for _tip_scheme
  virtual Real d2smooth(const RankTwoTensor & stress) const;

  /// cohesion as a function of internal parameter
  virtual Real cohesion(const Real internal_param) const;

  /// d(cohesion)/d(internal_param)
  virtual Real dcohesion(const Real internal_param) const;

  /// tan_phi as a function of internal parameter
  virtual Real tan_phi(const Real internal_param) const;

  /// d(tan_phi)/d(internal_param);
  virtual Real dtan_phi(const Real internal_param) const;

  /// tan_psi as a function of internal parameter
  virtual Real tan_psi(const Real internal_param) const;

  /// d(tan_psi)/d(internal_param);
  virtual Real dtan_psi(const Real internal_param) const;
};
