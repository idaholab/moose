/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "CosseratLinearElasticMaterial.h"

template<>
InputParameters validParams<CosseratLinearElasticMaterial>()
{
  InputParameters params = validParams<TensorMechanicsMaterial>();
  params.addParam<Real>("thermal_expansion_coeff", 0, "Thermal expansion coefficient in 1/K");
  params.addParam<Real>("T0", 300, "Reference temperature for thermal expansion in K");
  params.addCoupledVar("T", 300, "Temperature in Kelvin");
  params.addRequiredCoupledVar("Cosserat_rotations", "The 3 Cosserat rotation variables");
  params.addRequiredParam<std::vector<Real> >("B_ijkl", "Flexural bending rigidity tensor.  Should have 9 entries.");
  params.addParam<std::vector<Real> >("applied_strain_vector","Applied strain: e11, e22, e33, e23, e13, e12");

  MooseEnum fm = RankFourTensor::fillMethodEnum();
  fm = "antisymmetric_isotropic";
  params.addParam<MooseEnum>("fill_method_bending", fm, "The fill method for the 'bending' tensor.");

  return params;
}

CosseratLinearElasticMaterial::CosseratLinearElasticMaterial(const InputParameters & parameters) :
    TensorMechanicsMaterial(parameters),
    _curvature(declareProperty<RankTwoTensor>("curvature")),
    _stress_couple(declareProperty<RankTwoTensor>("couple_stress")),
    _elastic_flexural_rigidity_tensor(declareProperty<RankFourTensor>("elastic_flexural_rigidity_tensor")),
    _Jacobian_mult_couple(declareProperty<RankFourTensor>("couple_Jacobian_mult")),
    _Bijkl_vector(getParam<std::vector<Real> >("B_ijkl")),
    _Bijkl(),
    _T(coupledValue("T")),
    _thermal_expansion_coeff(getParam<Real>("thermal_expansion_coeff")),
    _T0(getParam<Real>("T0")),
    _applied_strain_vector(getParam<std::vector<Real> >("applied_strain_vector")),
    _nrots(coupledComponents("Cosserat_rotations")),
    _wc(_nrots),
    _grad_wc(_nrots),
    _fill_method_bending(getParam<MooseEnum>("fill_method_bending"))
{
  if (_nrots != 3)
    mooseError("CosseratLinearElasticMaterial: This Material is only defined for 3-dimensional simulations so 3 Cosserat rotation variables are needed");
  for (unsigned i = 0; i < _nrots; ++i)
  {
    _wc[i] = &coupledValue("Cosserat_rotations", i);
    _grad_wc[i] = &coupledGradient("Cosserat_rotations", i);
  }

  //Initialize applied strain tensor from input vector
  if (_applied_strain_vector.size() == 6)
    _applied_strain_tensor.fillFromInputVector(_applied_strain_vector);
  else
    _applied_strain_tensor.zero();

  _Bijkl.fillFromInputVector(_Bijkl_vector, (RankFourTensor::FillMethod)(int)_fill_method_bending);
}

void
CosseratLinearElasticMaterial::computeQpStrain()
{
  RankTwoTensor strain(_grad_disp_x[_qp], _grad_disp_y[_qp], _grad_disp_z[_qp]);
  RealVectorValue wc_vector((*_wc[0])[_qp], (*_wc[1])[_qp], (*_wc[2])[_qp]);

  for (unsigned i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned j = 0; j < LIBMESH_DIM; ++j)
      for (unsigned k = 0; k < LIBMESH_DIM; ++k)
        strain(i, j) += PermutationTensor::eps(i, j, k) * wc_vector(k);

  _elastic_strain[_qp] = strain;

  _curvature[_qp] = RankTwoTensor((*_grad_wc[0])[_qp], (*_grad_wc[1])[_qp], (*_grad_wc[2])[_qp]);
}

void
CosseratLinearElasticMaterial::computeQpStress()
{
  //Calculation and apply stress free strain
  RankTwoTensor stress_free_strain = computeStressFreeStrain();

  _elastic_strain[_qp] += stress_free_strain;

  // stress = E * e
  _stress[_qp] = _elasticity_tensor[_qp] * _elastic_strain[_qp];

  _stress_couple[_qp] = _elastic_flexural_rigidity_tensor[_qp] * _curvature[_qp];
}

RankTwoTensor
CosseratLinearElasticMaterial::computeStressFreeStrain()
{
  //Apply thermal expansion
  RankTwoTensor stress_free_strain;
  stress_free_strain.addIa(-_thermal_expansion_coeff * (_T[_qp] - _T0));

  //Apply uniform applied strain
  if (_applied_strain_vector.size() == 6)
    stress_free_strain += _applied_strain_tensor;

  return stress_free_strain;
}

void CosseratLinearElasticMaterial::computeQpElasticityTensor()
{
  TensorMechanicsMaterial::computeQpElasticityTensor();

  _elastic_flexural_rigidity_tensor[_qp] = _Bijkl;
  _Jacobian_mult_couple[_qp] = _Bijkl;
}
