// Original class author: A.M. Jokisaari, O. Heinonen

#include "LinearElasticMaterial.h"
#include <ostream>


/**
 * LinearElasticMaterial handles a fully anisotropic, single-crystal material's elastic
 * constants.  It takes all 21 independent stiffness tensor inputs.  This can be extended or
 * simplified to specify HCP, monoclinic, cubic, etc as needed.
 */

template<>
InputParameters validParams<LinearElasticMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredParam<std::vector<Real> >("C_ijkl", "Stiffness tensor for material");  
  params.addRequiredParam<bool>("all_21","If false, fill C_ijkl as: C1111, C1122, C1133, C2222, C2233, C3333, C2323, C1313, C1212; True if all 21 independent values are given (see RankFourTensor for details).");
  params.addParam<Real>("euler_angle_1", 0.0, "Euler angle in direction 1");
  params.addParam<Real>("euler_angle_2", 0.0, "Euler angle in direction 2");
  params.addParam<Real>("euler_angle_3", 0.0, "Euler angle in direction 3");
  params.addRequiredCoupledVar("disp_x", "The x displacement");
  params.addRequiredCoupledVar("disp_y", "The y displacement");
  params.addCoupledVar("disp_z", "The z displacement");

  return params;
}

LinearElasticMaterial::LinearElasticMaterial(const std::string & name, 
                                             InputParameters parameters)
    : Material(name, parameters),
      _grad_disp_x(coupledGradient("disp_x")),
      _grad_disp_y(coupledGradient("disp_y")),
      _grad_disp_z(_dim == 3 ? coupledGradient("disp_z") : _grad_zero),
      _stress(declareProperty<RankTwoTensor>("stress")),
      _elasticity_tensor(declareProperty<RankFourTensor>("elasticity_tensor")),
      _Jacobian_mult(declareProperty<RankFourTensor>("Jacobian_mult")),
      _elastic_strain(declareProperty<RankTwoTensor>("elastic_strain")),
      _d_stress_dT(declareProperty<RankTwoTensor>("d_stress_dT")),
      _euler_angle_1(getParam<Real>("euler_angle_1")),
      _euler_angle_2(getParam<Real>("euler_angle_2")),
      _euler_angle_3(getParam<Real>("euler_angle_3")),
      _Cijkl_vector(getParam<std::vector<Real> >("C_ijkl")),
      _all_21(getParam<bool>("all_21")),
      _Cijkl()
{
  // fill in the local tensors from the input vector information
  _Cijkl.fillFromInputVector(_Cijkl_vector, _all_21);
  
  //rotate the C_ijkl matrix 
  // _Cijkl_matrix.rotate(_euler_angle_1,_euler_angle_2,_euler_angle_3);
  
  //debugging
  /*_Cijkl_matrix.show_r_matrix();
    _Cijkl_matrix.show_dt_matrix();
    if(libMesh::on_command_line("--debug") || libMesh::on_command_line("--debug-elasticity-Cijkl"))
    {
    libMesh::out << "Material " << this->name() << " on mesh block " << this->blockID() << " has _Cijkl_matrix:\n" << _Cijkl_matrix << "\n";
    }*/
}

void
LinearElasticMaterial::computeQpProperties()
{
  computeQpElasticityTensor();
  computeQpStrain();
  computeQpStress();
}

void LinearElasticMaterial::computeQpElasticityTensor()
{
  // Fill in the matrix stiffness material property
  _elasticity_tensor[_qp] = _Cijkl;
  _Jacobian_mult[_qp] = _Cijkl;
}

void LinearElasticMaterial::computeQpStrain()
{
  _elastic_strain[_qp].setValue(_grad_disp_x[_qp](0), 1, 1);
  _elastic_strain[_qp].setValue(_grad_disp_y[_qp](1), 2, 2);
  _elastic_strain[_qp].setValue(_grad_disp_z[_qp](2), 3, 3);
  _elastic_strain[_qp].setValue(0.5*(_grad_disp_x[_qp](1)+_grad_disp_y[_qp](0)), 1, 2);
  _elastic_strain[_qp].setValue(0.5*(_grad_disp_x[_qp](1)+_grad_disp_y[_qp](0)), 2, 1);
  _elastic_strain[_qp].setValue(0.5*(_grad_disp_x[_qp](2)+_grad_disp_z[_qp](0)), 1, 3);
  _elastic_strain[_qp].setValue(0.5*(_grad_disp_x[_qp](2)+_grad_disp_z[_qp](0)), 3, 1);
  _elastic_strain[_qp].setValue(0.5*(_grad_disp_y[_qp](2)+_grad_disp_z[_qp](1)), 2, 3);
  _elastic_strain[_qp].setValue(0.5*(_grad_disp_y[_qp](2)+_grad_disp_z[_qp](1)), 3, 2);
}

void LinearElasticMaterial::computeQpStress()
{
  // stress = C * e
  _stress[_qp] = _elasticity_tensor[_qp]*_elastic_strain[_qp];
}

template <>
PropertyValue *
MaterialProperty<RankFourTensor>::init(int size)
{
  MaterialProperty<RankFourTensor> *copy
    = new MaterialProperty<RankFourTensor>();
  copy->_value.resize(size);
  return copy;
}

template <>
PropertyValue *
MaterialProperty<RankTwoTensor>::init(int size)
{
  MaterialProperty<RankTwoTensor> *copy
    = new MaterialProperty<RankTwoTensor>();
  copy->_value.resize(size);
  return copy;
}
