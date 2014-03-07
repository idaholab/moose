/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#include "GradParsedFunction.h"

template<>
InputParameters validParams<GradParsedFunction>()
{
  InputParameters params = validParams<MooseParsedFunction>();
  params += validParams<MooseParsedFunction>();
  params.addRequiredParam<RealVectorValue>("direction", "The direction in which to take the derivative.  This must not be a zero-length vector");
  return params;
}

GradParsedFunction::GradParsedFunction(const std::string & name, InputParameters parameters) :
    MooseParsedFunction(name, parameters),
    _direction(getParam<RealVectorValue>("direction"))
{
  _len = std::pow(_direction*_direction, 0.5);
  if (_len == 0)
    mooseError("The direction in the GradParsedFunction must have positive length.");
  _direction /= 2.0; // note - so we can do central differences
}

Real
GradParsedFunction::value(Real t, const Point & p)
{
  return (_function_ptr->evaluate<Real>(t, p + _direction) - _function_ptr->evaluate<Real>(t, p - _direction))/_len;
}
