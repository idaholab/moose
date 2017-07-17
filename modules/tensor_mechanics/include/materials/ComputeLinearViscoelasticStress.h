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
  virtual void initialSetup() override;

  /// Creep strain variable
  MaterialProperty<RankTwoTensor> & _creep_strain;
  const MaterialProperty<RankTwoTensor> & _creep_strain_old;

  std::string _viscoelastic_model_name;
  /// Pointer to the linear viscoelastic model
  std::shared_ptr<LinearViscoelasticityBase> _viscoelastic_model;
};

#endif // COMPUTELINEARVISCOELASTICSTRESS_H
