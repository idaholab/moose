//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
// Original class author: A.M. Jokisaari,  O. Heinonen

#include "ComputeStressBase.h"

/**
 * FiniteStrainPlasticMaterial implements rate-independent associative J2 plasticity
 * with isotropic hardening in the finite-strain framework.
 * Yield function = sqrt(3*s_ij*s_ij/2) - K(equivalent plastic strain)
 * where s_ij = stress_ij - delta_ij*trace(stress)/3 is the deviatoric stress
 * and K is the yield stress, specified as a piecewise-linear function by the user.
 * Integration is performed in an incremental manner using Newton Raphson.
 */
class FiniteStrainPlasticMaterial : public ComputeStressBase
{
public:
  static InputParameters validParams();

  FiniteStrainPlasticMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpStress();
  virtual void initQpStatefulProperties();
  std::vector<Real> _yield_stress_vector;
  MaterialProperty<RankTwoTensor> & _plastic_strain;
  const MaterialProperty<RankTwoTensor> & _plastic_strain_old;
  MaterialProperty<Real> & _eqv_plastic_strain;
  const MaterialProperty<Real> & _eqv_plastic_strain_old;
  /// The old stress tensor
  const MaterialProperty<RankTwoTensor> & _stress_old;
  const MaterialProperty<RankTwoTensor> & _strain_increment;
  const MaterialProperty<RankTwoTensor> & _rotation_increment;
  /// Name of the elasticity tensor material property
  const std::string _elasticity_tensor_name;
  /// Elasticity tensor material property
  const MaterialProperty<RankFourTensor> & _elasticity_tensor;
  Real _rtol;
  Real _ftol;
  Real _eptol;

  // outer and mixed product of the delta function tensors
  RankFourTensor _deltaOuter, _deltaMixed;

  /**
   * Implements the return map
   * @param sig_old  The stress at the previous "time" step
   * @param eqvpstrain_old  The equivalent plastic strain at the previous "time" step
   * @param plastic_strain_old  The value of plastic strain at the previous "time" step
   * @param delta_d  The total strain increment for this "time" step
   * @param E_ijkl   The elasticity tensor.  If no plasticity then sig_new = sig_old +
   * E_ijkl*delta_d
   * @param sig      The stress after returning to the yield surface   (this is an output variable)
   * @param eqvpstrain  The equivalent plastic strain after returning to the yield surface (this is
   * an output variable)
   * @param plastic_strain   The value of plastic strain after returning to the yield surface (this
   * is an output variable)
   * Note that this algorithm doesn't do any rotations.  In order to find the
   * final stress and plastic_strain, sig and plastic_strain must be rotated using
   * _rotation_increment.
   */
  virtual void returnMap(const RankTwoTensor & sig_old,
                         const Real eqvpstrain_old,
                         const RankTwoTensor & plastic_strain_old,
                         const RankTwoTensor & delta_d,
                         const RankFourTensor & E_ijkl,
                         RankTwoTensor & sig,
                         Real & eqvpstrain,
                         RankTwoTensor & plastic_strain);

  /**
   * Calculates the yield function
   * @param stress the stress at which to calculate the yield function
   * @param yield_stress the current value of the yield stress
   * @return equivalentstress - yield_stress
   */
  virtual Real yieldFunction(const RankTwoTensor & stress, const Real yield_stress);

  /**
   * Derivative of yieldFunction with respect to the stress
   */
  virtual RankTwoTensor dyieldFunction_dstress(const RankTwoTensor & stress);

  /**
   * Derivative of yieldFunction with respect to the equivalent plastic strain
   */
  virtual Real dyieldFunction_dinternal(const Real equivalent_plastic_strain);

  /**
   * Flow potential, which in this case is just dyieldFunction_dstress
   * because we are doing associative flow, and hence does not depend
   * on the internal hardening parameter equivalent_plastic_strain
   */
  virtual RankTwoTensor flowPotential(const RankTwoTensor & stress);

  /**
   * The internal potential.  For associative J2 plasticity this is just -1
   */
  virtual Real internalPotential();

  /**
   * Equivalent stress
   */
  Real getSigEqv(const RankTwoTensor & stress);

  /**
   * Evaluates the derivative d(resid_ij)/d(sig_kl), where
   * resid_ij = flow_incr*flowPotential_ij - (E^{-1}(trial_stress - sig))_ij
   * @param sig stress
   * @param E_ijkl elasticity tensor (sig = E*(strain - plastic_strain))
   * @param flow_incr consistency parameter
   * @param dresid_dsig the required derivative (this is an output variable)
   */
  virtual void getJac(const RankTwoTensor & sig,
                      const RankFourTensor & E_ijkl,
                      Real flow_incr,
                      RankFourTensor & dresid_dsig);

  /**
   * yield stress as a function of equivalent plastic strain.
   * This is a piecewise linear function entered by the user in the yield_stress vector
   */
  Real getYieldStress(const Real equivalent_plastic_strain);

  /**
   * d(yieldstress)/d(equivalent plastic strain)
   */
  Real getdYieldStressdPlasticStrain(const Real equivalent_plastic_strain);

  /// make i,j,k,l available as tensor indices for mixedProduct
  usingTensorIndices(i_, j_, k_, l_);
};
