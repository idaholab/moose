/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef RICHARDSSEFFPRIMEAUX_H
#define RICHARDSSEFFPRIMEAUX_H

#include "AuxKernel.h"

#include "RichardsSeff.h"

//Forward Declarations
class RichardsSeffPrimeAux;

template<>
InputParameters validParams<RichardsSeffPrimeAux>();

class RichardsSeffPrimeAux: public AuxKernel
{
public:
  RichardsSeffPrimeAux(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeValue();

  const RichardsSeff & _seff_UO;

  int _wrt1;

  std::vector<unsigned int> _pressure_vars;
  std::vector<VariableValue *> _pressure_vals;
};

#endif // RICHARDSSEFFPRIMEAUX_H
