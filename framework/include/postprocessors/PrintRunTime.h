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

#ifndef PRINTRUNTIME_H
#define PRINTRUNTIME_H

#include "GeneralPostprocessor.h"

//Forward Declarations
class PrintRunTime;

template<>
InputParameters validParams<PrintRunTime>();

class PrintRunTime : public GeneralPostprocessor
{
public:
  PrintRunTime(const std::string & name, InputParameters parameters);

  virtual void initialize() {}
  virtual void execute() {}

  /**
   * This will return the elapsed wall time.
   */
  virtual Real getValue();

protected:
  MooseEnum _time_type;
};

#endif //PRINTRUNTIME_H
