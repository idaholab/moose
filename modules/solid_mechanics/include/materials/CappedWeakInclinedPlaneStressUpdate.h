//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CappedWeakPlaneStressUpdate.h"

/**
 * CappedWeakInclinedPlaneStressUpdate performs the return-map
 * algorithm and associated stress updates for plastic
 * models that describe capped weak-plane plasticity
 *
 * It assumes various things about the elasticity tensor, viz
 * in the frame where the weak-plane's normal direction is the
 * "2" direction:
 * E(i,i,j,k) = 0 except if k=j
 * E(0,0,i,j) = E(1,1,i,j)
 */
class CappedWeakInclinedPlaneStressUpdate : public CappedWeakPlaneStressUpdate
{
public:
  static InputParameters validParams();

  CappedWeakInclinedPlaneStressUpdate(const InputParameters & parameters);

  /**
   * Does the model require the elasticity tensor to be isotropic?
   */
  bool requiresIsotropicTensor() override { return false; }

protected:
  virtual void initQpStatefulProperties() override;

  /// User-input value of the normal vector to the weak plane
  RealVectorValue _n_input;

  /// Current value of the normal
  MaterialProperty<RealVectorValue> & _n;

  /// Old value of the normal
  const MaterialProperty<RealVectorValue> & _n_old;

  /// Rotation matrix that rotates _n to "z"
  RealTensorValue _rot_n_to_z;

  /// Rotation matrix that rotates "z" to _n
  RealTensorValue _rot_z_to_n;

  /// Trial stress rotated to the frame where _n points along "z"
  RankTwoTensor _rotated_trial;

  /// Elasticity tensor rotated to the frame where _n points along "z"
  RankFourTensor _rotated_Eijkl;

  virtual void initializeReturnProcess() override;
  virtual void finalizeReturnProcess(const RankTwoTensor & rotation_increment) override;

  virtual void preReturnMap(Real p_trial,
                            Real q_trial,
                            const RankTwoTensor & stress_trial,
                            const std::vector<Real> & intnl_old,
                            const std::vector<Real> & yf,
                            const RankFourTensor & Eijkl) override;

  virtual void computePQ(const RankTwoTensor & stress, Real & p, Real & q) const override;

  virtual void setEppEqq(const RankFourTensor & Eijkl, Real & Epp, Real & Eqq) const override;

  virtual void setStressAfterReturn(const RankTwoTensor & stress_trial,
                                    Real p_ok,
                                    Real q_ok,
                                    Real gaE,
                                    const std::vector<Real> & intnl,
                                    const yieldAndFlow & smoothed_q,
                                    const RankFourTensor & Eijkl,
                                    RankTwoTensor & stress) const override;

  virtual void consistentTangentOperator(const RankTwoTensor & stress_trial,
                                         Real p_trial,
                                         Real q_trial,
                                         const RankTwoTensor & stress,
                                         Real p,
                                         Real q,
                                         Real gaE,
                                         const yieldAndFlow & smoothed_q,
                                         const RankFourTensor & Eijkl,
                                         bool compute_full_tangent_operator,
                                         RankFourTensor & cto) const override;

  virtual RankTwoTensor dpdstress(const RankTwoTensor & stress) const override;

  virtual RankTwoTensor dqdstress(const RankTwoTensor & stress) const override;

  virtual RankFourTensor d2qdstress2(const RankTwoTensor & stress) const override;
};
