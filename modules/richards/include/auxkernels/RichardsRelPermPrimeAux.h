/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef RICHARDSRELPERMPRIMEAUX_H
#define RICHARDSRELPERMPRIMEAUX_H

#include "AuxKernel.h"

#include "RichardsRelPerm.h"

//Forward Declarations
class RichardsRelPermPrimeAux;

template<>
InputParameters validParams<RichardsRelPermPrimeAux>();

class RichardsRelPermPrimeAux: public AuxKernel
{
public:
  RichardsRelPermPrimeAux(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeValue();

  VariableValue & _seff_var;
  const RichardsRelPerm & _relperm_UO;
};

#endif // RICHARDSRELPERMPRIMEAUX_H
