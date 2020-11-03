//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#pragma once
#include "MultiAppTransfer.h"

/**
 * Copy vector(s) of a VectorPostprocessor to/from the main and sub-applications(s)
 */
class MultiAppDirectVectorPostprocessorTransfer : public MultiAppTransfer
{
public:
  static InputParameters validParams();
  MultiAppDirectVectorPostprocessorTransfer(const InputParameters & parameters);
  virtual void execute() override;

protected:
  virtual void executeToMultiapp();
  virtual void executeFromMultiapp();

  const VectorPostprocessorName & _to_vpp_name;
  const VectorPostprocessorName & _from_vpp_name;
  const std::vector<std::string> & _vector_names;
  const unsigned int & _subapp_index;
};
