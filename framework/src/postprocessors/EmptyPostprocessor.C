#include "EmptyPostprocessor.h"

#include "MooseSystem.h"

// libMesh includes
#include "parallel.h"

template<>
InputParameters validParams<EmptyPostprocessor>()
{
  InputParameters params = validParams<MooseObject>();
  return params;
}

EmptyPostprocessor::EmptyPostprocessor(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :Postprocessor(name, moose_system, parameters)
{}




