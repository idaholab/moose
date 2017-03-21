/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef RICHARDSMASS_H
#define RICHARDSMASS_H

#include "ElementIntegralVariablePostprocessor.h"
#include "RichardsVarNames.h"

// Forward Declarations
class RichardsMass;

template <>
InputParameters validParams<RichardsMass>();

/**
 * This postprocessor computes the fluid mass by integrating the density over the volume
 *
 */
class RichardsMass : public ElementIntegralVariablePostprocessor
{
public:
  RichardsMass(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral();

  /// userobject that holds Richards variable names
  const RichardsVarNames & _richards_name_UO;

  /// Richards variable number that we want the mass for
  unsigned int _pvar;

  /// Mass, or vector of masses in multicomponent situation
  const MaterialProperty<std::vector<Real>> & _mass;
};

#endif
