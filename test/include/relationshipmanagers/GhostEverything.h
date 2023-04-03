//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RelationshipManager.h"

class GhostEverything : public RelationshipManager
{
public:
  GhostEverything(const InputParameters &);

  GhostEverything(const GhostEverything & others);

  static InputParameters validParams();

  void operator()(const MeshBase::const_element_iterator & range_begin,
                  const MeshBase::const_element_iterator & range_end,
                  processor_id_type p,
                  map_type & coupled_elements) override;

  std::unique_ptr<GhostingFunctor> clone() const override
  {
    return std::make_unique<GhostEverything>(*this);
  }

  std::string getInfo() const override;

  virtual bool operator>=(const RelationshipManager & other) const override;
};
