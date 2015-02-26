#include "ComputeSmallStrain.h"

template<>
InputParameters validParams<ComputeSmallStrain>()
{
  InputParameters params = validParams<ComputeStrainBase>();
  return params;
}

ComputeSmallStrain::ComputeSmallStrain(const std::string & name,
                                                 InputParameters parameters) :
    DerivativeMaterialInterface<ComputeStrainBase>(name, parameters),
    _total_strain(declareProperty<RankTwoTensor>(_base_name + "total_strain")),
    _eigen_strain(getDefaultMaterialProperty<RankTwoTensor>(_base_name + "eigen_strain"))
{
}

void
ComputeSmallStrain::initQpStatefulProperties()
{
  _total_strain[_qp].zero();
}

void
ComputeSmallStrain::computeProperties()
{
  for (unsigned int _qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    //strain = (grad_disp + grad_disp^T)/2
    RankTwoTensor grad_tensor(_grad_disp_x[_qp], _grad_disp_y[_qp], _grad_disp_z[_qp]);

    _total_strain[_qp] = ( grad_tensor + grad_tensor.transpose() )/2.0;

    //Remove thermal expansion
    _total_strain[_qp].addIa(-_thermal_expansion_coeff*( _T[_qp] - _T0 ));

    //Remove the Eigen strain
    _total_strain[_qp] -= _eigen_strain[_qp];

  }
}
