//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "BoundaryBase.h"
#include "Component2D.h"

/**
 * Base class for heat structure boundary components
 */
class HSBoundary : public BoundaryBase
{
public:
  HSBoundary(const InputParameters & params);

  virtual void check() const override;

protected:
  /**
   * Returns true if all of the boundaries are external.
   *
   * This method should only be called if the heat structure is known to be 2D.
   */
  bool allComponent2DBoundariesAreExternal() const;

  /**
   * Logs an error if any boundary is not external.
   *
   * This method should only be called if the heat structure is known to be 2D.
   */
  void checkAllComponent2DBoundariesAreExternal() const;

  /**
   * Returns true if all of the boundaries have the same external boundary type.
   *
   * This method should only be called if the heat structure is known to be 2D.
   */
  bool hasCommonComponent2DExternalBoundaryType() const;

  /**
   * Gets the common external boundary type.
   *
   * This method should only be called if the heat structure is known to be 2D,
   * and it is known that there is a common type.
   */
  Component2D::ExternalBoundaryType getCommonComponent2DExternalBoundaryType() const;

  /// Boundary names for which the boundary component applies
  const std::vector<BoundaryName> & _boundary;

  /// Heat structure name
  const std::string & _hs_name;

public:
  static InputParameters validParams();
};
