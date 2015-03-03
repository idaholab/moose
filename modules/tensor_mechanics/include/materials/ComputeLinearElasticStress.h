#ifndef COMPUTELINEARELASTICSTRESS_H
#define COMPUTELINEARELASTICSTRESS_H

#include "ComputeStressBase.h"

/**
 * ComputeLinearElasticStress computes the stress following linear elasticity theory (small strains)
 */
class ComputeLinearElasticStress : public ComputeStressBase
{
public:
  ComputeLinearElasticStress(const std:: string & name, InputParameters parameters);

protected:
  virtual void computeQpStress();

  MaterialProperty<RankTwoTensor> & _total_strain;
  const bool _is_finite_strain;
};

#endif //COMPUTELINEARELASTICSTRESS_H
