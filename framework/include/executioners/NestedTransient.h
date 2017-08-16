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

#ifndef NESTEDTRANSIENT_H
#define NESTEDTRANSIENT_H

#include "Transient.h"

// System includes
#include <string>
#include <fstream>

// Forward Declarations
class NestedTransient;
// class TimeStepper;
// class FEProblemBase;

template <>
InputParameters validParams<NestedTransient>();

/**
 * Transient executioners usually loop through a number of timesteps... calling solve()
 * for each timestep.
 */
class NestedTransient : public Transient
{
public:
  /**
   * Constructor
   *
   * @param parameters The parameters object holding data for the class to use.
   * @return Whether or not the solve was successful.
   */
  NestedTransient(const InputParameters & parameters);

  /**
   * Transient loop will continue as long as this keeps returning true.
   */
  virtual bool keepGoing() override;

  /**
   * Whether or not the last solve converged.
   */

  /**
   * This is where the solve step is actually incremented.
   */
  virtual void incrementStepOrReject() override;

  virtual void acceptanceCheck(Real current_dt) override;

  virtual void endStep(Real input_time = -1.0) override;

protected:
  /**
   * This should execute the solve for one timestep.
   */
  /// Here for backward compatibility

  /// Whether step should be repeated due to xfem modifying the mesh
  bool _xfem_repeat_step;
  unsigned int _xfem_update_count;
  unsigned int _max_xfem_update;

  /**
   * Picard Related
   */
  /// Number of Picard iterations to perform
};

#endif // NESTEDTRANSIENT_H
