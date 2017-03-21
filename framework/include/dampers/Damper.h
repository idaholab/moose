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

protected:
  SubProblem & _subproblem;
  SystemBase & _sys;
};

#endif
