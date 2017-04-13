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

#ifndef NUMVARS_H
#define NUMVARS_H

#include "GeneralPostprocessor.h"

// Forward Declarations
class NumVars;

template <>
InputParameters validParams<NumVars>();

class NumVars : public GeneralPostprocessor
{
public:
  NumVars(const InputParameters & parameters);

  virtual void initialize() override {}

  virtual void execute() override {}

  virtual Real getValue() override;

protected:
  MooseEnum _system;
};

#endif // NUMVARS_H
