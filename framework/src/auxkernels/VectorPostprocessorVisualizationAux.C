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

InputParameters
VectorPostprocessorVisualizationAux::validParams()
{
  InputParameters params = AuxKernel::validParams();

  params.addClassDescription("Read values from a VectorPostprocessor that is producing vectors "
                             "that are 'number of processors' * in length.  Puts the value for "
                             "each processor into an elemental auxiliary field.");

  params.addRequiredParam<VectorPostprocessorName>(
      "vpp", "The name of the VectorPostprocessor to pull the data from.");
  params.addRequiredParam<std::string>(
      "vector_name", "The name of the vector to use from the VectorPostprocessor");

  params.addParam<bool>("use_broadcast",
                        false,
                        "Causes this AuxKernel to use a broadcasted version of the vector instead "
                        "of a scattered version of the vector (the default).  This is slower - but "
                        "is useful for debugging and testing");

  return params;
}

VectorPostprocessorVisualizationAux::VectorPostprocessorVisualizationAux(
    const InputParameters & parameters)
  : AuxKernel(parameters),
    _use_broadcast(getParam<bool>("use_broadcast")),
    _vpp_scatter(getScatterVectorPostprocessorValue("vpp", getParam<std::string>("vector_name"))),
    _vpp_vector(
        getVectorPostprocessorValue("vpp", getParam<std::string>("vector_name"), _use_broadcast)),
    _my_pid(processor_id())
{
}

void
VectorPostprocessorVisualizationAux::timestepSetup()
{
  if (_my_pid == 0 && _vpp_vector.size() != n_processors())
    mooseError("Error in AuxKernel ",
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
  if (_use_broadcast)
  {
    mooseAssert(_vpp_vector.size() > _my_pid,
                "Vector does not contain enough entries in VectorPostprocessorVisualization named "
                    << name());
    return _vpp_vector[_my_pid];
  }
  return _vpp_scatter;
}
