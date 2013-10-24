#include "MatCoefDiffusion.h"

template<>
InputParameters validParams<MatCoefDiffusion>()
{
  MooseEnum test("none=0, hasMaterialProperty=1", "none", "Select optional test");

  InputParameters params = validParams<Kernel>();
  params.addParam<std::string>("conductivity", "the name of the material property for conductivity");
  params.addParam<MooseEnum>("test", test, "Select the desired test");
  return params;
}

MatCoefDiffusion::MatCoefDiffusion(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters),
    _prop_name(getParam<std::string>("conductivity"))
{

  // Get the test paramter
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
  if (hasBlockMaterialProperty(_prop_name))
    _coef = &getMaterialProperty<Real>(_prop_name);
  else
    mooseError("The material property " << _prop_name << " is not defined on all blocks of the kernel");
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
