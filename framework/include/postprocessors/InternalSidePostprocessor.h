//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef INTERNALSIDEPOSTPROCESSOR_H
#define INTERNALSIDEPOSTPROCESSOR_H

#include "InternalSideUserObject.h"
#include "Postprocessor.h"

// Forward Declarations
class InternalSidePostprocessor;

template <>
InputParameters validParams<InternalSidePostprocessor>();

class InternalSidePostprocessor : public InternalSideUserObject, public Postprocessor
{
public:
  InternalSidePostprocessor(const InputParameters & parameters);
};

#endif
