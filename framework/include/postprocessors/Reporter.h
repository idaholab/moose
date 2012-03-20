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

#ifndef REPORTER_H
#define REPORTER_H

#include "GeneralPostprocessor.h"

//Forward Declarations
class Reporter;

template<>
InputParameters validParams<Reporter>();

class Reporter : public GeneralPostprocessor
{
public:
  Reporter(const std::string & name, InputParameters parameters);

  virtual void initialize() {}
  virtual void execute() {}

  virtual Real getValue();

protected:
  PostprocessorValue & _my_value;
};

#endif //REPORTER_H
