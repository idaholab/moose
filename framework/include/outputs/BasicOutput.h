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

#ifndef BASICOUTPUT_H
#define BASICOUTPUT_H

// MOOSE includes
#include "MooseTypes.h"

// Forward declarations
class InputParameters;

/**
 * Based class for output objects
 *
 * Each output class (e.g., Exodus) should inherit from this base class. At a minimum, the pure
 * virtual methods for the various types of output must be defined in the child class.
 *
 * There are four possible base classes for this method: Output, PetscOutput, FileOutput,
 * OversampleOutput that
 * are explicitly instatiated in the source file.
 *
 * @see Exodus Console CSV
 */
template <class T>
class BasicOutput : public T
{
public:
  /**
   * Class constructor
   *
   * The constructor performs all of the necessary initialization of the various
   * output lists required for the various output types.
   *
   * @see initAvailable init separate
   */
  BasicOutput(const InputParameters & parameters) : T(parameters) {}

protected:
  /**
   * Overload to call the output() method at the correct time
   */
  virtual void outputStep(const ExecFlagType & type);

  /**
   * Overload this function with the desired output activities
   */
  virtual void output(const ExecFlagType & type) = 0;
};

#endif /* BASICOUTPUT_H */
