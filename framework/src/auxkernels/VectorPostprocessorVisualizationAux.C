//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorPostprocessorVisualizationAux.h"

registerMooseObject("MooseApp", VectorPostprocessorVisualizationAux);

template <>
InputParameters
validParams<VectorPostprocessorVisualizationAux>()
{
  InputParameters params = validParams<AuxKernel>();

  params.addClassDescription("Read values from a VectorPostprocessor that is producing vectors "
                             "that are 'number of processors' * in length.  Puts the value for "
                             "each processor into an elemental auxiliary field.");

  params.addRequiredParam<VectorPostprocessorName>(
      "vpp", "The name of the VectorPostprocessor to pull the data from.");
  params.addRequiredParam<std::string>(
      "vector_name", "The name of the vector to use from the VectorPostprocessor");

  return params;
}

VectorPostprocessorVisualizationAux::VectorPostprocessorVisualizationAux(
    const InputParameters & parameters)
  : AuxKernel(parameters),
    _vpp_vector(getVectorPostprocessorValue("vpp", getParam<std::string>("vector_name"))),
    _my_pid(processor_id())
{
}

void
VectorPostprocessorVisualizationAux::timestepSetup()
{
  if (_vpp_vector.size() != n_processors())
    mooseError("Error in VectorPostprocessor ",
               name(),
               ". Vector ",
               getParam<std::string>("vector_name"),
               " in VectorPostprocessor ",
               getParam<VectorPostprocessorName>("vpp"),
               " does not contain num_procs number of entries.  num_procs: ",
               n_processors(),
               " num_entries: ",
               _vpp_vector.size());
}

Real
VectorPostprocessorVisualizationAux::computeValue()
{
  return _vpp_vector[_my_pid];
}
