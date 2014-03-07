/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef RICHARDSRELPERMPRIMEPRIMEAUX_H
#define RICHARDSRELPERMPRIMEPRIMEAUX_H

#include "AuxKernel.h"

#include "RichardsRelPerm.h"

//Forward Declarations
class RichardsRelPermPrimePrimeAux;

template<>
InputParameters validParams<RichardsRelPermPrimePrimeAux>();

class RichardsRelPermPrimePrimeAux: public AuxKernel
{
public:
  RichardsRelPermPrimePrimeAux(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeValue();

  VariableValue & _seff_var;
  const RichardsRelPerm & _relperm_UO;
};

#endif // RICHARDSRELPERMPRIMEPRIMEAUX_H
