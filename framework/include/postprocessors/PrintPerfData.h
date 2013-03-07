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

#ifndef PRINTPERFDATA_H
#define PRINTPERFDATA_H

#include "GeneralPostprocessor.h"

//Forward Declarations
class PrintPerfData;

template<>
InputParameters validParams<PrintPerfData>();

class PrintPerfData : public GeneralPostprocessor
{
public:
  PrintPerfData(const std::string & name, InputParameters parameters);

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

#endif //PRINTPERFDATA_H
