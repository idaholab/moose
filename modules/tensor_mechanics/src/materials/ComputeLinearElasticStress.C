#include "ComputeLinearElasticStress.h"

template<>
InputParameters validParams<ComputeLinearElasticStress>()
{
  InputParameters params = validParams<ComputeStressBase>();
  return params;
}

ComputeLinearElasticStress::ComputeLinearElasticStress(const std::string & name,
                                                 InputParameters parameters) :
    DerivativeMaterialInterface<ComputeStressBase>(name, parameters),
    _total_strain(getMaterialProperty<RankTwoTensor>(_base_name + "total_strain"))
{
}

void
ComputeLinearElasticStress::computeQpStress()
{
  _stress[_qp] = _elasticity_tensor[_qp]*_total_strain[_qp];
  _Jacobian_mult[_qp] = _elasticity_tensor[_qp];
}
