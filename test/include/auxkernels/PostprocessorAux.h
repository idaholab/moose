//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POSTPROCESSORAUX_H
#define POSTPROCESSORAUX_H

#include "AuxKernel.h"

// Forward Declarations
class PostprocessorAux;

template <>
InputParameters validParams<PostprocessorAux>();

/**
 * Testing object that just utilizes the value of a Postprocessor for the value of the Aux Variable
 */
class PostprocessorAux : public AuxKernel
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  PostprocessorAux(const InputParameters & parameters);

  virtual ~PostprocessorAux();

protected:
  virtual Real computeValue();

  const PostprocessorValue & _pp_val;
};

#endif // POSTPROCESSORAUX_H
