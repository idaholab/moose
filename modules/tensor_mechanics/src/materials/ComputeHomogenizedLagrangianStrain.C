#include "ComputeHomogenizedLagrangianStrain.h"

registerMooseObject("TensorMechanicsApp", ComputeHomogenizedLagrangianStrain);

InputParameters
ComputeHomogenizedLagrangianStrain::validParams()
{
  InputParameters params = Material::validParams();

  params.addRequiredCoupledVar("displacements", "Displacement variables");

  params.addParam<bool>(
      "large_kinematics", false, "Use large displacement kinematics in the kernel.");

  params.addParam<MaterialPropertyName>("homogenization_gradient_name",
                                        "homogenization_gradient",
                                        "Name of the constant gradient field");
  params.addRequiredCoupledVar("macro_gradient",
                               "Scalar field defining the "
                               "macro gradient");

  return params;
}

ComputeHomogenizedLagrangianStrain::ComputeHomogenizedLagrangianStrain(
    const InputParameters & parameters)
  : Material(parameters),
    _ndisp(coupledComponents("displacements")),
    _large_kinematics(getParam<bool>("large_kinematics")),
    _macro_gradient(coupledScalarValue("macro_gradient")),
    _homogenization_contribution(declareProperty<RankTwoTensor>("homogenization_gradient_name"))
{
}

void
ComputeHomogenizedLagrangianStrain::computeQpProperties()
{
  _homogenization_contribution[_qp] = calculateTensorContribution();
}

RankTwoTensor
ComputeHomogenizedLagrangianStrain::calculateTensorContribution()
{
  if (_large_kinematics)
  {
    if (_ndisp == 1)
      return RankTwoTensor(_macro_gradient[0], 0, 0, 0, 0, 0, 0, 0, 0);
    else if (_ndisp == 2)
      return RankTwoTensor(_macro_gradient[0],
                           _macro_gradient[2],
                           0,
                           _macro_gradient[3],
                           _macro_gradient[1],
                           0,
                           0,
                           0,
                           0);
    else
      return RankTwoTensor(_macro_gradient[0],
                           _macro_gradient[1],
                           _macro_gradient[2],
                           _macro_gradient[3],
                           _macro_gradient[4],
                           _macro_gradient[5],
                           _macro_gradient[6],
                           _macro_gradient[7],
                           _macro_gradient[8]);
  }
  else
  {
    if (_ndisp == 1)
      return RankTwoTensor(_macro_gradient[0], 0, 0, 0, 0, 0, 0, 0, 0);
    else if (_ndisp == 2)
      return RankTwoTensor(
          _macro_gradient[0], 0, 0, _macro_gradient[2], _macro_gradient[1], 0, 0, 0, 0);
    else
      return RankTwoTensor(_macro_gradient[0],
                           0.0,
                           0.0,
                           _macro_gradient[5],
                           _macro_gradient[1],
                           0.0,
                           _macro_gradient[4],
                           _macro_gradient[3],
                           _macro_gradient[2]);
  }
}
