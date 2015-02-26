#ifndef COMPUTEELASTICITYSTRESS_H
#define COMPUTEELASTICITYSTRESS_H

#include "ComputeStressBase.h"
#include "RankTwoTensor.h"
#include "ElasticityTensorR4.h"
#include "RotationTensor.h"
#include "DerivativeMaterialInterface.h"

/**
 * ComputeElasticityStress computes the stress following elasticity theory for either small or finite strains
 */
class ComputeElasticityStress : public DerivativeMaterialInterface<ComputeStressBase>
{
public:
  ComputeElasticityStress(const std:: string & name, InputParameters parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpStress();

  MaterialProperty<RankTwoTensor> & _total_strain;
  bool _is_finite_strain;
  const MaterialProperty<RankTwoTensor> & _strain_increment;
  const MaterialProperty<RankTwoTensor> & _rotation_increment;
  MaterialProperty<RankTwoTensor> * _stress_old;
};

#endif //COMPUTEELASTICITYSTRESS_H
