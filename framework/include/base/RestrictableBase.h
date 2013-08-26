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

#ifndef RESTRICTABLEBASE_H
#define RESTRICTABLEBASE_H

#include "InputParameters.h"
#include "MooseTypes.h"
#include "FEProblem.h"

/**
 * A base class for creating restricted objects
 * \see BlockRestrictable BoundaryRestrictable
 */
class RestrictableBase
{
public:

  /**
   * Class constructor
   * Populates the FEProblem and MooseMesh pointers
   */
  RestrictableBase(InputParameters & parameters);

  /**
   * Emtpy destructor
   */
  virtual ~RestrictableBase();

protected:

  /// Pointer to the FEProblem class
  FEProblem * _r_feproblem;

  /// Pointer to the MooseMesh class
  MooseMesh * _r_mesh;
};

#endif // RESTRICTABLEBASE
