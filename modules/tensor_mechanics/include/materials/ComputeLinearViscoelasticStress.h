/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTELINEARVISCOELASTICSTRESS_H
#define COMPUTELINEARVISCOELASTICSTRESS_H

#include "ComputeLinearElasticStress.h"
#include "LinearViscoelasticityBase.h"

class ComputeLinearViscoelasticStress;

template <>
InputParameters validParams<ComputeLinearViscoelasticStress>();

/**
 * Computes the stress of a linear viscoelastic material, using total
 * small strains. The mechanical strain is decomposed into the elastic
 * strain + the creep strain, the creep strain itself resulting from
 * a spring-dashpot model.
 *
 * If you need to accomodate other sources of inelastic strains, use
 * a ComputeMultipleInelasticStress material instead, associated with a
 * LinearViscoelasticStressUpdate to represent the creep strain.
 */
class ComputeLinearViscoelasticStress : public ComputeLinearElasticStress
{
public:
  ComputeLinearViscoelasticStress(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpStress() override;

  /// Creep strain variable
  MaterialProperty<RankTwoTensor> & _creep_strain;
  const MaterialProperty<RankTwoTensor> & _creep_strain_old;

  /// Apparent creep strain (extracted from a LinearViscoelasticityBase object)
  const MaterialProperty<RankTwoTensor> & _apparent_creep_strain;
  /// Apparent elasticity tensor (extracted from a LinearViscoelasticityBase object)
  const MaterialProperty<RankFourTensor> & _apparent_elasticity_tensor;
  /// Instantaneous compliance tensor (extracted from a LinearViscoelasticityBase object)
  const MaterialProperty<RankFourTensor> & _instantaneous_elasticity_tensor_inv;
};

#endif // COMPUTELINEARVISCOELASTICSTRESS_H
