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

class RichardsSatPrimeAux: public AuxKernel
{
public:
  RichardsSatPrimeAux(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeValue();

  VariableValue & _seff_var;
  const RichardsSat & _sat_UO;
};

#endif // RICHARDSSATPRIMEAUX_H
