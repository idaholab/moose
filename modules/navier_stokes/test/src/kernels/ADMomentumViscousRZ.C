//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADMomentumViscousRZ.h"

registerMooseObject("NavierStokesTestApp", ADMomentumViscousRZ);

InputParameters
ADMomentumViscousRZ::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addParam<MaterialPropertyName>(
      "mu_name", "mu", "The name of the viscosity material property");
  params.addRequiredParam<unsigned short>("component", "The momentum component this object is for");
  return params;
}

ADMomentumViscousRZ::ADMomentumViscousRZ(const InputParameters & parameters)
  : ADKernel(parameters),
    _mu(getADMaterialProperty<Real>("mu_name")),
    _coord_sys(_assembly.coordSystem()),
    _rz_radial_coord(_mesh.getAxisymmetricRadialCoord()),
    _component(getParam<unsigned short>("component"))
{
}

ADReal
ADMomentumViscousRZ::computeQpResidual()
{
  mooseAssert(_coord_sys == Moose::COORD_RZ, "this object is only meant for RZ coordinate systems");

  if (_component == _rz_radial_coord)
  {
    const auto r = _ad_q_point[_qp](_rz_radial_coord);
    return _mu[_qp] * _u[_qp] / (r * r) * _test[_i][_qp];
  }
  else
    return 0;
}
