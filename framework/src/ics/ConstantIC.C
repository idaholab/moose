#include "ConstantIC.h"

template<class ConstantIC>
InputParameters valid_params()
{
  InputParameters params;
  params.set<Real>("value") = 0.0;
  return params;
}

ConstantIC::ConstantIC(std::string name,
                       InputParameters parameters,
                       std::string var_name)
  :InitialCondition(name,parameters,var_name),
   _value(parameters.get<Real>("value"))
{}

Real
ConstantIC::value(const Point & p)
{
  return _value;
}

  


  
