/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
// Original class author: A.M. Jokisaari
// O. Heinonen, et al. at ANL also have contributed significantly - thanks guys!

#include "LinearGeneralAnisotropicMaterial.h"
#include <ostream>

/**
 * LinearGeneralAnisotropicMaterial handles a fully anisotropic, single-crystal material's elastic
 * constants.  It takes all 21 independent stiffness tensor inputs.  This can be extended or
 * simplified to specify HCP, monoclinic, cubic, etc as needed.
 */

template <>
InputParameters
validParams<LinearGeneralAnisotropicMaterial>()
{
  InputParameters params = validParams<SolidMechanicsMaterial>();
  params.addRequiredParam<std::vector<Real>>("C_matrix", "Stiffness tensor for matrix");
  params.addRequiredParam<bool>("all_21",
                                "True if all 21 independent values are given; else false "
                                "indicates only 9 values given (C11, C12, C13, C22, C23, "
                                "C33, C44, C55, C66.");
  params.addParam<Real>("euler_angle_1", 0.0, "Euler angle in direction 1");
  params.addParam<Real>("euler_angle_2", 0.0, "Euler angle in direction 2");
  params.addParam<Real>("euler_angle_3", 0.0, "Euler angle in direction 3");

  return params;
}

LinearGeneralAnisotropicMaterial::LinearGeneralAnisotropicMaterial(
    const InputParameters & parameters)
  : SolidMechanicsMaterial(parameters),
    _euler_angle_1(getParam<Real>("euler_angle_1")),
    _euler_angle_2(getParam<Real>("euler_angle_2")),
    _euler_angle_3(getParam<Real>("euler_angle_3")),
    _Cijkl_matrix_vector(getParam<std::vector<Real>>("C_matrix")),
    _all_21(getParam<bool>("all_21")),
    _Cijkl_matrix()
{
  // fill in the local tensors from the input vector information
  _Cijkl_matrix.fillFromInputVector(_Cijkl_matrix_vector, _all_21);

  // rotate the C_ijkl matrix
  _Cijkl_matrix.rotate(_euler_angle_1, _euler_angle_2, _euler_angle_3);

  // debugging
  /*_Cijkl_matrix.show_r_matrix();
    _Cijkl_matrix.show_dt_matrix();
    if (libMesh::on_command_line("--debug") || libMesh::on_command_line("--debug-elasticity-Cijkl"))
    {
      libMesh::out << "Material " << this->name() << " on mesh block " << this->blockID() << " has
    _Cijkl_matrix:\n" << _Cijkl_matrix << "\n";
    }*/
}

void
LinearGeneralAnisotropicMaterial::computeQpProperties()
{
  computeQpElasticityTensor();
  computeQpStrain();
  computeQpStress();
}

void
LinearGeneralAnisotropicMaterial::computeQpElasticityTensor()
{
  // Fill in the matrix stiffness material property
  _elasticity_tensor[_qp] = _Cijkl_matrix;
  _Jacobian_mult[_qp] = _Cijkl_matrix;
}

void
LinearGeneralAnisotropicMaterial::computeQpStrain()
{
  _elastic_strain[_qp] = SymmTensor(_grad_disp_x[_qp](0),
                                    _grad_disp_y[_qp](1),
                                    _grad_disp_z[_qp](2),
                                    0.5 * (_grad_disp_x[_qp](1) + _grad_disp_y[_qp](0)),
                                    0.5 * (_grad_disp_y[_qp](2) + _grad_disp_z[_qp](1)),
                                    0.5 * (_grad_disp_z[_qp](0) + _grad_disp_x[_qp](2)));
}

void
LinearGeneralAnisotropicMaterial::computeQpStress()
{
  // stress = C * e
  _stress[_qp] = _elasticity_tensor[_qp] * _elastic_strain[_qp];
}
