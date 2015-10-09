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

#ifndef MAXVARNDOFSPERELEMPP_H
#define MAXVARNDOFSPERELEMPP_H

// MOOSE includes
#include "GeneralPostprocessor.h"

// Forward declerations
class MaxVarNDofsPerElemPP;

template<>
InputParameters validParams<GeneralPostprocessor>();

/**
 * Testing object to make sure the maximum number of n dofs per element is computed properly
 */
class MaxVarNDofsPerElemPP : public GeneralPostprocessor
{
public:
  MaxVarNDofsPerElemPP(const InputParameters & parameters);

  virtual void initialize() {}
  virtual void execute() {}

  virtual PostprocessorValue getValue();
};

#endif //MAXVARNDOFSPERELEMPP_H
