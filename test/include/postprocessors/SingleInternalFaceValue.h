//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InternalSidePostprocessor.h"

/**
 * Compute an internal face value for a single side of a single element
 */
class SingleInternalFaceValue : public InternalSidePostprocessor
{
public:
  static InputParameters validParams();

  SingleInternalFaceValue(const InputParameters & parameters);

protected:
  virtual void initialize() override { _value = 0; };
  virtual void execute() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & y) override;
  virtual PostprocessorValue getValue() const override { return _value; };

private:
  Real _value;
  const dof_id_type _element_id;
  const unsigned int _side_index;
  const MooseEnum _state;
  const VariableValue & _value_current;
  const VariableValue & _value_old;
  const VariableValue & _value_older;
  const VariableValue & _neighbor_value;
  const VariableValue & _neighbor_value_old;
  const VariableValue & _neighbor_value_older;
};
