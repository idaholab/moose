//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalVariablePostprocessor.h"

/**
 * Computes the id of the node with the maximum value across all processes
 */
class NodalMaxValueId : public NodalVariablePostprocessor
{
public:
  static InputParameters validParams();

  NodalMaxValueId(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual Real getValue() override;
  virtual void finalize() override;

  /**
   * The method called to compute the value that will be returned
   * by the proxy value.
   */
  virtual Real computeValue();

  void threadJoin(const UserObject & y) override;

protected:
  Real _value;
  dof_id_type _node_id;
};
