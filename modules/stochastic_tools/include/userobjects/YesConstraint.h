//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TopologicalConstraintBase.h"

/**
 *
 */
class YesConstraint : public TopologicalConstraintBase
{
public:
  static InputParameters validParams();

  YesConstraint(const InputParameters & params);
  virtual bool isConfigAllowed(const std::vector<dof_id_type> config,
                               const MooseMesh * _subapp_mesh) const override;
};
