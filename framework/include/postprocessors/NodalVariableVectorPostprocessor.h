//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NODALVARIABLEVECTORPOSTPROCESSOR_H
#define NODALVARIABLEVECTORPOSTPROCESSOR_H

#include "NodalVectorPostprocessor.h"

// Forward Declarations
class NodalVariableVectorPostprocessor;

template <>
InputParameters validParams<NodalVariableVectorPostprocessor>();

/**
 * Base class VectorPostprocessors operating on nodal variables.
 */
class NodalVariableVectorPostprocessor : public NodalVectorPostprocessor
{
public:
  NodalVariableVectorPostprocessor(const InputParameters & parameters);
};

#endif
