/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
// Original class author: A.M. Jokisaari, O. Heinonen, M. R. Tonks

#include "TensorMechanicsMaterial.h"
#include "Function.h"
#include "MooseMesh.h"

// libmesh includes
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<TensorMechanicsMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredParam<std::vector<Real>>("C_ijkl", "Stiffness tensor for material");
  params.addParam<MooseEnum>(
      "fill_method", RankFourTensor::fillMethodEnum() = "symmetric9", "The fill method");
  params.addParam<Real>("euler_angle_1", 0.0, "Euler angle in direction 1");
  params.addParam<Real>("euler_angle_2", 0.0, "Euler angle in direction 2");
  params.addParam<Real>("euler_angle_3", 0.0, "Euler angle in direction 3");
  params.addRequiredCoupledVar("disp_x", "The x displacement");
  params.addRequiredCoupledVar("disp_y", "The y displacement");
  params.addCoupledVar("disp_z", "The z displacement");
  params.addCoupledVar("temperature", "temperature variable");
  params.addParam<std::vector<FunctionName>>(
      "initial_stress",
      "A list of functions describing the initial stress.  If provided, there "
      "must be 9 of these, corresponding to the xx, yx, zx, xy, yy, zy, xz, yz, "
      "zz components respectively.  If not provided, all components of the "
      "initial stress will be zero");
  params.addParam<FunctionName>(
      "elasticity_tensor_prefactor",
      "Optional function to use as a scalar prefactor on the elasticity tensor.");
  params.addParam<std::string>("base_name", "Material property base name");
  return params;
}

TensorMechanicsMaterial::TensorMechanicsMaterial(const InputParameters & parameters)
  : Material(parameters),
    _grad_disp_x(coupledGradient("disp_x")),
    _grad_disp_y(coupledGradient("disp_y")),
    _grad_disp_z(_mesh.dimension() == 3 ? coupledGradient("disp_z") : _grad_zero),
    _grad_disp_x_old(_fe_problem.isTransient() ? coupledGradientOld("disp_x") : _grad_zero),
    _grad_disp_y_old(_fe_problem.isTransient() ? coupledGradientOld("disp_y") : _grad_zero),
    _grad_disp_z_old(_fe_problem.isTransient() && _mesh.dimension() == 3
                         ? coupledGradientOld("disp_z")
                         : _grad_zero),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),

    _stress(declareProperty<RankTwoTensor>(_base_name + "stress")),
    _total_strain(declareProperty<RankTwoTensor>(_base_name + "total_strain")),
    _elastic_strain(declareProperty<RankTwoTensor>(_base_name + "elastic_strain")),

    _elasticity_tensor_name(_base_name + "elasticity_tensor"),
    _elasticity_tensor(declareProperty<RankFourTensor>(_elasticity_tensor_name)),

    _Jacobian_mult(declareProperty<RankFourTensor>(_base_name + "Jacobian_mult")),

    _Euler_angles(getParam<Real>("euler_angle_1"),
                  getParam<Real>("euler_angle_2"),
                  getParam<Real>("euler_angle_3")),

    _Cijkl(getParam<std::vector<Real>>("C_ijkl"),
           (RankFourTensor::FillMethod)(int)getParam<MooseEnum>("fill_method")),
    _prefactor_function(isParamValid("elasticity_tensor_prefactor")
                            ? &getFunction("elasticity_tensor_prefactor")
                            : NULL)
{
  mooseDeprecated("EigenStrainBaseMaterial is deprecated.   Please use the TensorMechanics "
                  "plug-and-play system instead: "
                  "http://mooseframework.org/wiki/PhysicsModules/TensorMechanics/"
                  "PlugAndPlayMechanicsApproach/");

  const std::vector<FunctionName> & fcn_names(
      getParam<std::vector<FunctionName>>("initial_stress"));
  const unsigned num = fcn_names.size();

  if (!(num == 0 || num == 3 * 3))
    mooseError(
        "Either zero or ",
        3 * 3,
        " initial stress functions must be provided to TensorMechanicsMaterial.  You supplied ",
        num,
        "\n");

  _initial_stress.resize(num);
  for (unsigned i = 0; i < num; ++i)
    _initial_stress[i] = &getFunctionByName(fcn_names[i]);
}

void
TensorMechanicsMaterial::initQpStatefulProperties()
{
  _stress[_qp].zero();
  if (_initial_stress.size() == 3 * 3)
    for (unsigned i = 0; i < 3; ++i)
      for (unsigned j = 0; j < 3; ++j)
        _stress[_qp](i, j) = _initial_stress[i * 3 + j]->value(_t, _q_point[_qp]);

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

void
TensorMechanicsMaterial::computeQpElasticityTensor()
{
  // Fill in the matrix stiffness material property
  RotationTensor R(_Euler_angles); // R type: RealTensorValue
  _elasticity_tensor[_qp] = _Cijkl;

  if (_prefactor_function)
    _elasticity_tensor[_qp] *= _prefactor_function->value(_t, _q_point[_qp]);

  _elasticity_tensor[_qp].rotate(R);
  _Jacobian_mult[_qp] = _elasticity_tensor[_qp];
}

void
TensorMechanicsMaterial::computeStrain()
{
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    computeQpStrain();
}
