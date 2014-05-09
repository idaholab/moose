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

/**
 * Relative Permeability as a function of effective saturation
 */
class RichardsRelPermPrimePrimeAux: public AuxKernel
{
public:
  RichardsRelPermPrimePrimeAux(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeValue();

  /// effective saturation
  VariableValue & _seff_var;

  /// userobject that defines relative permeability function
  const RichardsRelPerm & _relperm_UO;
};

#endif // RICHARDSRELPERMPRIMEPRIMEAUX_H
