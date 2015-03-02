/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef TENSORMECHANICSPLASTICMODEL_H
#define TENSORMECHANICSPLASTICMODEL_H

#include "GeneralUserObject.h"
#include "RankTwoTensor.h"

class TensorMechanicsPlasticModel;


template<>
InputParameters validParams<TensorMechanicsPlasticModel>();

/**
 * Plastic Model base class
 * The virtual functions written below must be
 * over-ridden in derived classes to provide actual values
 *
 * It is assumed there is only one internal parameter, and
 * that is a function of the plastic multiplier, with rate
 * given by hardPotential
 *
 * For better or worse, I have created two versions of
 * all functions (eg yieldFunction, flowPotential, etc).
 * This is so that for single-surface plasticity you can
 * just override the 'protected' functions:
 * Real yieldFunction(stress, intnl)
 * (and similar), and don't have to worry about all the
 * multi-surface stuff, since in multi-surface yieldFunction
 * (etc) return std::vectors of stuff.  In the case of
 * multi-surface plasticity models you DO need to override the
 * 'public' functions (with a 'V' in their name):
 * void yieldFunctionV(stress, intnl, f)
 * versions.
 */
class TensorMechanicsPlasticModel : public GeneralUserObject
{
 public:
  TensorMechanicsPlasticModel(const std::string & name, InputParameters parameters);

  void initialize();
  void execute();
  void finalize();

  /// The number of yield surfaces for this plasticity model
  virtual unsigned int numberSurfaces() const;

  /**
   * Calculates the yield functions.  Note that for single-surface plasticity
   * you don't want to override this - override the private yieldFunction below
   * @param stress the stress at which to calculate the yield function
   * @param intnl internal parameter
   * @param f (output) the yield functions
   */
  virtual void yieldFunctionV(const RankTwoTensor & stress, const Real & intnl, std::vector<Real> & f) const;

  /**
   * The derivative of yield functions with respect to stress
   * @param stress the stress at which to calculate the yield function
   * @param intnl internal parameter
   * @param df_dstress (output) df_dstress[alpha](i, j) = dyieldFunction[alpha]/dstress(i, j)
   */
  virtual void dyieldFunction_dstressV(const RankTwoTensor & stress, const Real & intnl, std::vector<RankTwoTensor> & df_dstress) const;

  /**
   * The derivative of yield functions with respect to the internal parameter
   * @param stress the stress at which to calculate the yield function
   * @param intnl internal parameter
   * @param df_dintnl (output) df_dintnl[alpha] = df[alpha]/dintnl
   */
  virtual void dyieldFunction_dintnlV(const RankTwoTensor & stress, const Real & intnl, std::vector<Real> & df_dintnl) const;

  /**
   * The flow potentials
   * @param stress the stress at which to calculate the flow potential
   * @param intnl internal parameter
   * @param r (output) r[alpha] is the flow potential for the "alpha" yield function
   */
  virtual void flowPotentialV(const RankTwoTensor & stress, const Real & intnl, std::vector<RankTwoTensor> & r) const;

  /**
   * The derivative of the flow potential with respect to stress
   * @param stress the stress at which to calculate the flow potential
   * @param intnl internal parameter
   * @param dr_dstress (output) dr_dstress[alpha](i, j, k, l) = dr[alpha](i, j)/dstress(k, l)
   */
  virtual void dflowPotential_dstressV(const RankTwoTensor & stress, const Real & intnl, std::vector<RankFourTensor> & dr_dstress) const;

  /**
   * The derivative of the flow potential with respect to the internal parameter
   * @param stress the stress at which to calculate the flow potential
   * @param intnl internal parameter
   * @param dr_dintnl (output)  dr_dintnl[alpha](i, j) = dr[alpha](i, j)/dintnl
   */
  virtual void dflowPotential_dintnlV(const RankTwoTensor & stress, const Real & intnl, std::vector<RankTwoTensor> & dr_dintnl) const;

  /**
   * The hardening potential
   * @param stress the stress at which to calculate the hardening potential
   * @param intnl internal parameter
   * @param h (output) h[alpha] is the hardening potential for the "alpha" yield function
   */
  virtual void hardPotentialV(const RankTwoTensor & stress, const Real & intnl, std::vector<Real> & h) const;

  /**
   * The derivative of the hardening potential with respect to stress
   * @param stress the stress at which to calculate the hardening potentials
   * @param intnl internal parameter
   * @param dh_dstress (output) dh_dstress[alpha](i, j) = dh[alpha]/dstress(i, j)
   */
  virtual void dhardPotential_dstressV(const RankTwoTensor & stress, const Real & intnl, std::vector<RankTwoTensor> & dh_dstress) const;

