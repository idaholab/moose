/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef RICHARDSSEFFAUX_H
#define RICHARDSSEFFAUX_H

#include "AuxKernel.h"

#include "RichardsSeff.h"

//Forward Declarations
class RichardsSeffAux;

template<>
InputParameters validParams<RichardsSeffAux>();

class RichardsSeffAux: public AuxKernel
{
public:
  RichardsSeffAux(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeValue();

  const RichardsSeff & _seff_UO;

  std::vector<unsigned int> _pressure_vars;
  std::vector<VariableValue *> _pressure_vals;
};

#endif // RICHARDSSEFFAUX_H
