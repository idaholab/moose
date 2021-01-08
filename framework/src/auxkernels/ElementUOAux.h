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

// Forward Declarations
class ElementUOProvider;

/**
 * This AuxKernel retrieves values from a ElementUOProvider derived class and returns the reported
 * spatial values. This class throws an error if used on a Lagrange basis.
 */
class ElementUOAux : public AuxKernel
{
public:
  static InputParameters validParams();

  ElementUOAux(const InputParameters & params);

protected:
  virtual Real computeValue() override;

  const ElementUOProvider & _elem_uo;
  const std::string _field_name;
  const std::string _field_type;
};
