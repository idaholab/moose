//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

/**
 * Testing object that just utilizes the value of a Postprocessor for the value of the Aux Variable
 */
class PostprocessorAux : public AuxKernel
{
public:
  static InputParameters validParams();

  PostprocessorAux(const InputParameters & parameters);

  virtual ~PostprocessorAux();

protected:
  virtual Real computeValue();

  const PostprocessorValue & _pp_val;
};
