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

#ifndef RESIDUAL_H
#define RESIDUAL_H

#include "GeneralPostprocessor.h"

//Forward Declarations
class Residual;

template<>
InputParameters validParams<Residual>();

class Residual : public GeneralPostprocessor
{
public:
  Residual(const std::string & name, InputParameters parameters);

  virtual void initialize() {}
  virtual void execute() {}

  /**
   * This will return the final nonlinear residual.
   */
  virtual Real getValue();
};

#endif // RESIDUAL_H
