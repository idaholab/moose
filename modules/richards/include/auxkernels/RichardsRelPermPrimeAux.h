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

/**
 * Derivative of relative Permeability wrt effective saturation
 */
class RichardsRelPermPrimeAux: public AuxKernel
{
public:
  RichardsRelPermPrimeAux(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeValue();

  /// effective saturation
  VariableValue & _seff_var;

  /// userobject that defines relative permeability function
  const RichardsRelPerm & _relperm_UO;
};

#endif // RICHARDSRELPERMPRIMEAUX_H
