//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalUserObject.h"

namespace libMesh
{
class Elem;
class QBase;
}

class UndisplacedMeshUpdater : public NodalUserObject
{
public:
  static InputParameters validParams();

  UndisplacedMeshUpdater(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject &) override {}

protected:
  const unsigned int _n_vars;
  const std::vector<const VariableValue *> _input_variables;
  std::vector<MooseWritableVariable *> _output_variables;
  /// Compute the value used in the criterion
  virtual Real computeValue();

  /// Threshold to modify the element subdomain ID
  const Real _threshold;

  /// Criterion type
  const enum class CriterionType { Below, Equal, Above } _criterion_type;
  const VariableValue & _criterion_variable;
};
