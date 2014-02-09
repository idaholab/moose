/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef RICHARDSDENSITYPRIMEPRIMEAUX_H
#define RICHARDSDENSITYPRIMEPRIMEAUX_H

#include "AuxKernel.h"

#include "RichardsDensity.h"

//Forward Declarations
class RichardsDensityPrimePrimeAux;

template<>
InputParameters validParams<RichardsDensityPrimePrimeAux>();

class RichardsDensityPrimePrimeAux: public AuxKernel
{
public:
  RichardsDensityPrimePrimeAux(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeValue();

  VariableValue & _pressure_var;
  const RichardsDensity & _density_UO;
};

#endif // RICHARDSDENSITYPRIMEPRIMEAUX_H
