#include "InitialCondition.h"

InitialCondition::InitialCondition(std::string name,
                                   Parameters parameters,
                                   std::string var_name)
  :_name(name),
   _parameters(parameters),
   _var_name(var_name)
{}

  