  /**
   * The derivative of the hardening potential with respect to the internal parameter
   * @param stress the stress at which to calculate the hardening potentials
   * @param intnl internal parameter
   * @param dh_dintnl (output) dh_dintnl[alpha] = dh[alpha]/dintnl
   */
  virtual void dhardPotential_dintnlV(const RankTwoTensor & stress, const Real & intnl, std::vector<Real> & dh_dintnl) const;

  /**
   * The active yield surfaces, given a vector of yield functions.
   * This is used by FiniteStrainMultiPlasticity to determine the initial
   * set of active constraints at the trial (stress, intnl) configuration.
   * It is up to you (the coder) to determine how accurate you want the
   * returned_stress to be.  Currently it is only used by FiniteStrainMultiPlasticity
   * to estimate a good starting value for the Newton-Rahson procedure,
   * so currently it may not need to be super perfect.
   * @param f values of the yield functions
   * @param stress stress tensor
   * @param intnl internal parameter
   * @param Eijkl elasticity tensor (stress = Eijkl*strain)
   * @param act (output) act[i] = true if the i_th yield function is active
   * @param returned_stress (output) Approximate value of the returned stress
   */
  virtual void activeConstraints(const std::vector<Real> & f, const RankTwoTensor & stress, const Real & intnl, const RankFourTensor & Eijkl, std::vector<bool> & act, RankTwoTensor & returned_stress) const;

  /// Returns the model name (eg "MohrCoulom")
  virtual std::string modelName() const;

  /// Tolerance on yield function
  Real _f_tol;

  /// Tolerance on internal constraint
  Real _ic_tol;


 protected:

  /// The following functions are what you should override when building single-plasticity models
  /**
   * The yield function
   * @param stress the stress at which to calculate the yield function
   * @param intnl internal parameter
   * @return the yield function
   */
  virtual Real yieldFunction(const RankTwoTensor & stress, const Real & intnl) const;

  /**
   * The derivative of yield function with respect to stress
   * @param stress the stress at which to calculate the yield function
   * @param intnl internal parameter
   * @return df_dstress(i, j) = dyieldFunction/dstress(i, j)
   */
  virtual RankTwoTensor dyieldFunction_dstress(const RankTwoTensor & stress, const Real & intnl) const;

  /**
   * The derivative of yield function with respect to the internal parameter
   * @param stress the stress at which to calculate the yield function
   * @param intnl internal parameter
   * @return the derivative
   */
  virtual Real dyieldFunction_dintnl(const RankTwoTensor & stress, const Real & intnl) const;

  /**
   * The flow potential
   * @param stress the stress at which to calculate the flow potential
   * @param intnl internal parameter
   * @return the flow potential
   */
  virtual RankTwoTensor flowPotential(const RankTwoTensor & stress, const Real & intnl) const;

  /**
   * The derivative of the flow potential with respect to stress
   * @param stress the stress at which to calculate the flow potential
   * @param intnl internal parameter
   * @return dr_dstress(i, j, k, l) = dr(i, j)/dstress(k, l)
   */
  virtual RankFourTensor dflowPotential_dstress(const RankTwoTensor & stress, const Real & intnl) const;

  /**
   * The derivative of the flow potential with respect to the internal parameter
   * @param stress the stress at which to calculate the flow potential
   * @param intnl internal parameter
   * @return dr_dintnl(i, j) = dr(i, j)/dintnl
   */
  virtual RankTwoTensor dflowPotential_dintnl(const RankTwoTensor & stress, const Real & intnl) const;

  /**
   * The hardening potential
   * @param stress the stress at which to calculate the hardening potential
   * @param intnl internal parameter
   * @return the hardening potential
   */
  virtual Real hardPotential(const RankTwoTensor & stress, const Real & intnl) const;

  /**
   * The derivative of the hardening potential with respect to stress
   * @param stress the stress at which to calculate the hardening potentials
   * @param intnl internal parameter
   * @return dh_dstress(i, j) = dh/dstress(i, j)
   */
  virtual RankTwoTensor dhardPotential_dstress(const RankTwoTensor & stress, const Real & intnl) const;

  /**
   * The derivative of the hardening potential with respect to the internal parameter
   * @param stress the stress at which to calculate the hardening potentials
   * @param intnl internal parameter
   * @return the derivative
   */
  virtual Real dhardPotential_dintnl(const RankTwoTensor & stress, const Real & intnl) const;

};

#endif // TENSORMECHANICSPLASTICMODEL_H
