//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef PROCESSORIDAUX_H
#define PROCESSORIDAUX_H

#include "AuxKernel.h"

// Forward Declarations
class ProcessorIDAux;

template <>
InputParameters validParams<ProcessorIDAux>();

class ProcessorIDAux : public AuxKernel
{
public:
  ProcessorIDAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;
};

#endif // PROCESSORIDAUX_H
