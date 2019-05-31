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
#include "MultiAppFieldTransfer.h"

// Forward declarations
class MultiAppUserObjectTransfer;

template <>
InputParameters validParams<MultiAppUserObjectTransfer>();

/**
 * Samples a variable's value in the Master domain at the point where
 * the MultiApp is.  Copies that value into a postprocessor in the
 * MultiApp.
 */
class MultiAppUserObjectTransfer : public MultiAppFieldTransfer
{
public:
  MultiAppUserObjectTransfer(const InputParameters & parameters);

  virtual void execute() override;

protected:
  std::string _user_object_name;

  /**
   * Boolean variable to generate error if every master node
   * cannot be mapped to a subApp during from_multiapp transfer
   **/
  const bool _all_master_nodes_contained_in_sub_app;
};
