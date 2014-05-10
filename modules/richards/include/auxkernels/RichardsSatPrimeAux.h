/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef RICHARDSSATPRIMEAUX_H
#define RICHARDSSATPRIMEAUX_H

#include "AuxKernel.h"

#include "RichardsSat.h"

//Forward Declarations
class RichardsSatPrimeAux;

template<>
InputParameters validParams<RichardsSatPrimeAux>();

/**
 * Derivative of fluid Saturation wrt effective saturation
 */
class RichardsSatPrimeAux: public AuxKernel
{
public:
  RichardsSatPrimeAux(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeValue();

  /// effective saturation
  VariableValue & _seff_var;

  /// User object defining saturation as a function of effective saturation
  const RichardsSat & _sat_UO;
};

#endif // RICHARDSSATPRIMEAUX_H
