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
 * Copies the values of a VectorPostprocessor from the parent application to postprocessors
 * on each subapp of a MultiApp
 * or collects the postprocessors on each subapp of a MultiApp into a VectorPostprocessor
 * on the parent app
 */
class MultiAppVectorPostprocessorTransfer : public MultiAppTransfer
{
public:
  static InputParameters validParams();

  MultiAppVectorPostprocessorTransfer(const InputParameters & parameters);

  virtual void execute() override;

protected:
  virtual void executeToMultiapp();
  virtual void executeFromMultiapp();

  /// Name of the postprocessor on the MultiApp
  const PostprocessorName & _sub_pp_name;

  /// Name of the VectorPostprocessor on the parent app
  const VectorPostprocessorName & _master_vpp_name;

  /// Specific vector to transfer among the vectors in the parent VPP
  const std::string & _vector_name;
};
