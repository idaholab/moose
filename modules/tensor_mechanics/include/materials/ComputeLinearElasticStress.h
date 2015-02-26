#ifndef COMPUTELINEARELASTICSTRESS_H
#define COMPUTELINEARELASTICSTRESS_H

#include "ComputeStressBase.h"
#include "RankTwoTensor.h"
#include "ElasticityTensorR4.h"
#include "RotationTensor.h"
#include "DerivativeMaterialInterface.h"

/**
 * ComputeLinearElasticStress computes the stress following linear elasticity theory
 */
class ComputeLinearElasticStress : public DerivativeMaterialInterface<ComputeStressBase>
{
public:
  ComputeLinearElasticStress(const std:: string & name, InputParameters parameters);

protected:
  virtual void computeQpStress() ;

  MaterialProperty<RankTwoTensor> & _total_strain;
};

#endif //COMPUTELINEARELASTICSTRESS_H
