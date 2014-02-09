/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef RICHARDSDENSITYAUX_H
#define RICHARDSDENSITYAUX_H

#include "AuxKernel.h"

#include "RichardsDensity.h"

//Forward Declarations
class RichardsDensityAux;

template<>
InputParameters validParams<RichardsDensityAux>();

class RichardsDensityAux: public AuxKernel
{
public:
  RichardsDensityAux(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeValue();

  VariableValue & _pressure_var;
  const RichardsDensity & _density_UO;
};

#endif // RICHARDSDENSITYAUX_H
