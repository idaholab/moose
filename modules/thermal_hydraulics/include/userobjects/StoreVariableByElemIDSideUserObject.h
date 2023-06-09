//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideUserObject.h"

/**
 * Stores variable values at each quadrature point on a side by element ID.
 */
class StoreVariableByElemIDSideUserObject : public SideUserObject
{
public:
  static InputParameters validParams();

  StoreVariableByElemIDSideUserObject(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & y) override;

  /**
   * Gets the variable values at each quadrature point on the provided element
   *
   * @param[in] elem_id  Element ID at which to get variable values
   */
  const std::vector<ADReal> & getVariableValues(dof_id_type elem_id) const;

protected:
  /// Map of element ID to variable values at each side quadrature point
  std::map<dof_id_type, std::vector<ADReal>> _elem_id_to_var_values;

  /// Variable value
  const ADVariableValue & _u;
};
