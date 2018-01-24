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

#ifndef EXODUSTIMESEQUENCESTEPPER_H
#define EXODUSTIMESEQUENCESTEPPER_H

#include "TimeSequenceStepperBase.h"

class ExodusTimeSequenceStepper;

template <>
InputParameters validParams<ExodusTimeSequenceStepper>();

/**
 * Solves the PDEs at a sequence of time points given as a vector in the input file.
 * Adjusts the time sequence vector according to Transient start_time and end_time.
 */
class ExodusTimeSequenceStepper : public TimeSequenceStepperBase
{
public:
  ExodusTimeSequenceStepper(const InputParameters & parameters);

protected:
  /// The ExodusII file that is being read
  std::string _mesh_file;
};

#endif // EXODUSTIMESEQUENCESTEPPER_H
