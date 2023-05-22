//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeneralizedPlaneStrainPD.h"
#include "GeneralizedPlaneStrainUserObjectBasePD.h"
#include "MooseVariableScalar.h"
#include "Assembly.h"

registerMooseObject("PeridynamicsApp", GeneralizedPlaneStrainPD);

InputParameters
GeneralizedPlaneStrainPD::validParams()
{
  InputParameters params = ScalarKernel::validParams();
  params.addClassDescription("Class for claculating residual and diagonal Jacobian for"
                             "state-based peridynamic generalized plane strain formulation");

  params.addRequiredParam<UserObjectName>(
      "generalized_plane_strain_uo",
      "UserObject name of the GeneralizedPlaneStrainUserObjectBasePD");

  return params;
}

GeneralizedPlaneStrainPD::GeneralizedPlaneStrainPD(const InputParameters & parameters)
  : ScalarKernel(parameters),
    _gpsuo(getUserObject<GeneralizedPlaneStrainUserObjectBasePD>("generalized_plane_strain_uo"))
{
}

void
GeneralizedPlaneStrainPD::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());
  for (unsigned int i = 0; i < _local_re.size(); ++i)
    _local_re(i) += _gpsuo.returnResidual();
  accumulateTaggedLocalResidual();
}

void
GeneralizedPlaneStrainPD::computeJacobian()
{
  prepareMatrixTag(_assembly, _var.number(), _var.number());
  for (unsigned int i = 0; i < _local_ke.m(); ++i)
    _local_ke(i, i) += _gpsuo.returnJacobian();
  accumulateTaggedLocalMatrix();
}
