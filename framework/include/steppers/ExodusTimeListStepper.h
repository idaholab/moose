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

#ifndef EXODUSTIMELISTSTEPPER_H
#define EXODUSTIMELISTSTEPPER_H

#include "TimeListStepperBase.h"

class ExodusTimeListStepper;

template<>
InputParameters validParams<ExodusTimeListStepper>();

/**
 * Solves the PDEs at a list of time points given as a vector in the input file.
 * Adjusts the time list vector according to Transient start_time and end_time.
 */
class ExodusTimeListStepper : public TimeListStepperBase
{
public:
  ExodusTimeListStepper(const InputParameters & parameters);

protected:
  /// The ExodusII file that is being read
  std::string _mesh_file;
  std::vector<Real> _times;
};

#endif //EXODUSTIMELISTSTEPPER_H
