/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeLayeredCosseratElasticityTensor.h"
#include "libmesh/utility.h"

template <>
InputParameters
validParams<ComputeLayeredCosseratElasticityTensor>()
{
  InputParameters params = validParams<ComputeElasticityTensorBase>();
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
    _elastic_flexural_rigidity_tensor(
        declareProperty<RankFourTensor>("elastic_flexural_rigidity_tensor"))
{
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
}

void
ComputeLayeredCosseratElasticityTensor::computeQpElasticityTensor()
{
  _elasticity_tensor[_qp] = _Eijkl;
  _elastic_flexural_rigidity_tensor[_qp] = _Bijkl;
}
