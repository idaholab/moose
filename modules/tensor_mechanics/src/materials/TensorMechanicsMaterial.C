// Original class author: A.M. Jokisaari, O. Heinonen, M. R. Tonks

#include "TensorMechanicsMaterial.h"

/**
 * TensorMechanicsMaterial handles a fully anisotropic, single-crystal material's elastic
 * constants.  It takes all 21 independent stiffness tensor inputs.  This can be extended or
 * simplified to specify HCP, monoclinic, cubic, etc as needed.
 */

template<>
InputParameters validParams<TensorMechanicsMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredParam<std::vector<Real> >("C_ijkl", "Stiffness tensor for material");
  params.addRequiredParam<bool>("all_21","If false, fill C_ijkl as: C1111, C1122, C1133, C2222, C2233, C3333, C2323, C1313, C1212; True if all 21 independent values are given (see ElasticityTensorR4 for details).");
  params.addParam<Real>("euler_angle_1", 0.0, "Euler angle in direction 1");
  params.addParam<Real>("euler_angle_2", 0.0, "Euler angle in direction 2");
  params.addParam<Real>("euler_angle_3", 0.0, "Euler angle in direction 3");
  params.addRequiredCoupledVar("disp_x", "The x displacement");
  params.addRequiredCoupledVar("disp_y", "The y displacement");
  params.addCoupledVar("disp_z", "The z displacement");

  params.addCoupledVar("temperature", "temperature variable");

  return params;
}

TensorMechanicsMaterial::TensorMechanicsMaterial(const std::string & name,
                                             InputParameters parameters)
    : Material(name, parameters),
      _grad_disp_x(coupledGradient("disp_x")),
      _grad_disp_y(coupledGradient("disp_y")),
      _grad_disp_z(_mesh.dimension() == 3 ? coupledGradient("disp_z") : _grad_zero),
      _grad_disp_x_old(coupledGradientOld("disp_x")),
      _grad_disp_y_old(coupledGradientOld("disp_y")),
      _grad_disp_z_old(_mesh.dimension() == 3 ? coupledGradientOld("disp_z") : _grad_zero),
      _stress(declareProperty<RankTwoTensor>("stress")),
      _elastic_strain(declareProperty<RankTwoTensor>("elastic_strain")),
      _elasticity_tensor(declareProperty<ElasticityTensorR4>("elasticity_tensor")),
      _Jacobian_mult(declareProperty<ElasticityTensorR4>("Jacobian_mult")),
      //_d_stress_dT(declareProperty<RankTwoTensor>("d_stress_dT")),
      _euler_angle_1(getParam<Real>("euler_angle_1")),
      _euler_angle_2(getParam<Real>("euler_angle_2")),
      _euler_angle_3(getParam<Real>("euler_angle_3")),
      _Cijkl_vector(getParam<std::vector<Real> >("C_ijkl")),
      _all_21(getParam<bool>("all_21")),
      _Cijkl(),
      _Euler_angles(_euler_angle_1, _euler_angle_2, _euler_angle_3),
      _has_T(isCoupled("temperature")),
      _T(_has_T ? &coupledValue("temperature") : NULL)
{
  // fill in the local tensors from the input vector information

  _Cijkl.fillFromInputVector(_Cijkl_vector, _all_21);
}

void
TensorMechanicsMaterial::computeProperties()
{
  computeStrain();
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    computeQpElasticityTensor();
    computeQpStress();
  }

}

void TensorMechanicsMaterial::computeQpElasticityTensor()
{
  // Fill in the matrix stiffness material property
  _elasticity_tensor[_qp] = _Cijkl;
  _Jacobian_mult[_qp] = _Cijkl;
}

void TensorMechanicsMaterial::computeStrain()
{
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    computeQpStrain();

}

