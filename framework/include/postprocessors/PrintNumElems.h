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

#ifndef PRINTELEMS_H
#define PRINTELEMS_H

#include "GeneralPostprocessor.h"

//Forward Declarations
class PrintNumElems;

template<>
InputParameters validParams<PrintNumElems>();

class PrintNumElems : public GeneralPostprocessor
{
public:
  PrintNumElems(const std::string & name, InputParameters parameters);

  virtual void initialize() {}

  virtual void execute() {}

  /**
   * This will return the number of elements in the system
   */
  virtual Real getValue();
};

#endif //PRINTELEMS_H
