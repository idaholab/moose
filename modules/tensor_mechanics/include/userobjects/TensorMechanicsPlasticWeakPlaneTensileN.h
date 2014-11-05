#ifndef TENSORMECHANICSPLASTICWEAKPLANETENSILEN_H
#define TENSORMECHANICSPLASTICWEAKPLANETENSILEN_H

#include "TensorMechanicsPlasticModel.h"


class TensorMechanicsPlasticWeakPlaneTensileN;


template<>
InputParameters validParams<TensorMechanicsPlasticWeakPlaneTensileN>();

/**
 * Rate-independent associative weak-plane tensile failure
 * with hardening/softening
 */
class TensorMechanicsPlasticWeakPlaneTensileN : public TensorMechanicsPlasticModel
{
 public:
  TensorMechanicsPlasticWeakPlaneTensileN(const std::string & name, InputParameters parameters);


 protected:

  /**
   * The yield function
   * @param stress the stress at which to calculate the yield function
   * @param intnl internal parameter
   * @return the yield function
   */
  Real yieldFunction(const RankTwoTensor & stress, const Real & intnl) const;

  /**
   * The derivative of yield function with respect to stress
   * @param stress the stress at which to calculate the yield function
   * @param intnl internal parameter
   * @return df_dstress(i, j) = dyieldFunction/dstress(i, j)
   */
  RankTwoTensor dyieldFunction_dstress(const RankTwoTensor & stress, const Real & intnl) const;

  /**
   * The derivative of yield function with respect to the internal parameter
   * @param stress the stress at which to calculate the yield function
   * @param intnl internal parameter
   * @return the derivative
   */
  Real dyieldFunction_dintnl(const RankTwoTensor & stress, const Real & intnl) const;

  /**
   * The flow potential
   * @param stress the stress at which to calculate the flow potential
   * @param intnl internal parameter
   * @return the flow potential
   */
  RankTwoTensor flowPotential(const RankTwoTensor & stress, const Real & intnl) const;

  /**
   * The derivative of the flow potential with respect to stress
   * @param stress the stress at which to calculate the flow potential
   * @param intnl internal parameter
   * @return dr_dstress(i, j, k, l) = dr(i, j)/dstress(k, l)
   */
  RankFourTensor dflowPotential_dstress(const RankTwoTensor & stress, const Real & intnl) const;

  /**
   * The derivative of the flow potential with respect to the internal parameter
   * @param stress the stress at which to calculate the flow potential
   * @param intnl internal parameter
   * @return dr_dintnl(i, j) = dr(i, j)/dintnl
   */
  RankTwoTensor dflowPotential_dintnl(const RankTwoTensor & stress, const Real & intnl) const;


  /// tension cutoff
  Real _tension_cutoff;

  /// tension cutoff at infinite hardening/softening
  Real _tension_cutoff_residual;

  /// Tensile strength = cubic between tensile_strength (at zero internal parameter) and tensile_strength_residual (at internal_parameter = tensile_strength_limit).
  Real _tension_cutoff_limit;

  /// Useful quantity in the cubic hardening
  Real _half_tension_cutoff_limit;

  /// Useful quantity in the cubic hardening
  Real _alpha_tension_cutoff;

  /// Useful quantity in the cubic hardening
  Real _beta_tension_cutoff;

  /// Unit normal inputted by user
  RealVectorValue _input_n;

  /// Flow direction, which is constant in this case
  RankTwoTensor _df_dsig;

  /// This rotation matrix rotates _input_n to (0, 0, 1)
  RealTensorValue _rot;

  /// tensile strength as a function of internal_param
  virtual Real tensile_strength(const Real internal_param) const;

  /// d(tensile strength)/d(internal_param) as a function of internal_param
  virtual Real dtensile_strength(const Real internal_param) const;
};

#endif // TENSORMECHANICSPLASTICWEAKPLANETENSILEN_H
