//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVAttributes.h"
#include "INSFVFlowBC.h"
#include "INSFVFullyDevelopedFlowBC.h"
#include "INSFVNoSlipWallBC.h"
#include "INSFVSlipWallBC.h"
#include "INSFVSymmetryBC.h"
#include "INSFVMomentumResidualObject.h"

void
AttribINSFVBCs::initFrom(const MooseObject * obj)
{
  _val = 0;
  // clang-format off
  _val |= (unsigned int)INSFVBCs::INSFVFlowBC                * (dynamic_cast<const INSFVFlowBC *>(obj) != nullptr);
  _val |= (unsigned int)INSFVBCs::INSFVFullyDevelopedFlowBC  * (dynamic_cast<const INSFVFullyDevelopedFlowBC *>(obj) != nullptr);
  _val |= (unsigned int)INSFVBCs::INSFVNoSlipWallBC          * (dynamic_cast<const INSFVNoSlipWallBC *>(obj) != nullptr);
  _val |= (unsigned int)INSFVBCs::INSFVSlipWallBC            * (dynamic_cast<const INSFVSlipWallBC *>(obj) != nullptr);
  _val |= (unsigned int)INSFVBCs::INSFVSymmetryBC            * (dynamic_cast<const INSFVSymmetryBC *>(obj) != nullptr);
  // clang-format on
}

bool
AttribINSFVBCs::isMatch(const Attribute & other) const
{
  auto a = dynamic_cast<const AttribINSFVBCs *>(&other);
  return a && (a->_val & _val);
}

bool
AttribINSFVBCs::isEqual(const Attribute & other) const
{
  auto a = dynamic_cast<const AttribINSFVBCs *>(&other);
  return a && (a->_val == _val);
}

void
AttribINSFVMomentumResidualObject::initFrom(const MooseObject * obj)
{
  _val = dynamic_cast<const INSFVMomentumResidualObject *>(obj);
}

bool
AttribINSFVMomentumResidualObject::isMatch(const Attribute & other) const
{
  auto a = dynamic_cast<const AttribINSFVMomentumResidualObject *>(&other);
  return a && (a->_val == _val);
}

bool
AttribINSFVMomentumResidualObject::isEqual(const Attribute & other) const
{
  auto a = dynamic_cast<const AttribINSFVMomentumResidualObject *>(&other);
  return a && (a->_val == _val);
}
