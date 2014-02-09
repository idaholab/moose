/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef RICHARDSEXCAV
#define RICHARDSEXCAV

#include "NodalBC.h"

// Forward Declarations
class RichardsExcav;
class Function;

template<>
InputParameters validParams<RichardsExcav>();

class RichardsExcav : public NodalBC
{
public:

  RichardsExcav(const std::string & name,
                        InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  virtual bool shouldApply();

  Real _p_excav;
  Function & _func;
};

#endif //RICHARDSEXCAV

