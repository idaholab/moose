#ifndef COMPUTESTRESSBASE_H
#define COMPUTESTRESSBASE_H

#include "Material.h"
#include "RankTwoTensor.h"
#include "ElasticityTensorR4.h"
#include "RotationTensor.h"
#include "DerivativeMaterialInterface.h"

/**
 * ComputeStressBase defines is the base class for strain tensors
 */
class ComputeStressBase : public DerivativeMaterialInterface<Material>
{
public:
  ComputeStressBase(const std:: string & name, InputParameters parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();
  virtual void computeQpStress() = 0;

  std::string _base_name;

  MaterialProperty<RankTwoTensor> & _stress;
  MaterialProperty<ElasticityTensorR4> & _elasticity_tensor;

  /// initial stress components
  std::vector<Function *> _initial_stress;

  /// derivative of stress w.r.t. strain (_dstress_dstrain)
  MaterialProperty<ElasticityTensorR4> & _Jacobian_mult;
};

#endif //COMPUTESTRESSBASE_H
