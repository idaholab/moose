#include "SidePostprocessor.h"

template<>
InputParameters validParams<SidePostprocessor>()
{
  InputParameters params = validParams<BoundaryCondition>();
  params += validParams<Postprocessor>();
  return params;
}

SidePostprocessor::SidePostprocessor(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :BoundaryCondition(name, moose_system, parameters),
   Postprocessor(name, moose_system, parameters)
{}
