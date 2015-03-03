#ifndef COMPUTEFINITESTRAINELASTICSTRESS_H
#define COMPUTEFINITESTRAINELASTICSTRESS_H

#include "ComputeStressBase.h"

/**
 * ComputeFiniteStrainElasticStress computes the stress following elasticity theory for either small or finite strains
 */
class ComputeFiniteStrainElasticStress : public ComputeStressBase
{
public:
  ComputeFiniteStrainElasticStress(const std:: string & name, InputParameters parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpStress();

  const MaterialProperty<RankTwoTensor> & _strain_increment;
  const MaterialProperty<RankTwoTensor> & _rotation_increment;
  MaterialProperty<RankTwoTensor> & _stress_old;
};

#endif //COMPUTEFINITESTRAINELASTICSTRESS_H
