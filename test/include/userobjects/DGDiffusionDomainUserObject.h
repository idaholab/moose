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

class DGDiffusionDomainUserObject : public DomainUserObject
{
public:
  static InputParameters validParams();

  DGDiffusionDomainUserObject(const InputParameters & parameters);

  void initialize() override;
  void executeOnElement() override;
  void executeOnBoundary() override;
  void executeOnInternalSide() override;
  void finalize() override;
  void threadJoin(const UserObject & y) override;

protected:
  const VariableValue & _u;
  const VariableValue & _u_neighbor;
  const VariableGradient & _grad_u;
  const VariableGradient & _grad_u_neighbor;
  MooseVariable & _var;
  const VariableTestValue & _test;
  const VariableTestGradient & _grad_test;
  const VariableTestValue & _test_face;
  const VariableTestGradient & _grad_test_face;
  const VariableTestValue & _test_face_neighbor;
  const VariableTestGradient & _grad_test_face_neighbor;
  const Function & _func;
  const Real & _epsilon;
  const Real & _sigma;
  const MaterialProperty<Real> & _diff;
  const MaterialProperty<Real> & _diff_face;
  const ADMaterialProperty<Real> & _ad_diff_face;
  const MaterialProperty<Real> & _diff_face_by_name;
  const MaterialProperty<Real> & _diff_neighbor;
  const MaterialProperty<Real> & _diff_face_old;
  const MaterialProperty<Real> & _diff_face_older;
  std::vector<std::vector<Real>> _elem_integrals;
};
