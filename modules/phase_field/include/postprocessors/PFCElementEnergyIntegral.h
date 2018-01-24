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

#ifndef PFCELEMENTENERGYINTEGRAL_H
#define PFCELEMENTENERGYINTEGRAL_H

#include "ElementIntegralPostprocessor.h"
#include "MooseVariableInterface.h"

// Forward Declarations
class PFCElementEnergyIntegral;

template <>
InputParameters validParams<PFCElementEnergyIntegral>();

/**
 * Compute a volume integral of the specified variable.
 *
 * Note that specializations of this integral are possible by deriving from this
 * class and overriding computeQpIntegral().
 */
class PFCElementEnergyIntegral : public ElementIntegralPostprocessor, public MooseVariableInterface
{
public:
  PFCElementEnergyIntegral(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral();

  MooseVariable & _var;

  /// Holds the solution at current quadrature points
  const VariableValue & _u;

  /// Holds the solution gradient at the current quadrature points
  const VariableGradient & _grad_u;

  /// Holds the solution derivative at the current quadrature points
  const VariableValue & _u_dot;

  /// Temperature
  const Real _temp;
};

#endif
