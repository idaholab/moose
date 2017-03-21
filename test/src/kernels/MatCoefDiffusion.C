/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/
#include "MatCoefDiffusion.h"

template <>
InputParameters
validParams<MatCoefDiffusion>()
{
  MooseEnum test("none=0 hasMaterialProperty=1", "none", "Select optional test");

  InputParameters params = validParams<Kernel>();
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
