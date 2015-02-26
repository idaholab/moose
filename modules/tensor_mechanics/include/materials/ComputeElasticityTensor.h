#ifndef COMPUTEELASTICITYTENSOR_H
#define COMPUTEELASTICITYTENSOR_H

#include "Material.h"
#include "RankTwoTensor.h"
#include "ElasticityTensorR4.h"
#include "RotationTensor.h"

/**
 * ComputeElasticityTensor defines an elasticity tensor material object with a given base name. Can also be used as a base class to define more complicated elasticity tensors.
 */
class ComputeElasticityTensor : public DerivativeMaterialInterface<Material>
{
public:
  ComputeElasticityTensor(const std:: string & name, InputParameters parameters);

protected:
  virtual void computeQpProperties();

  std::string _base_name;

  MaterialProperty<ElasticityTensorR4> & _elasticity_tensor;

  RealVectorValue _Euler_angles;

  /// Individual material information
  ElasticityTensorR4 _Cijkl;

  /// prefactor function to multiply the elasticity tensor with
  Function * const _prefactor_function;

};

#endif //COMPUTEELASTICITYTENSOR_H
