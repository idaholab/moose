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

#ifndef PERFORMANCEDATA_H
#define PERFORMANCEDATA_H

#include "GeneralPostprocessor.h"

//Forward Declarations
class PerformanceData;

template<>
InputParameters validParams<PerformanceData>();

class PerformanceData : public GeneralPostprocessor
{
public:
  PerformanceData(const std::string & name, InputParameters parameters);

  virtual void initialize() {}
  virtual void execute() {}

  /**
   * This will return the elapsed wall time.
   */
  virtual Real getValue();

protected:
  MooseEnum _column;

  std::string _event;
};

#endif // PERFORMANCEDATA_H
