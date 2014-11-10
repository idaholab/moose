// Original class author: A.M. Jokisaari, O. Heinonen, M. R. Tonks

#include "TensorMechanicsMaterial.h"

template<>
InputParameters validParams<TensorMechanicsMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredParam<std::vector<Real> >("C_ijkl", "Stiffness tensor for material");
  params.addParam<MooseEnum>("fill_method", RankFourTensor::fillMethodEnum() = "symmetric9", "The fill method");
  params.addParam<Real>("euler_angle_1", 0.0, "Euler angle in direction 1");
  params.addParam<Real>("euler_angle_2", 0.0, "Euler angle in direction 2");
  params.addParam<Real>("euler_angle_3", 0.0, "Euler angle in direction 3");
  params.addRequiredCoupledVar("disp_x", "The x displacement");
  params.addRequiredCoupledVar("disp_y", "The y displacement");
  params.addCoupledVar("disp_z", "The z displacement");
  params.addCoupledVar("temperature", "temperature variable");
  params.addParam<std::vector<FunctionName> >("initial_stress", "A list of functions describing the initial stress.  If provided, there must be 9 of these, corresponding to the xx, yx, zx, xy, yy, zy, xz, yz, zz components respectively.  If not provided, all components of the initial stress will be zero");
  return params;
}

TensorMechanicsMaterial::TensorMechanicsMaterial(const std::string & name,
                                                 InputParameters parameters) :
    Material(name, parameters),
    _grad_disp_x(coupledGradient("disp_x")),
    _grad_disp_y(coupledGradient("disp_y")),
    _grad_disp_z(_mesh.dimension() == 3 ? coupledGradient("disp_z") : _grad_zero),
    _grad_disp_x_old(_fe_problem.isTransient() ? coupledGradientOld("disp_x") : _grad_zero),
    _grad_disp_y_old(_fe_problem.isTransient() ? coupledGradientOld("disp_y") : _grad_zero),
    _grad_disp_z_old(_fe_problem.isTransient() && _mesh.dimension() == 3 ? coupledGradientOld("disp_z") : _grad_zero),
    _stress(declareProperty<RankTwoTensor>("stress")),
    _total_strain(declareProperty<RankTwoTensor>("total_strain")),
    _elastic_strain(declareProperty<RankTwoTensor>("elastic_strain")),
    _elasticity_tensor(declareProperty<ElasticityTensorR4>("elasticity_tensor")),
    _Jacobian_mult(declareProperty<ElasticityTensorR4>("Jacobian_mult")),
    // _d_stress_dT(declareProperty<RankTwoTensor>("d_stress_dT")),
    _euler_angle_1(getParam<Real>("euler_angle_1")),
    _euler_angle_2(getParam<Real>("euler_angle_2")),
    _euler_angle_3(getParam<Real>("euler_angle_3")),
    _Cijkl_vector(getParam<std::vector<Real> >("C_ijkl")),
    _Cijkl(),
    _Euler_angles(_euler_angle_1, _euler_angle_2, _euler_angle_3),
    _has_T(isCoupled("temperature")),
    _T(_has_T ? &coupledValue("temperature") : NULL),
    _fill_method((RankFourTensor::FillMethod)(int)getParam<MooseEnum>("fill_method"))
{
  _Cijkl.fillFromInputVector(_Cijkl_vector, _fill_method);

  RotationTensor R(_Euler_angles); // R type: RealTensorValue
  _Cijkl.rotate(R);

  const std::vector<FunctionName> & fcn_names( getParam<std::vector<FunctionName> >("initial_stress") );
  const unsigned num = fcn_names.size();
  if (!(num == 0 || num == 3*3))
    mooseError("Either zero or " << 3*3 << " initial stress functions must be provided to TensorMechanicsMaterial.  You supplied " << num << "\n");
  _initial_stress.resize(num);
  for (unsigned i = 0 ; i < num ; ++i)
    _initial_stress[i] = &getFunctionByName(fcn_names[i]);
}

void
TensorMechanicsMaterial::initQpStatefulProperties()
{
  _stress[_qp].zero();
  if (_initial_stress.size() == 3*3)
    for (unsigned i = 0 ; i < 3 ; ++i)
      for (unsigned j = 0 ; j < 3 ; ++j)
        _stress[_qp](i, j) = _initial_stress[i*3 + j]->value(_t, _q_point[_qp]);

  _total_strain[_qp].zero();
  _elastic_strain[_qp].zero();

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
