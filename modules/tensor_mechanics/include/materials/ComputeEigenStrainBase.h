#ifndef COMPUTEEIGENSTRAINBASE_H
#define COMPUTEEIGENSTRAINBASE_H

#include "Material.h"
#include "RankTwoTensor.h"
#include "ElasticityTensorR4.h"
#include "RotationTensor.h"
#include "DerivativeMaterialInterface.h"

/**
 * ComputeEigenStrainBase defines is the base class for strain tensors
 */
class ComputeEigenStrainBase : public DerivativeMaterialInterface<Material>
{
public:
  ComputeEigenStrainBase(const std:: string & name, InputParameters parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();
  virtual void computeQpEigenStrain() = 0;

  std::string _base_name;

  bool _incremental_form;

  MaterialProperty<RankTwoTensor> & _eigen_strain;
  MaterialProperty<RankTwoTensor> * _eigen_strain_old;
  MaterialProperty<RankTwoTensor> & _eigen_strain_increment;
};

#endif //COMPUTEEIGENSTRAINBASE_H
