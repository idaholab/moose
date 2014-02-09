/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef GRADPARSEDFUNCTION_H
#define GRADPARSEDFUNCTION_H

#include "MooseParsedFunction.h"

//Forward declarations
class GradParsedFunction;

template<>
InputParameters validParams<GradParsedFunction>();

class GradParsedFunction :
  public MooseParsedFunction
{
public:

  GradParsedFunction(const std::string & name, InputParameters parameters);

  virtual Real value(Real t, const Point & pt);

protected:

  RealVectorValue _direction;
  Real _len;

};
#endif //GRADPARSEDFUNCTION_H
