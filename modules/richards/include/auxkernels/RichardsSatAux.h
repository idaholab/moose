/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef RICHARDSSATAUX_H
#define RICHARDSSATAUX_H

#include "AuxKernel.h"

#include "RichardsSat.h"

//Forward Declarations
class RichardsSatAux;

template<>
InputParameters validParams<RichardsSatAux>();

class RichardsSatAux: public AuxKernel
{
public:
  RichardsSatAux(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeValue();

  VariableValue & _seff_var;
  const RichardsSat & _sat_UO;
};

#endif // RICHARDSSATAUX_H
