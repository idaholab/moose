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
 * Adds the components and controls for a production well.
 */
class ProductionWell : public WellBase
{
public:
  static InputParameters validParams();
  ProductionWell(const InputParameters & params);

protected:
  virtual void addTHMComponents() override;
  virtual void addControlLogic() override;

  /// Adds outlet component
  void addOutlet();
  /// Outlet component name
  std::string outletName() const;
};
