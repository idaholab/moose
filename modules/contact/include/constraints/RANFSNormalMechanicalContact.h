//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "NodeFaceConstraint.h"
#include "MooseEnum.h"

#include <vector>

class RANFSNormalMechanicalContact;
class PenetrationInfo;

namespace libMesh
{
template <typename>
class NumericVector;
}

template <>
InputParameters validParams<RANFSNormalMechanicalContact>();

class RANFSNormalMechanicalContact : public NodeFaceConstraint
{
public:
  RANFSNormalMechanicalContact(const InputParameters & parameters);

  void computeJacobian() override;
  void computeOffDiagJacobian(unsigned int jvar) override;
  bool shouldApply() override;

protected:
  virtual Real computeQpSlaveValue() override;

  virtual Real computeQpResidual(Moose::ConstraintType type) override;
  virtual Real computeQpJacobian(Moose::ConstraintJacobianType type) override;
  virtual Real computeQpOffDiagJacobian(Moose::ConstraintJacobianType type, unsigned jvar) override;

  const MooseEnum _component;
  const unsigned int _mesh_dimension;
  NumericVector<Number> & _residual_copy;

  unsigned int _largest_component;
  std::vector<unsigned int> _vars;
  std::vector<MooseVariable *> _var_objects;
  Real _lagrange_multiplier;
  PenetrationInfo * _pinfo;
};
