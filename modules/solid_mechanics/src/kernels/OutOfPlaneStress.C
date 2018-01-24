//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OutOfPlaneStress.h"

#include "Material.h"
#include "SymmElasticityTensor.h"

template <>
InputParameters
validParams<OutOfPlaneStress>()
{
  InputParameters params = validParams<Kernel>();
  params.addCoupledVar("disp_x", "The x displacement");
  params.addCoupledVar("disp_y", "The y displacement");
  params.addCoupledVar("temp", "The temperature");
  params.addParam<std::string>(
      "appended_property_name", "", "Name appended to material properties to make them unique");

  params.set<bool>("use_displaced_mesh") = true;

  return params;
}

OutOfPlaneStress::OutOfPlaneStress(const InputParameters & parameters)
  : Kernel(parameters),
    _stress(getMaterialProperty<SymmTensor>("stress" +
                                            getParam<std::string>("appended_property_name"))),
    _Jacobian_mult(getMaterialProperty<SymmElasticityTensor>(
        "Jacobian_mult" + getParam<std::string>("appended_property_name"))),
    _d_stress_dT(getMaterialProperty<SymmTensor>("d_stress_dT" +
                                                 getParam<std::string>("appended_property_name"))),
    _xdisp_coupled(isCoupled("disp_x")),
    _ydisp_coupled(isCoupled("disp_y")),
    _temp_coupled(isCoupled("temp")),
    _xdisp_var(_xdisp_coupled ? coupled("disp_x") : 0),
    _ydisp_var(_ydisp_coupled ? coupled("disp_y") : 0),
    _temp_var(_temp_coupled ? coupled("temp") : 0)
{
}

Real
OutOfPlaneStress::computeQpResidual()
{
  return _stress[_qp].component(2) * _test[_i][_qp];
}

Real
OutOfPlaneStress::computeQpJacobian()
{
  Real C33 = _Jacobian_mult[_qp].valueAtIndex(11);
  return C33 * _test[_i][_qp] * _phi[_j][_qp];
}

Real
OutOfPlaneStress::computeQpOffDiagJacobian(unsigned int /*jvar*/)
{
  return 0;
}
