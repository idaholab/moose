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
      _Cijkl_matrix(true),
      _Cijkl_matrix_MP(declareProperty<SymmElasticityTensor >("Cijkl_matrix_MP"))
{
  // fill in the local tensors from the input vector information
  _Cijkl_matrix.fillFromInputVector(_Cijkl_matrix_vector, _all_21);
}

void
 LinearGeneralAnisotropicMaterial::computeQpProperties()
 {
   // Fill in the matrix stiffness material property
   _Cijkl_matrix_MP[_qp] = _Cijkl_matrix;
   _Jacobian_mult[_qp] = _Cijkl_matrix;

   //debugging
   //std::cout << _Cijkl_matrix, std::cout << std::endl;
   //std::cout <<  _Cijkl_matrix_MP[_qp], std::cout << std::endl;
 }
