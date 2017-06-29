/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTELINEARVISCOELASTICCREEPSTRAIN_H
#define COMPUTELINEARVISCOELASTICCREEPSTRAIN_H

#include "ComputeCreepStrainBase.h"
#include "LinearViscoelasticityBase.h"

class ComputeLinearViscoelasticCreepStrain;

template <>
InputParameters validParams<ComputeLinearViscoelasticCreepStrain>();

/**
 * Computes the creep strains resulting from a spring-dashpot model.
 * The spring-dashpot model itself is stored in a LinearViscoelasticityBase object,
 * which performs the update and computation of the creep strain itself.
 *
 * The creep strain is separated into a series of internal viscous strain,
 * on a one-by-one basis for each dashpot in the spring-dashpot model.
 *
 * The creep strain can also be drived by an eigenstrain, in which case
 * the eigenstrain acts as an additional stress on the spring-dashpot model,
 * and therefore increases the creep strain accordingly.
 */
class ComputeLinearViscoelasticCreepStrain : public ComputeCreepStrainBase
{
public:
  ComputeLinearViscoelasticCreepStrain(const InputParameters & parameters);

  // calls the spring-dashpot model to update the viscous strains
  virtual void updateQpViscousStrain(unsigned int qp,
                                     const RankTwoTensor & strain,
                                     const RankTwoTensor & stress);

protected:
  virtual void initQpStatefulProperties();
  // calls the spring-dashpot model to compute the creep strain from the viscous strains
  virtual void computeQpCreepStrain();

  // pointer to a LinearViscoelasticityBase object that represents an arbitrary
  // assembly of springs and dashpots
  std::string _viscoelastic_model_name;
  MooseSharedPointer<LinearViscoelasticityBase> _viscoelastic_model;

  // internal viscous strains (one component for each dashpot in the model)
  MaterialProperty<std::vector<RankTwoTensor>> & _viscous_strains;
  MaterialProperty<std::vector<RankTwoTensor>> & _viscous_strains_old;

  // apparent and instantaneous elasticity tensors of the spring-dashpot assembly
  const MaterialProperty<RankFourTensor> & _apparent_elasticity_tensor;
  const MaterialProperty<RankFourTensor> & _instantaneous_elasticity_tensor;

  // eigenstrain that causes an additional creep deformation under zero stress
  bool _has_driving_eigenstrain;
  Real _driving_eigenstrain_prefactor;
  std::string _driving_eigenstrain_name;
  const MaterialProperty<RankTwoTensor> * _driving_eigenstrain;

private:
  // get the pointer to a LinearViscoelasticityBase object
  MooseSharedPointer<LinearViscoelasticityBase>
  getViscoelasticModel(const std::string & viscoelastic_model_name) const;
};

#endif // COMPUTELINEARVISCOELASTICCREEPSTRAIN_H
