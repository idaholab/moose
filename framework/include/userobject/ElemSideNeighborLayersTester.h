//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementUOProvider.h"

/**
 * User object to show information about the ElemSideNeighborLayer object's "ghosting" behaviors
 */
class ElemSideNeighborLayersTester : public ElementUOProvider
{
public:
  static InputParameters validParams();

  ElemSideNeighborLayersTester(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual unsigned long getElementalValueLong(dof_id_type element_id,
                                              const std::string & field_name) const override;

protected:
  std::set<dof_id_type> _ghost_data;
  std::set<dof_id_type> _evaluable_data;

  const dof_id_type _rank;
};
