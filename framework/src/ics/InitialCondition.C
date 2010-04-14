#include "InitialCondition.h"

InitialCondition::InitialCondition(std::string name,
                                   MooseSystem & moose_system,
                                   InputParameters parameters)
  :_name(name),
   _moose_system(moose_system),
   _parameters(parameters),
   _var_name(parameters.get<std::string>("var_name"))
{}

  
