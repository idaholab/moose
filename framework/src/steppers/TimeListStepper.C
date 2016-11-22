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

#include "TimeListStepper.h"
#include "Stepper.h"

template<>
InputParameters validParams<TimeListStepper>()
{
  InputParameters params = validParams<TimeListStepperBase>();
  params.addRequiredParam<std::vector<Real> >("time_list", "The values of t");
  params.addClassDescription("Solves the Transient problem at a list of given time points.");
  return params;
}

TimeListStepper::TimeListStepper(const InputParameters & parameters) :
    TimeListStepperBase(parameters)
{
  setupList(getParam<std::vector<Real> >("time_list"));
}
