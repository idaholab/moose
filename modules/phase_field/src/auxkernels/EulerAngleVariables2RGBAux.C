/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "EulerAngleVariables2RGBAux.h"
#include "Euler2RGB.h"

template <>
InputParameters
validParams<EulerAngleVariables2RGBAux>()
{
  InputParameters params = validParams<AuxKernel>();
  MooseEnum sd_enum = MooseEnum("100=1 010=2 001=3", "001");
  params.addParam<MooseEnum>("sd", sd_enum, "Reference sample direction");
  MooseEnum output_types = MooseEnum("red green blue scalar", "scalar");
  params.addParam<MooseEnum>("output_type", output_types, "Type of value that will be outputted");
  params.addCoupledVar("phi1", "Euler angle 1");
  params.addCoupledVar("phi", "Euler angle 2");
  params.addCoupledVar("phi2", "Euler angle 3");
  params.addCoupledVar("phase", "Grain phase index");
  params.addCoupledVar("symmetry", "Grain symmetry indentifier");
  return params;
}

EulerAngleVariables2RGBAux::EulerAngleVariables2RGBAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _sd(getParam<MooseEnum>("sd")),
    _output_type(getParam<MooseEnum>("output_type")),
    _phi1(coupledValue("phi1")),
    _phi(coupledValue("phi")),
    _phi2(coupledValue("phi2")),
    _phase(coupledValue("phase")),
    _sym(coupledValue("symmetry"))
{
}

Real
EulerAngleVariables2RGBAux::computeValue()
{
  // Call Euler2RGB Function to get RGB vector
  Point RGB = euler2RGB(_sd,
                        _phi1[0] / 180.0 * libMesh::pi,
                        _phi[0] / 180.0 * libMesh::pi,
                        _phi2[0] / 180.0 * libMesh::pi,
                        _phase[0],
                        _sym[0]);

  // Create correct scalar output
  if (_output_type < 3)
    return RGB(_output_type);
  else if (_output_type == 3)
  {
    Real RGBint = 0.0;
    for (unsigned int i = 0; i < 3; ++i)
      RGBint = 256 * RGBint + (RGB(i) >= 1 ? 255 : std::floor(RGB(i) * 256.0));

    return RGBint;
  }
  else
    mooseError("Incorrect value for output_type in EulerAngleVariables2RGBAux");
}
