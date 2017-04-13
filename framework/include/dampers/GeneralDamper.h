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

#ifndef GENERALDAMPER_H
#define GENERALDAMPER_H

// Moose Includes
#include "Damper.h"

// Forward Declarations
class GeneralDamper;
class SubProblem;
class SystemBase;
class MooseVariable;
class Assembly;

template <>
InputParameters validParams<GeneralDamper>();

/**
 * Base class for deriving general dampers
 */
class GeneralDamper : public Damper
{
public:
  GeneralDamper(const InputParameters & parameters);

  /**
   * Computes this Damper's damping
   */
  virtual Real computeDamping(const NumericVector<Number> & solution,
                              const NumericVector<Number> & update) = 0;
};

#endif // GENERALDAMPER_H
