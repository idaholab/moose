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

/**
 * Second derivative of fluid density wrt porepressure
 */
class RichardsDensityPrimePrimeAux: public AuxKernel
{
public:
  RichardsDensityPrimePrimeAux(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeValue();

  /// porepressure
  VariableValue & _pressure_var;

  /// userobject that defines density as a fcn of porepressure
  const RichardsDensity & _density_UO;
};

#endif // RICHARDSDENSITYPRIMEPRIMEAUX_H
