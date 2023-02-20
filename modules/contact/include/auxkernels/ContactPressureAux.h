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

class NodalArea;
class PenetrationLocator;

/**
 * Computes the contact pressure from the contact force and nodal area
 */
class ContactPressureAux : public AuxKernel
{
public:
  static InputParameters validParams();

  ContactPressureAux(const InputParameters & parameters);

  virtual ~ContactPressureAux();

protected:
  virtual Real computeValue() override;

  /// AuxVariable containing the nodal area
  const VariableValue & _nodal_area;

  /// Number of contact pairs in the entire model
  const size_t _number_pairs;

  /// References to the PenetrationLocator objects for the individual interactions
  std::vector<const PenetrationLocator *> _penetration_locators;
};
