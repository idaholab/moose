/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTECAPPEDWEAKINCLINEDPLANESTRESS_H
#define COMPUTECAPPEDWEAKINCLINEDPLANESTRESS_H

#include "ComputeCappedWeakPlaneStress.h"

class ComputeCappedWeakInclinedPlaneStress;

template <>
InputParameters validParams<ComputeCappedWeakInclinedPlaneStress>();

/**
 * ComputeCappedWeakInclinedPlaneStress performs the return-map
 * algorithm and associated stress updates for plastic
 * models that describe capped weak-plane plasticity
 *
 * It assumes various things about the elasticity tensor, viz
 * in the frame where the weak-plane's normal direction is the
 * "2" direction:
 * E(i,i,j,k) = 0 except if k=j
 * E(0,0,i,j) = E(1,1,i,j)
 */
class ComputeCappedWeakInclinedPlaneStress : public ComputeCappedWeakPlaneStress
{
public:
  ComputeCappedWeakInclinedPlaneStress(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpStress() override;

  /// User-input value of the normal vector to the weak plane
  RealVectorValue _n_input;

  /// Current value of the normal
  MaterialProperty<RealVectorValue> & _n;

  /// Old value of the normal
  MaterialProperty<RealVectorValue> & _n_old;

  /// Rotation matrix that rotates _n to "z"
  RealTensorValue _rot_n_to_z;

  /// Rotation matrix that rotates "z" to _n
  RealTensorValue _rot_z_to_n;

  /// Trial stress rotated to the frame where _n points along "z"
  RankTwoTensor _rotated_trial;

  /// Elasticity tensor rotated to the frame where _n points along "z"
  RankFourTensor _rotated_Eijkl;

  virtual void initialiseReturnProcess() override;

  virtual void preReturnMap(Real p_trial,
                            Real q_trial,
                            const RankTwoTensor & stress_trial,
                            const std::vector<Real> & intnl_old,
                            const std::vector<Real> & yf) override;

  virtual void computePQ(const RankTwoTensor & stress, Real & p, Real & q) const override;

  virtual void setEppEqq(const RankFourTensor & Eijkl, Real & Epp, Real & Eqq) const override;

  virtual void setStressAfterReturn(const RankTwoTensor & stress_trial,
                                    Real p_ok,
                                    Real q_ok,
                                    Real gaE,
                                    const std::vector<Real> & intnl,
                                    const f_and_derivs & smoothed_q,
                                    RankTwoTensor & stress) const override;

  virtual void consistentTangentOperator(const RankTwoTensor & stress_trial,
                                         Real p_trial,
                                         Real q_trial,
                                         const RankTwoTensor & stress,
                                         Real p,
                                         Real q,
                                         Real gaE,
                                         const f_and_derivs & smoothed_q,
                                         RankFourTensor & cto) const override;

  virtual RankTwoTensor dpdstress(const RankTwoTensor & stress) const override;

  virtual RankTwoTensor dqdstress(const RankTwoTensor & stress) const override;

  virtual RankFourTensor d2qdstress2(const RankTwoTensor & stress) const override;
};

#endif // COMPUTECAPPEDWEAKINCLINEDPLANESTRESS_H
