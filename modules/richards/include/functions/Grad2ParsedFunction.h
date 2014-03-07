/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef GRAD2PARSEDFUNCTION_H
#define GRAD2PARSEDFUNCTION_H

#include "MooseParsedFunction.h"

//Forward declarations
class Grad2ParsedFunction;

template<>
InputParameters validParams<Grad2ParsedFunction>();

class Grad2ParsedFunction :
  public MooseParsedFunction
{
public:

  Grad2ParsedFunction(const std::string & name, InputParameters parameters);

  virtual Real value(Real t, const Point & pt);

protected:

  RealVectorValue _direction;
  Real _len2;

};
#endif //GRAD2PARSEDFUNCTION_H
