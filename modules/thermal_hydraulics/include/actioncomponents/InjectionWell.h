//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "WellBase.h"

/**
 * Adds the components and controls for an injection well.
 */
class InjectionWell : public WellBase
{
public:
  static InputParameters validParams();
  InjectionWell(const InputParameters & params);

protected:
  virtual void addTHMComponents() override;
  virtual void addControlLogic() override;

  /// Adds inlet component
  void addInlet();
  /// Inlet component name
  std::string inletName() const;
};
