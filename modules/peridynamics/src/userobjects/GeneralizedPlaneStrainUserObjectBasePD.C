//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeneralizedPlaneStrainUserObjectBasePD.h"
#include "RankFourTensor.h"

InputParameters
GeneralizedPlaneStrainUserObjectBasePD::validParams()
{
  InputParameters params = ElementUserObjectBasePD::validParams();
  params.addClassDescription("Base class for calculating the scalar residual and diagonal Jacobian "
                             "entry for generalized plane strain formulation");

  MooseEnum strainType("SMALL", "SMALL");
  params.addParam<MooseEnum>("strain", strainType, "Strain formulation");
  params.addParam<FunctionName>(
      "out_of_plane_pressure",
      "0",
      "Function used to prescribe pressure in the out-of-plane direction");
  params.addParam<Real>("factor", 1.0, "Scale factor applied to prescribed pressure");
  params.set<ExecFlagEnum>("execute_on") = EXEC_LINEAR;

  return params;
}

GeneralizedPlaneStrainUserObjectBasePD::GeneralizedPlaneStrainUserObjectBasePD(
    const InputParameters & parameters)
  : ElementUserObjectBasePD(parameters),
    _strain(getParam<MooseEnum>("strain")),
    _Cijkl(getMaterialProperty<RankFourTensor>("elasticity_tensor")),
    _pressure(getFunction("out_of_plane_pressure")),
    _factor(getParam<Real>("factor"))
{
}

void
GeneralizedPlaneStrainUserObjectBasePD::initialize()
{
  _residual = 0;
  _jacobian = 0;
}

void
GeneralizedPlaneStrainUserObjectBasePD::threadJoin(const UserObject & uo)
{
  const GeneralizedPlaneStrainUserObjectBasePD & gpsuo =
      static_cast<const GeneralizedPlaneStrainUserObjectBasePD &>(uo);
  _residual += gpsuo._residual;
  _jacobian += gpsuo._jacobian;
}

void
GeneralizedPlaneStrainUserObjectBasePD::finalize()
{
  gatherSum(_residual);
  gatherSum(_jacobian);
}

Real
GeneralizedPlaneStrainUserObjectBasePD::returnResidual() const
{
  return _residual;
}

Real
GeneralizedPlaneStrainUserObjectBasePD::returnJacobian() const
{
  return _jacobian;
}
