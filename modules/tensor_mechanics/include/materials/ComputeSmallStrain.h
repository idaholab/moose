#ifndef COMPUTESMALLSTRAIN_H
#define COMPUTESMALLSTRAIN_H

#include "ComputeStrainBase.h"

/**
 * ComputeSmallStrain defines a strain tensor, assuming small strains.
 */
class ComputeSmallStrain : public DerivativeMaterialInterface<ComputeStrainBase>
{
public:
  ComputeSmallStrain(const std:: string & name, InputParameters parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeProperties();

  MaterialProperty<RankTwoTensor> & _total_strain;
  const MaterialProperty<RankTwoTensor> & _eigen_strain;

};

#endif //COMPUTESMALLSTRAIN_H
