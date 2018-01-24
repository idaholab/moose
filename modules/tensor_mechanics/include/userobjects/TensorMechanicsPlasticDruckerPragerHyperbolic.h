/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef TENSORMECHANICSPLASTICDRUCKERPRAGERHYPERBOLIC_H
#define TENSORMECHANICSPLASTICDRUCKERPRAGERHYPERBOLIC_H

#include "TensorMechanicsPlasticDruckerPrager.h"
#include "TensorMechanicsHardeningModel.h"

class TensorMechanicsPlasticDruckerPragerHyperbolic;

template <>
InputParameters validParams<TensorMechanicsPlasticDruckerPragerHyperbolic>();

/**
 * Rate-independent non-associative Drucker Prager
 * with hardening/softening.  The cone's tip is smoothed in a hyperbolic fashion
 * Most functions (eg flowPotential, etc) are simply inherited from
 * TensorMechanicsPlasticDruckerPrager.  Note df_dsig is over-ridden
 */
class TensorMechanicsPlasticDruckerPragerHyperbolic : public TensorMechanicsPlasticDruckerPrager
{
public:
  TensorMechanicsPlasticDruckerPragerHyperbolic(const InputParameters & parameters);

  virtual std::string modelName() const override;

  virtual bool useCustomReturnMap() const override;

  virtual bool useCustomCTO() const override;

protected:
  Real yieldFunction(const RankTwoTensor & stress, Real intnl) const override;

  RankFourTensor dflowPotential_dstress(const RankTwoTensor & stress, Real intnl) const override;

  /// Function that's used in dyieldFunction_dstress and flowPotential
  virtual RankTwoTensor df_dsig(const RankTwoTensor & stress, Real bbb) const override;

  virtual bool returnMap(const RankTwoTensor & trial_stress,
                         Real intnl_old,
                         const RankFourTensor & E_ijkl,
                         Real ep_plastic_tolerance,
                         RankTwoTensor & returned_stress,
                         Real & returned_intnl,
                         std::vector<Real> & dpm,
                         RankTwoTensor & delta_dp,
                         std::vector<Real> & yf,
                         bool & trial_stress_inadmissible) const override;

  virtual RankFourTensor
  consistentTangentOperator(const RankTwoTensor & trial_stress,
                            Real intnl_old,
                            const RankTwoTensor & stress,
                            Real intnl,
                            const RankFourTensor & E_ijkl,
                            const std::vector<Real> & cumulative_pm) const override;

private:
  /// smoothing parameter for the cone's tip
  const Real _smoother2;

  /// whether to use the custom returnMap function
  const bool _use_custom_returnMap;

  /// Whether to use the custom consistent tangent operator calculation
  const bool _use_custom_cto;

  /// max iters for custom return map loop
  const unsigned _max_iters;
};

#endif // TENSORMECHANICSPLASTICDRUCKERPRAGERHYPERBOLIC_H
