// Original class author: A.M. Jokisaari

#include "LinearGeneralAnisotropicMaterial.h"
#include <ostream>

// may need some more includes here.

/**
 * LinearGeneralAnisotropicMaterial handles a fully anisotropic, single-crystal material's elastic
 * constants.  It takes all 21 independent stiffness tensor inputs.  This can be extended or
 * simplified to specify HCP, monoclinic, cubic, etc as needed.
 */

template<>
InputParameters validParams<LinearGeneralAnisotropicMaterial>()
{
  InputParameters params = validParams<SolidMechanicsMaterial>();
  params.addRequiredParam<std::vector<Real> >("C_matrix", "Stiffness tensor for matrix");  
  params.addRequiredParam<bool>("all_21","True if all 21 independent values are given; else false indicates only 9 values given (C11, C12, C13, C22, C23, C33, C44, C55, C66.");
  
  return params;
}

LinearGeneralAnisotropicMaterial::LinearGeneralAnisotropicMaterial(const std::string & name, 
                                                                   InputParameters parameters)
    : SolidMechanicsMaterial(name, parameters),
      _Cijkl_matrix_vector(getParam<std::vector<Real> >("C_matrix")),
      _all_21(getParam<bool>("all_21")),
      _Cijkl_matrix(true)
      //_Cijkl_matrix_MP(declareProperty<SymmElasticityTensor >("Cijkl_matrix_MP"))
{
  // fill in the local tensors from the input vector information
  _Cijkl_matrix.fillFromInputVector(_Cijkl_matrix_vector, _all_21);
}

void
LinearGeneralAnisotropicMaterial::computeQpProperties()
{
  computeQpElasticityTensor();
  computeQpStrain();
  computeQpStress();
}

void LinearGeneralAnisotropicMaterial::computeQpElasticityTensor()
{
  // Fill in the matrix stiffness material property
  _elasticity_tensor[_qp] = _Cijkl_matrix;
  _Jacobian_mult[_qp] = _Cijkl_matrix;
}

void LinearGeneralAnisotropicMaterial::computeQpStrain()
{
 _elastic_strain[_qp] = SymmTensor ( _grad_disp_x[_qp](0),
                                     _grad_disp_y[_qp](1),
                                     _grad_disp_z[_qp](2),
                                     0.5*(_grad_disp_x[_qp](1)+ _grad_disp_y[_qp](0)),
                                     0.5*(_grad_disp_y[_qp](2)+ _grad_disp_z[_qp](1)),
                                     0.5*(_grad_disp_z[_qp](0)+ _grad_disp_x[_qp](2)) );
}

void LinearGeneralAnisotropicMaterial::computeQpStress()
{
  // stress = C * e
  _stress[_qp] = _elasticity_tensor[_qp]*_elastic_strain[_qp];
}
