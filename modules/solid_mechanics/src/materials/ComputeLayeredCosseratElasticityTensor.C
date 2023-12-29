//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeLayeredCosseratElasticityTensor.h"
#include "libmesh/utility.h"
#include "Function.h"
#include "RankTwoTensor.h"

registerMooseObject("TensorMechanicsApp", ComputeLayeredCosseratElasticityTensor);

InputParameters
ComputeLayeredCosseratElasticityTensor::validParams()
{
  InputParameters params = ComputeElasticityTensorBase::validParams();
  params.addClassDescription("Computes Cosserat elasticity and flexural bending rigidity tensors "
                             "relevant for simulations with layered materials.  The layering "
                             "direction is assumed to be perpendicular to the 'z' direction.");
  params.addRequiredParam<Real>("young", "The Young's modulus");
  params.addRequiredParam<Real>("poisson", "The Poisson's ratio");
  params.addRequiredRangeCheckedParam<Real>(
      "layer_thickness", "layer_thickness>=0", "The layer thickness");
  params.addRequiredRangeCheckedParam<Real>(
      "joint_normal_stiffness", "joint_normal_stiffness>=0", "The joint normal stiffness");
  params.addRequiredRangeCheckedParam<Real>(
      "joint_shear_stiffness", "joint_shear_stiffness>=0", "The joint shear stiffness");
  return params;
}

ComputeLayeredCosseratElasticityTensor::ComputeLayeredCosseratElasticityTensor(
    const InputParameters & parameters)
  : ComputeElasticityTensorBase(parameters),
    _Eijkl(RankFourTensor()),
    _Bijkl(RankFourTensor()),
    _Cijkl(RankFourTensor()),
    _elastic_flexural_rigidity_tensor(
        declareProperty<RankFourTensor>("elastic_flexural_rigidity_tensor")),
    _compliance(declareProperty<RankFourTensor>(_base_name + "compliance_tensor"))
{
  if (!isParamValid("elasticity_tensor_prefactor"))
    issueGuarantee(_elasticity_tensor_name, Guarantee::CONSTANT_IN_TIME);

  const Real E = getParam<Real>("young");
  const Real nu = getParam<Real>("poisson");
  const Real b = getParam<Real>("layer_thickness");
  const Real kn = getParam<Real>("joint_normal_stiffness");
  const Real ks = getParam<Real>("joint_shear_stiffness");

  // shear modulus of solid
  const Real G = 0.5 * E / (1.0 + nu);
  // shear modulus of jointed material
  const Real Gprime = G * b * ks / (b * ks + G);

  const Real a0000 =
      (b * kn > 0.0)
          ? E / (1.0 - nu * nu - Utility::pow<2>(nu * (1.0 + nu)) / (1.0 - nu * nu + E / b / kn))
          : E / (1.0 - nu * nu);
  const Real a0011 = nu * a0000 / (1.0 - nu);
  const Real a2222 =
      (b * kn > 0.0) ? 1.0 / ((1.0 + nu) * (1.0 - 2.0 * nu) / E / (1.0 - nu) + 1.0 / b / kn) : 0.0;
  const Real a0022 = nu * a2222 / (1.0 - nu);
  const Real a0101 = G;
  const Real a66 = Gprime;
  const Real a77 = 0.5 * (G + Gprime);

  // Eijkl does not obey the usual symmetries, viz Eijkl != Ejikl, so must fill manually
  _Eijkl(0, 0, 0, 0) = _Eijkl(1, 1, 1, 1) = a0000;
  _Eijkl(0, 0, 1, 1) = _Eijkl(1, 1, 0, 0) = a0011;
  _Eijkl(2, 2, 2, 2) = a2222;
  _Eijkl(0, 0, 2, 2) = _Eijkl(1, 1, 2, 2) = _Eijkl(2, 2, 0, 0) = _Eijkl(2, 2, 1, 1) = a0022;
  _Eijkl(0, 1, 0, 1) = _Eijkl(0, 1, 1, 0) = _Eijkl(1, 0, 0, 1) = _Eijkl(1, 0, 1, 0) = a0101;
  _Eijkl(0, 2, 0, 2) = _Eijkl(0, 2, 2, 0) = _Eijkl(2, 0, 0, 2) = _Eijkl(1, 2, 1, 2) =
      _Eijkl(1, 2, 2, 1) = _Eijkl(2, 1, 1, 2) = a66;
  _Eijkl(2, 0, 2, 0) = _Eijkl(2, 1, 2, 1) = a77;

  // most of Bijkl is zero since the only nonzero moment stresses are m01 and m10.
  // It also does not have the usual symmetries.
  const Real D0 = E * Utility::pow<3>(b) / 12.0 / (1.0 - nu * nu); // bending rigidity of a layer
  const Real b0101 = D0 / b * G / (2.0 * b * ks + G);
  const Real b0110 = -nu * b0101;
  _Bijkl(0, 1, 0, 1) = _Bijkl(1, 0, 1, 0) = b0101;
  _Bijkl(0, 1, 1, 0) = _Bijkl(1, 0, 0, 1) = b0110;

  // The compliance tensor also does not obey the usual symmetries, and
  // this is the main reason it is calculated here, since we can't use
  // _Eijkl.invSymm()
  const Real pre = (nu - 1.0) / (a0000 * (nu - 1.0) + 2.0 * a2222 * Utility::pow<2>(nu));
  const Real cp0000 =
      ((a2222 - a0000) * nu * nu + 2.0 * a0000 * nu - a0000) / (2.0 * a0000 * nu - a0000);
  const Real cp0011 = -((a0000 + a2222) * nu * nu - a0000 * nu) / (2.0 * a0000 * nu - a0000);
  _Cijkl(0, 0, 0, 0) = _Cijkl(1, 1, 1, 1) = pre * cp0000;
  _Cijkl(0, 0, 1, 1) = _Cijkl(1, 1, 0, 0) = pre * cp0011;
  _Cijkl(2, 2, 2, 2) = pre * a0000 / a2222;
  _Cijkl(0, 0, 2, 2) = _Cijkl(1, 1, 2, 2) = _Cijkl(2, 2, 0, 0) = _Cijkl(2, 2, 1, 1) = -nu * pre;
  _Cijkl(0, 1, 0, 1) = _Cijkl(0, 1, 1, 0) = _Cijkl(1, 0, 0, 1) = _Cijkl(1, 0, 1, 0) = 0.25 / a0101;
  const Real slip = 2.0 / (G - Gprime);
  _Cijkl(0, 2, 2, 0) = _Cijkl(2, 0, 0, 2) = _Cijkl(1, 2, 2, 1) = _Cijkl(2, 1, 1, 2) = -slip;
  _Cijkl(0, 2, 0, 2) = _Cijkl(1, 2, 1, 2) = (G + Gprime) * slip / 2.0 / Gprime;
  _Cijkl(2, 0, 2, 0) = _Cijkl(2, 1, 2, 1) = slip;
}

void
ComputeLayeredCosseratElasticityTensor::computeQpElasticityTensor()
{
  _elasticity_tensor[_qp] = _Eijkl;
  _elastic_flexural_rigidity_tensor[_qp] = _Bijkl;
  _compliance[_qp] = _Cijkl;

  if (_prefactor_function)
    _compliance[_qp] /= _prefactor_function->value(_t, _q_point[_qp]);
}
