/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ACGrGrElasticDrivingForce.h"

#include "Material.h"
#include "RankFourTensor.h"
#include "RankTwoTensor.h"

template <>
InputParameters
validParams<ACGrGrElasticDrivingForce>()
{
  InputParameters params = ACBulk<Real>::validParams();
  params.addClassDescription("Adds elastic energy contribution to the Allen-Cahn equation");
  params.addRequiredParam<MaterialPropertyName>(
      "D_tensor_name", "The elastic tensor derivative for the specific order parameter");
  return params;
}

ACGrGrElasticDrivingForce::ACGrGrElasticDrivingForce(const InputParameters & parameters)
  : ACBulk<Real>(parameters),
    _D_elastic_tensor(getMaterialProperty<RankFourTensor>("D_tensor_name")),
    _elastic_strain(getMaterialPropertyByName<RankTwoTensor>("elastic_strain"))
{
}

Real
ACGrGrElasticDrivingForce::computeDFDOP(PFFunctionType type)
{
  // Access the heterogeneous strain calculated by the Solid Mechanics kernels
  RankTwoTensor strain(_elastic_strain[_qp]);

  // Compute the partial derivative of the stress wrt the order parameter
  RankTwoTensor D_stress = _D_elastic_tensor[_qp] * strain;

  switch (type)
  {
    case Residual:
      return 0.5 *
             D_stress.doubleContraction(strain); // Compute the deformation energy driving force

    case Jacobian:
      return 0.0;
  }

  mooseError("Invalid type passed in");
}
