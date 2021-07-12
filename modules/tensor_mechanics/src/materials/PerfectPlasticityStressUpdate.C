#include "PerfectPlasticityStressUpdate.h"

registerMooseObject("TensorMechanicsApp", PerfectPlasticityStressUpdate);

InputParameters
PerfectPlasticityStressUpdate::validParams()
{
  InputParameters params = ComputeLagrangianStressSmall::validParams();

  params.addParam<MaterialPropertyName>("yield_stress",
                                        "yield_stress",
                                        "The name of the material property "
                                        "with the yield stress.");
  params.addParam<MaterialPropertyName>(
      "elasticity_tensor", "elasticity_tensor", "The name of the elasticity tensor.");

  return params;
}

PerfectPlasticityStressUpdate::PerfectPlasticityStressUpdate(const InputParameters & parameters)
  : ComputeLagrangianStressSmall(parameters),
    _elasticity_tensor(
        getMaterialProperty<RankFourTensor>(getParam<MaterialPropertyName>("elasticity_tensor"))),
    _yield_stress(getMaterialProperty<Real>(getParam<MaterialPropertyName>("yield_stress")))
{
}

void
PerfectPlasticityStressUpdate::computeQpSmallStress()
{
  // Elastic predictor
  RankTwoTensor s_tr = _small_stress_old[_qp] + _elasticity_tensor[_qp] * _strain_increment[_qp];

  // Flow surface and distance we are from it
  Real f_tr = f(s_tr);
  Real dist = f_tr - _yield_stress[_qp];
  // In the elastic regime
  if (dist <= 0.0)
  {
    // Stress is just the predictor
    _small_stress[_qp] = s_tr;
    // Jacobian is the elasticity tensor
    _small_jacobian[_qp] = _elasticity_tensor[_qp];
  }
  else
  {
    // Flow direction
    RankTwoTensor ni = n(s_tr);
    // Derivative of the flow direction
    RankFourTensor Ni = N(s_tr);
    // Flow direction "length"
    Real nn = ni.doubleContraction(ni);
    // Used in tangent
    RankFourTensor nxn = ni.outerProduct(ni);
    // Consistency parameter
    Real alpha = dist / nn;
    // Stress update: trial minus correction
    _small_stress[_qp] = s_tr - alpha * ni;
    // See documentation
    _small_jacobian[_qp] =
        _elasticity_tensor[_qp] *
        (RankFourTensor::IdentityFour() + 2.0 / nn * alpha * nxn * Ni - nxn / nn - alpha * Ni);
  }
}

Real
PerfectPlasticityStressUpdate::f(const RankTwoTensor & stress)
{
  RankTwoTensor s = stress.deviatoric();

  return sqrt(3.0 / 2.0 * s.doubleContraction(s));
}

RankTwoTensor
PerfectPlasticityStressUpdate::n(const RankTwoTensor & stress)
{
  return 3.0 / (2.0 * f(stress)) * stress.deviatoric();
}

RankFourTensor
PerfectPlasticityStressUpdate::N(const RankTwoTensor & stress)
{
  RankTwoTensor ni = n(stress);
  RankFourTensor ID = RankFourTensor::IdentityDeviatoric();

  return 3.0 / (2.0 * f(stress)) * (ID - 2.0 / 3.0 * ni.outerProduct(ni));
}
