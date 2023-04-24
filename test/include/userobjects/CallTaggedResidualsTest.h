//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"

class NonlinearSystemBase;

class CallTaggedResidualsTest : public GeneralUserObject
{
public:
  static InputParameters validParams();
  CallTaggedResidualsTest(const InputParameters & parameters);
  virtual void initialSetup() override;
  virtual void execute() override;
  virtual void initialize() override {}
  virtual void finalize() override {}

private:
  NonlinearSystemBase & _nl;
  std::vector<TagID> _tags;
  std::vector<const NumericVector<Number> *> _tag_vectors;
  std::vector<std::unique_ptr<NumericVector<Number>>> _saved_vectors;
};
