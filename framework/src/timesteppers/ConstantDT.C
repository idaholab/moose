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

#include "ConstantDT.h"
#include "FEProblem.h"
#include "Transient.h"

template<>
InputParameters validParams<ConstantDT>()
{
  InputParameters params = validParams<TimeStepper>();
  params.addRequiredParam<Real>("dt", "Size of the time step");
  return params;
}

ConstantDT::ConstantDT(const std::string & name, InputParameters parameters) :
    TimeStepper(name, parameters)
{}

void
ConstantDT::computeInitialDT()
{
  _current_dt = getParam<Real>("dt");
}

void
ConstantDT::computeDT()
{
  // Nothing to do because _current_dt should just be left alone
}
