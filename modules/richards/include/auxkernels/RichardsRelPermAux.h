/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef RICHARDSRELPERMAUX_H
#define RICHARDSRELPERMAUX_H

#include "AuxKernel.h"

#include "RichardsRelPerm.h"

//Forward Declarations
class RichardsRelPermAux;

template<>
InputParameters validParams<RichardsRelPermAux>();

class RichardsRelPermAux: public AuxKernel
{
public:
  RichardsRelPermAux(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeValue();

  VariableValue & _seff_var;
  const RichardsRelPerm & _relperm_UO;
};

#endif // RICHARDSRELPERMAUX_H
