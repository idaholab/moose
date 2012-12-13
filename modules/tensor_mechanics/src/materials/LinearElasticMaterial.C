// Original class author: A.M. Jokisaari, O. Heinonen

#include "LinearElasticMaterial.h"

/**
 * LinearElasticMaterial handles a fully anisotropic, single-crystal material's elastic
 * constants.  It takes all 21 independent stiffness tensor inputs.  This can be extended or
 * simplified to specify HCP, monoclinic, cubic, etc as needed.
 */

template<>
InputParameters validParams<LinearElasticMaterial>()
{
  InputParameters params = validParams<TensorMechanicsMaterial>();

  return params;
}

LinearElasticMaterial::LinearElasticMaterial(const std::string & name, 
                                             InputParameters parameters)
    : TensorMechanicsMaterial(name, parameters),
      _elastic_strain(declareProperty<RankTwoTensor>("elastic_strain"))
{
}

void LinearElasticMaterial::computeQpStrain()
{
  //strain = (grad_disp + grad_disp^T)/2
  RankTwoTensor grad_tensor(_grad_disp_x[_qp],_grad_disp_y[_qp],_grad_disp_z[_qp]);
  
  if (_t_step > 1000000)
  {
    RankTwoTensor test = grad_tensor;
    test.addIa(1.0);
    
    RankTwoTensor eye = test*test.inverse();
    eye.print();
  }
  
  _elastic_strain[_qp] = (grad_tensor + grad_tensor.transpose())/2.0;
  _elastic_strain[_qp] += _applied_strain;
}

void LinearElasticMaterial::computeQpStress()
{
  // stress = C * e
  _stress[_qp] = _elasticity_tensor[_qp]*_elastic_strain[_qp];
}
