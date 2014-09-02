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
 */
class TensorMechanicsPlasticModel : public GeneralUserObject
{
 public:
  TensorMechanicsPlasticModel(const std::string & name, InputParameters parameters);

  void initialize();
  void execute();
  void finalize();

  /**
   * The yield function
   * @param stress the stress at which to calculate the yield function
   * @param intnl internal parameter
   * @return the yield function
   */
  virtual Real yieldFunction(const RankTwoTensor & stress, const Real & intnl) const = 0;

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
  virtual Real dyieldFunction_dintnl(const RankTwoTensor & stress, const Real & intnl) const = 0;

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

  /// Tolerance on yield function
  Real _f_tol;

  /// Tolerance on internal constraint
  Real _ic_tol;

};

#endif // TENSORMECHANICSPLASTICMODEL_H
