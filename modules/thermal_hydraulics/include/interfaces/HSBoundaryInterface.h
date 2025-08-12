//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InputParameters.h"
#include "HeatStructureBase.h"
#include "LoggingInterface.h"

class Component;

/**
 * Interface class for coupling to a heat structure boundary
 */
class HSBoundaryInterface
{
public:
  static InputParameters validParams();

  HSBoundaryInterface(Component * component);

  void check(const Component * const component) const;

protected:
  /**
   * Returns true if the specified heat structure boundary is valid.
   */
  bool HSBoundaryIsValid(const Component * const component) const;

  /**
   * Gets the boundary name corresponding to the heat structure and side
   *
   * @param[in] component   Component pointer
   */
  const BoundaryName & getHSBoundaryName(const Component * const component) const;
  /**
   * Gets the external boundary type for the coupled heat structure boundary
   *
   * @param[in] component   Component pointer
   */
  Component2D::ExternalBoundaryType
  getHSExternalBoundaryType(const Component * const component) const;

  /// Heat structure name
  const std::string & _hs_name;
  // TODO: make the following parameters private after transition to use above APIs
  /// Heat structure side
  Component2D::ExternalBoundaryType _hs_side;
  /// True if the heat structure side enum is valid
  bool _hs_side_valid;
};
