//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef INTERFACEUOPOSTPROCESSOR_H
#define INTERFACEUOPOSTPROCESSOR_H

#include "InterfaceUserObject.h"
#include "Postprocessor.h"

// Forward Declarations
class InterfaceUOPostprocessor;

template <>
InputParameters validParams<InterfaceUOPostprocessor>();

class InterfaceUOPostprocessor : public InterfaceUserObject, public Postprocessor
{
public:
  InterfaceUOPostprocessor(const InputParameters & parameters);
};

#endif // INTERFACEUOPOSTPROCESSOR_H
