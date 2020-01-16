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

template <>
InputParameters
validParams<GeneralizedPlaneStrainPD>()
{
  InputParameters params = validParams<ScalarKernel>();
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
  DenseVector<Number> & re = _assembly.residualBlock(_var.number());
  for (_i = 0; _i < re.size(); _i++)
    re(_i) += _gpsuo.returnResidual();
}

void
GeneralizedPlaneStrainPD::computeJacobian()
{
  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), _var.number());
  for (_i = 0; _i < ke.m(); _i++)
    ke(_i, _i) += _gpsuo.returnJacobian();
}
