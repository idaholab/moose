/* Should probably use FunctionIC for this purpose instead. */
#include "LinearIC.h"

template<>
InputParameters validParams<LinearIC>()
{
  InputParameters params = validParams<InitialCondition>();
  params.addRequiredParam<Real>("xA", "The 'left' endpoint");
  params.addRequiredParam<Real>("xB", "The 'right' endpoint");
  params.addRequiredParam<Real>("valueA", "The 'left' endpoint value");
  params.addRequiredParam<Real>("valueB", "The 'right' endpoint value");
  params.addRequiredParam<unsigned>("coordinate_direction", "0, 1, or 2: The direction in which the linear profile is aligned");
  return params;
}

LinearIC::LinearIC(const std::string & name,
                     InputParameters parameters)
  :InitialCondition(name, parameters),
   _xA(getParam<Real>("xA")),
   _xB(getParam<Real>("xB")),
   _valueA(getParam<Real>("valueA")),
   _valueB(getParam<Real>("valueB")),
   _coordinate_direction(getParam<unsigned>("coordinate_direction"))
{
  if (_coordinate_direction > 2)
    {
      std::cerr << "You must provide coordinate direction = 0, 1, or 2." << std::endl;
      libmesh_error(); 
    }
}

Real
LinearIC::value(const Point & p)
{
  // You must supply the value of your initial condition at
  // the provided point, p.
  Real x = p(_coordinate_direction);
  
  return _valueA + (_valueB - _valueA) / (_xB - _xA) * (x - _xA);
}
