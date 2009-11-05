#include "ConstantIC.h"

template<class ConstantIC>
Parameters valid_params()
{
  Parameters params;
  params.set<Real>("value") = 0.0;
  return params;
}

ConstantIC::ConstantIC(std::string name,
                       Parameters parameters,
                       std::string var_name)
  :InitialCondition(name,parameters,var_name),
   _value(parameters.get<Real>("value"))
{}

Real
ConstantIC::value(const Point & p)
{
  return _value;
}

  


  
