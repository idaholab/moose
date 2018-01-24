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

#include "VectorPostprocessorAux.h"

template <>
InputParameters
validParams<VectorPostprocessorAux>()
{
  InputParameters params = validParams<AuxKernel>();

  params.addRequiredParam<VectorPostprocessorName>("vpp",
                                                   "The VectorPostprocessor to pull values out of");
  params.addRequiredParam<std::string>("vector", "The vector to use from the VectorPostprocessor");
  params.addRequiredParam<unsigned int>("index", "The entry in the VectorPostprocessor to use");

  return params;
}

VectorPostprocessorAux::VectorPostprocessorAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _vpp(getVectorPostprocessorValue("vpp", getParam<std::string>("vector"))),
    _index(getParam<unsigned int>("index"))
{
}

Real
VectorPostprocessorAux::computeValue()
{
  return _vpp[_index];
}
