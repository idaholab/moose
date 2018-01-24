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

#ifndef DAMPER_H
#define DAMPER_H

// Moose Includes
#include "MooseObject.h"
#include "SetupInterface.h"
#include "Restartable.h"
#include "MeshChangedInterface.h"

// Forward Declarations
class Damper;
class SubProblem;
class SystemBase;

template <>
InputParameters validParams<Damper>();

/**
 * Base class for deriving dampers
 */
class Damper : public MooseObject,
               public SetupInterface,
               public Restartable,
               public MeshChangedInterface
{
public:
  Damper(const InputParameters & parameters);

  /**
   * Check whether damping is below the user-specified minimum value,
   * and throw an exception if it is.
   * @param cur_damping The computed damping to be checked against that minimum
   */
  void checkMinDamping(const Real cur_damping) const;

protected:
  SubProblem & _subproblem;
  SystemBase & _sys;

  /// Minimum allowable value of damping
  const Real & _min_damping;
};

#endif
