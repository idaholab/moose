// Original class author: A.M. Jokisaari, O. Heinonen

#include "CosseratLinearElasticMaterial.h"

/**
 * CosseratLinearElasticMaterial handles a fully anisotropic, single-crystal material's elastic
 * constants.  It takes all 21 independent stiffness tensor inputs.  This can be extended or
 * simplified to specify HCP, monoclinic, cubic, etc as needed.
 */

template<>
InputParameters validParams<CosseratLinearElasticMaterial>()
{
  InputParameters params = validParams<TensorMechanicsMaterial>();
  params.addParam<Real>("thermal_expansion_coeff",0,"Thermal expansion coefficient in 1/K");
  params.addParam<Real>("T0",300,"Reference temperature for thermal expansion in K");
  params.addParam<Real>("Temp",300,"Current temperature for thermal expansion in K");
  params.addCoupledVar("T", "Temperature in Kelvin");
  params.addCoupledVar("wc_x", 0, "Cosserat rotation around x axis");
  params.addCoupledVar("wc_y", 0, "Cosserat rotation around y axis");
  params.addCoupledVar("wc_z", 0, "Cosserat rotation around z axis");

  params.addRequiredParam<std::vector<Real> >("B_ijkl", "Flexural bending rigidity tensor.  Should have 9 entries.");

  params.addParam<std::vector<Real> >("applied_strain_vector","Applied strain: e11, e22, e33, e23, e13, e12");


  MooseEnum fm = RankFourTensor::fillMethodEnum();
  fm = "antisymmetric_isotropic";

  params.addParam<MooseEnum>("fill_method_bending", fm, "The fill method for the 'bending' tensor.");

  return params;
}

CosseratLinearElasticMaterial::CosseratLinearElasticMaterial(const std::string & name,
                                             InputParameters parameters)
    : TensorMechanicsMaterial(name, parameters),
      _eigenstrain(declareProperty<RankTwoTensor>("eigenstrain")),
      _delasticity_tensor_dc(declareProperty<ElasticityTensorR4>("delasticity_tensor_dc")),
      _d2elasticity_tensor_dc2(declareProperty<ElasticityTensorR4>("d2elasticity_tensor_dc2")),
      _deigenstrain_dc(declareProperty<RankTwoTensor>("deigenstrain_dc")),
      _d2eigenstrain_dc2(declareProperty<RankTwoTensor>("d2eigenstrain_dc2")),
      _symmetric_strain(declareProperty<RankTwoTensor>("symmetric_strain")),
      _antisymmetric_strain(declareProperty<RankTwoTensor>("antisymmetric_strain")),
      _curvature(declareProperty<RankTwoTensor>("curvature")),
      _symmetric_stress(declareProperty<RankTwoTensor>("symmetric_stress")),
      _antisymmetric_stress(declareProperty<RankTwoTensor>("antisymmetric_stress")),
      _stress_couple(declareProperty<RankTwoTensor>("stress_couple")),
      _elastic_flexural_rigidity_tensor(declareProperty<ElasticityTensorR4>("elastic_flexural_rigidity_tensor")),
      _Jacobian_mult_couple(declareProperty<ElasticityTensorR4>("Jacobian_mult_couple")),
      _Bijkl_vector(getParam<std::vector<Real> >("B_ijkl")),
      _Bijkl(),
      _has_T(isCoupled("T")),
      _T(_has_T ? &coupledValue("T") : NULL),
      _thermal_expansion_coeff(getParam<Real>("thermal_expansion_coeff")),
      _Temp(getParam<Real>("Temp")),
      _T0(getParam<Real>("T0")),
      _applied_strain_vector(getParam<std::vector<Real> >("applied_strain_vector")),
      _wc_x(coupledValue("wc_x")),
      _wc_y(coupledValue("wc_y")),
      _wc_z(coupledValue("wc_z")),
      _grad_wc_x(coupledGradient("wc_x")),
      _grad_wc_y(coupledGradient("wc_y")),
      _grad_wc_z(coupledGradient("wc_z")),
      _fill_method_bending(getParam<MooseEnum>("fill_method_bending"))
{
  //Initialize applied strain tensor from input vector
  if (_applied_strain_vector.size() == 6)
    _applied_strain_tensor.fillFromInputVector(_applied_strain_vector);
  else
    _applied_strain_tensor.zero();


  _Bijkl.fillFromInputVector(_Bijkl_vector, (RankFourTensor::FillMethod)(int)_fill_method_bending);
}

void CosseratLinearElasticMaterial::computeQpStrain()
{
  //strain = (grad_disp + grad_disp^T)/2
  RankTwoTensor grad_tensor(_grad_disp_x[_qp],_grad_disp_y[_qp],_grad_disp_z[_qp]);

  RealVectorValue wc_vector(_wc_x[_qp], _wc_y[_qp], _wc_z[_qp]);

  for (unsigned i = 0 ; i < LIBMESH_DIM ; i++)
    for (unsigned j = 0 ; j < LIBMESH_DIM ; j++)
      for (unsigned k = 0 ; k < LIBMESH_DIM ; k++)
        grad_tensor(i, j) += PermutationTensor::eps(i, j, k)*wc_vector(k);

  _symmetric_strain[_qp] = (grad_tensor + grad_tensor.transpose())/2.0;

  _antisymmetric_strain[_qp] = (grad_tensor - grad_tensor.transpose())/2.0;

  _elastic_strain[_qp] = grad_tensor;

  RankTwoTensor wc_grad_tensor(_grad_wc_x[_qp],_grad_wc_y[_qp],_grad_wc_z[_qp]);

  _curvature[_qp] = wc_grad_tensor;
}

void CosseratLinearElasticMaterial::computeQpStress()
{
  //Calculation and Apply stress free strain
  RankTwoTensor stress_free_strain = computeStressFreeStrain();

  _elastic_strain[_qp] += stress_free_strain;

  // stress = C * e
  _stress[_qp] = _elasticity_tensor[_qp]*_elastic_strain[_qp];

  _symmetric_stress[_qp] = (_stress[_qp] + _stress[_qp].transpose())/2.0;
  _antisymmetric_stress[_qp] = (_stress[_qp] - _stress[_qp].transpose())/2.0;

  _stress_couple[_qp] = _elastic_flexural_rigidity_tensor[_qp] * _curvature[_qp];
}

RankTwoTensor CosseratLinearElasticMaterial::computeStressFreeStrain()
{
  //Apply thermal expansion
  Real T;
  if (_has_T)
    T = (*_T)[_qp];
  else
    T = _Temp;

  RankTwoTensor stress_free_strain;
  stress_free_strain.addIa(-_thermal_expansion_coeff*(T - _T0));

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
