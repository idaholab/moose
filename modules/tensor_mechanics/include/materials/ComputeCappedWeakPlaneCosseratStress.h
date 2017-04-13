/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTECAPPEDWEAKPLANECOSSERATSTRESS_H
#define COMPUTECAPPEDWEAKPLANECOSSERATSTRESS_H

#include "ComputeCappedWeakPlaneStress.h"

class ComputeCappedWeakPlaneCosseratStress;

template <>
InputParameters validParams<ComputeCappedWeakPlaneCosseratStress>();

/**
 * ComputeCappedWeakPlaneCosseratStress performs the return-map
 * algorithm and associated stress updates for plastic
 * models that describe capped weak-plane Cosserat plasticity
 *
 * It assumes various things about the elasticity tensor, viz
 * E(i,i,j,k) = 0 except if k=j
 * E(0,0,i,j) = E(1,1,i,j)
 */
class ComputeCappedWeakPlaneCosseratStress : public ComputeCappedWeakPlaneStress
{
public:
  ComputeCappedWeakPlaneCosseratStress(const InputParameters & parameters);

protected:
  /// The Cosserat curvature strain
  const MaterialProperty<RankTwoTensor> & _curvature;

  /// The Cosserat elastic flexural rigidity tensor
  const MaterialProperty<RankFourTensor> & _elastic_flexural_rigidity_tensor;

  /// the Cosserat couple-stress
  MaterialProperty<RankTwoTensor> & _couple_stress;

  /// the old value of Cosserat couple-stress
  MaterialProperty<RankTwoTensor> & _couple_stress_old;

  /// derivative of couple-stress w.r.t. curvature
  MaterialProperty<RankFourTensor> & _Jacobian_mult_couple;

  virtual void initQpStatefulProperties() override;

  virtual void initialiseReturnProcess() override;

  virtual void consistentTangentOperator(const RankTwoTensor & stress_trial,
                                         Real p_trial,
                                         Real q_trial,
                                         const RankTwoTensor & stress,
                                         Real p,
                                         Real q,
                                         Real gaE,
                                         const f_and_derivs & smoothed_q,
                                         RankFourTensor & cto) const override;

  virtual void setStressAfterReturn(const RankTwoTensor & stress_trial,
                                    Real p_ok,
                                    Real q_ok,
                                    Real gaE,
                                    const std::vector<Real> & intnl,
                                    const f_and_derivs & smoothed_q,
                                    RankTwoTensor & stress) const override;

  virtual RankTwoTensor dqdstress(const RankTwoTensor & stress) const override;

  virtual RankFourTensor d2qdstress2(const RankTwoTensor & stress) const override;
};

#endif // COMPUTECAPPEDWEAKPLANECOSSERATSTRESS_H
