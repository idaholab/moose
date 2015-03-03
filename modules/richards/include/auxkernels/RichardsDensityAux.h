/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef RICHARDSDENSITYAUX_H
#define RICHARDSDENSITYAUX_H

#include "AuxKernel.h"

#include "RichardsDensity.h"

//Forward Declarations
class RichardsDensityAux;

template<>
InputParameters validParams<RichardsDensityAux>();

/**
 * Fluid density as a function of porepressure
 */
class RichardsDensityAux: public AuxKernel
{
public:
  RichardsDensityAux(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeValue();

  /// porepressure
  VariableValue & _pressure_var;

  /// userobject that defines density as a fcn of porepressure
  const RichardsDensity & _density_UO;
};

#endif // RICHARDSDENSITYAUX_H
