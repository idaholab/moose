#ifndef EIGENSTRAINBASEMATERIAL_H
#define EIGENSTRAINBASEMATERIAL_H

#include "LinearElasticMaterial.h"

/**
 * EigenStrainBaseMaterial is a base to construct material kernels that represent
 * different eigenstrain - concentration relationships.
 * It provides place holders for calculating eigenstrain and its 1st and 2nd
 * order derivatives with respect to c, elasticity_tensor and its 1st and 2nd
 * order derivatives wrt c if it is a function of c instead of a constant.
 */
class EigenStrainBaseMaterial : public LinearElasticMaterial
{
public:
  EigenStrainBaseMaterial(const std:: string & name, InputParameters parameters);

protected:
  virtual void computeEigenStrain() = 0;
  virtual RankTwoTensor computeStressFreeStrain();

  MaterialProperty<RankTwoTensor> & _eigenstrain;
  MaterialProperty<RankTwoTensor> & _deigenstrain_dc;
  MaterialProperty<RankTwoTensor> & _d2eigenstrain_dc2;
};

#endif //EIGENSTRAINBASEMATERIAL_H
