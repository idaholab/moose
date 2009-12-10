#include "RandomIC.h"

template<>
InputParameters validParams<RandomIC>()
{
  InputParameters params;
  params.addRequiredParam<Real>("min", "Lower bound of the randomly generated values");
  params.addRequiredParam<Real>("max", "Upper bound of the randomly generated values");
  return params;
}

RandomIC::RandomIC(std::string name,
                       InputParameters parameters,
                       std::string var_name)
  :InitialCondition(name,parameters,var_name),
   _min(parameters.get<Real>("min")),
   _max(parameters.get<Real>("max")),
   _range(_max - _min)
{
  mooseAssert(_range > 0.0, "Min > Max for RandomIC!");
}

Real
RandomIC::value(const Point & p)
{
  //Random number between 0 and 1
  Real rand_num = (Real)rand() / (Real)RAND_MAX;

  //Between 0 and range
  rand_num *= _range;

  //Between min and max
  rand_num += _min;

  return rand_num;
}

  


  
