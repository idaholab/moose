//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DomainUserObject.h"

class Function;

class InterfaceDomainUserObject : public DomainUserObject
{
public:
  static InputParameters validParams();

  InterfaceDomainUserObject(const InputParameters & parameters);

  void initialize() override;
  void executeOnElement() override;
  void executeOnBoundary() override;
  void executeOnInterface() override;
  void finalize() override;
  void threadJoin(const UserObject & y) override;

protected:
  const VariableValue & _u;
  const VariableValue & _v_neighbor;
  const VariableGradient & _grad_u;
  const VariableGradient & _grad_v_neighbor;
  MooseVariable & _var;
  const MooseVariableFieldBase & _v_var;
  const VariableTestValue & _test;
  const VariableTestGradient & _grad_test;
  const VariableTestValue & _test_face;
  const VariableTestGradient & _grad_test_face;
  const VariableTestValue & _test_face_neighbor;
  const VariableTestGradient & _grad_test_face_neighbor;
  const Function & _func;
  const Real _robin_coef;
  const Real _interface_penalty;
  const Real _nl_abs_tol;
  std::set<BoundaryID> _robin_bnd_ids;
  std::vector<Real> _nodal_integrals;
};
