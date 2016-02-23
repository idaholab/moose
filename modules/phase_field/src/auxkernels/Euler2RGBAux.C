/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "Euler2RGBAux.h"
#include "Euler2RGB.h"

template<>
InputParameters validParams<Euler2RGBAux>()
{
  InputParameters params = validParams<AuxKernel>();
  MooseEnum sd_enum = MooseEnum("100=1 010=2 001=3", "001");
  params.addParam<MooseEnum>("sd", sd_enum, "Reference sample direction");
  params.addCoupledVar("phi1", "Euler angle 1");
  params.addCoupledVar("phi", "Euler angle 2");
  params.addCoupledVar("phi2", "Euler angle 3");
  params.addCoupledVar("phase", "Grain phase index");
  params.addCoupledVar("symmetry", "Grain symmetry indentifier");
  return params;
}

Euler2RGBAux::Euler2RGBAux(const InputParameters & parameters) :
    AuxKernel(parameters),
    _sd(getParam<MooseEnum>("sd")),
    _phi1(coupledValue("phi1")),
    _phi(coupledValue("phi")),
    _phi2(coupledValue("phi2")),
    _phase(coupledValue("phase")),
    _sym(coupledValue("symmetry"))
{
}

Real
Euler2RGBAux::computeValue()
{
  // Call Euler2RGB Function and output RGB value as an integer
  // (only for quadrature point "0" within the element)
  return Euler2RGB(_sd, _phi1[0] / 180.0 * libMesh::pi, _phi[0] / 180.0 * libMesh::pi, _phi2[0] / 180.0 * libMesh::pi, _phase[0], _sym[0]);
}
