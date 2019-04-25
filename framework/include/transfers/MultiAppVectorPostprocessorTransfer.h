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

// Forward declarations
class MultiAppVectorPostprocessorTransfer;

template <>
InputParameters validParams<MultiAppVectorPostprocessorTransfer>();

/**
 * Copies the values of a VectorPostprocessor from the Master to postprocessors
 * on each MultiApp
 * or collects the postprocessors on each MultiApp into a VectorPostprocessor
 */
class MultiAppVectorPostprocessorTransfer : public MultiAppTransfer
{
public:
  MultiAppVectorPostprocessorTransfer(const InputParameters & parameters);

  virtual void execute() override;

protected:
  virtual void executeToMultiapp();
  virtual void executeFromMultiapp();

  const PostprocessorName & _sub_pp_name;
  const VectorPostprocessorName & _master_vpp_name;
  const std::string & _vector_name;
};

