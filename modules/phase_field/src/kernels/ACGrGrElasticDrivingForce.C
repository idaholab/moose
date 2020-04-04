//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ACGrGrElasticDrivingForce.h"

#include "Material.h"
#include "RankFourTensor.h"
#include "RankTwoTensor.h"

registerMooseObject("PhaseFieldApp", ACGrGrElasticDrivingForce);

InputParameters
ACGrGrElasticDrivingForce::validParams()
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
