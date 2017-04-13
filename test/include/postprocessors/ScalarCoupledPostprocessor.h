/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef SCALARCOUPLEDPOSTPROCESSOR_H
#define SCALARCOUPLEDPOSTPROCESSOR_H

// MOOSE includes
#include "SideIntegralPostprocessor.h"

class ScalarCoupledPostprocessor;

template <>
InputParameters validParams<ScalarCoupledPostprocessor>();

/**
* This postprocessor demonstrates coupling a scalar variable to a postprocessor
*/
class ScalarCoupledPostprocessor : public SideIntegralPostprocessor
{
public:
  ScalarCoupledPostprocessor(const InputParameters & parameters);

protected:
  Real computeQpIntegral();

  const VariableValue & _coupled_scalar;
  const VariableValue & _u;
};

#endif // SCALARCOUPLEDPOSTPROCESSOR_H
