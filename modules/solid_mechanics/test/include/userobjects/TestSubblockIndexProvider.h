//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SubblockIndexProvider.h"
#include "GeneralUserObject.h"

/**
 * A class used to set the subblock index for testing generalized plane strain
 * calculations when more than one out-of-plane strain is provided on different
 * subsets of elements.
 */
class TestSubblockIndexProvider : public SubblockIndexProvider, public GeneralUserObject
{
public:
  static InputParameters validParams();

  TestSubblockIndexProvider(const InputParameters & params);

  virtual void initialize() override{};
  virtual void execute() override{};
  virtual void finalize() override{};
  /**
   * The index of subblock this element is on.
   */
  virtual unsigned int getSubblockIndex(const Elem & /* elem */) const override;

  /**
   * The max index of subblock.
   */
  virtual unsigned int getMaxSubblockIndex() const override;
};
