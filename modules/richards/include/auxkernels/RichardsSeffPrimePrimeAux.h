/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef RICHARDSSEFFPRIMEPRIMEAUX_H
#define RICHARDSSEFFPRIMEPRIMEAUX_H

#include "AuxKernel.h"

#include "RichardsSeff.h"

//Forward Declarations
class RichardsSeffPrimePrimeAux;

template<>
InputParameters validParams<RichardsSeffPrimePrimeAux>();

class RichardsSeffPrimePrimeAux: public AuxKernel
{
public:
  RichardsSeffPrimePrimeAux(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeValue();

  const RichardsSeff & _seff_UO;

  int _wrt1;
  int _wrt2;

  std::vector<unsigned int> _pressure_vars;
  std::vector<VariableValue *> _pressure_vals;
};

#endif // RICHARDSSEFFPRIMEPRIMEAUX_H
