//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MatCoefDiffusion.h"

registerMooseObject("MooseTestApp", MatCoefDiffusion);

InputParameters
MatCoefDiffusion::validParams()
{
  MooseEnum test("none=0 hasMaterialProperty=1", "none", "Select optional test");

  InputParameters params = Kernel::validParams();
  params.addParam<MaterialPropertyName>("conductivity",
                                        "the name of the material property for conductivity");
  params.addParam<MooseEnum>("test", test, "Select the desired test");
  return params;
}

MatCoefDiffusion::MatCoefDiffusion(const InputParameters & parameters)
  : Kernel(parameters), _prop_name(getParam<MaterialPropertyName>("conductivity"))
{

  // Get the test parameter
  MooseEnum test = parameters.get<MooseEnum>("test");

  // Perform the special tests available
  switch (test)
  {
    // Check that hasMaterialProperty is working
    // (true if material property exists anywhere)
    case 1:
      if (hasMaterialProperty<Real>(_prop_name))
        mooseError("hasMaterialProperty test passed");
      else
        mooseError("hasMaterialProperty test failed");
  }

  // Check that hasBlockMaterialProperty is working
  // (true only if blocks of material match blocks of object)
  if (hasBlockMaterialProperty<Real>(_prop_name))
    _coef = &getMaterialPropertyByName<Real>(_prop_name);
  else
    mooseError("The material property ", _prop_name, " is not defined on all blocks of the kernel");
}

Real
MatCoefDiffusion::computeQpResidual()
{
  return (*_coef)[_qp] * _grad_test[_i][_qp] * _grad_u[_qp];
}

Real
MatCoefDiffusion::computeQpJacobian()
{
  return (*_coef)[_qp] * _grad_test[_i][_qp] * _grad_phi[_j][_qp];
}
