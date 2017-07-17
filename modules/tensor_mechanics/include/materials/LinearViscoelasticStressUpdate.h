/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef LINEARVISCOELASTICSTRESSUPDATE_H
#define LINEARVISCOELASTICSTRESSUPDATE_H

#include "StressUpdateBase.h"
#include "LinearViscoelasticityBase.h"

class LinearViscoelasticStressUpdate;

template <>
InputParameters validParams<LinearViscoelasticStressUpdate>();

class LinearViscoelasticStressUpdate : public StressUpdateBase
{
public:
  LinearViscoelasticStressUpdate(const InputParameters & parameters);

  /**
   * Given a strain increment that results in a trial stress, perform some
   * procedure (such as an iterative return-mapping process) to produce
   * an admissible stress, an elastic strain increment and an inelastic
   * strain increment, as well as d(stress)/d(strain) (or some approximation
   * to it).
   *
   * This method is called by ComputeMultipleInelasticStress.
   * This method is pure virutal: all inheriting classes must overwrite this method.
   *
   * @param strain_increment Upon input: the strain increment.  Upon output: the elastic strain
   * increment
   * @param inelastic_strain_increment The inelastic_strain resulting from the interative procedure
   * @param rotation_increment The finite-strain rotation increment
   * @param stress_new Upon input: the trial stress that results from applying strain_increment as
   * an elastic strain.  Upon output: the admissible stress
   * @param stress_old The old value of stress
   * @param elasticity_tensor The elasticity tensor
   * @param compute_full_tangent_operator The calling routine would like the full consistent tangent
   * operator to be placed in tangent_operator, if possible.
   * @param tangent_operator d(stress)/d(strain), or some approximation to it  If
   * compute_full_tangent_operator=false, then tangent_operator=elasticity_tensor is an appropriate
   * choice
   */
  virtual void updateState(RankTwoTensor & strain_increment,
                           RankTwoTensor & inelastic_strain_increment,
                           const RankTwoTensor & rotation_increment,
                           RankTwoTensor & stress_new,
                           const RankTwoTensor & stress_old,
                           const RankFourTensor & elasticity_tensor,
                           const RankTwoTensor & elastic_strain_old,
                           bool compute_full_tangent_operator,
                           RankFourTensor & tangent_operator) override;

  /**
   * If updateState is not called during a timestep, this will be.  This method allows derived
   * classes to set internal parameters from their Old values, for instance
   */
  virtual void propagateQpStatefulProperties() override;

  virtual bool requiresIsotropicTensor() override { return false; }

protected:
  virtual void initQpStatefulProperties() override;

  std::string _base_name;

  MaterialProperty<RankTwoTensor> & _creep_strain;
  MaterialProperty<RankTwoTensor> & _creep_strain_old;

  std::string _viscoelastic_model_name;
  std::shared_ptr<LinearViscoelasticityBase> _viscoelastic_model;

};

#endif // LINEARVISCOELASTICSTRESSUPDATE_H
