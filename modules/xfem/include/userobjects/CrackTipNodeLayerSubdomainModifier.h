//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CrackFrontDefinition.h"
#include "ElementSubdomainModifier.h"

#include <unordered_map>
#include <unordered_set>

/**
 * Assigns an enriched subdomain to elements within a specified number of node-connectivity
 * layers from crack-tip seed elements.
 */
class CrackTipNodeLayerSubdomainModifier : public ElementSubdomainModifier
{
public:
  static InputParameters validParams();

  CrackTipNodeLayerSubdomainModifier(const InputParameters & parameters);

  virtual void initialize() override;

protected:
  virtual SubdomainID computeSubdomainID() override;

private:
  void buildEnrichedElementSet();

  const SubdomainID _enriched_subdomain_id;
  const SubdomainID _base_subdomain_id;
  const unsigned int _num_node_layers;
  const CrackFrontDefinition & _crack_front_definition;

  std::unordered_set<dof_id_type> _enriched_elem_ids;
  std::unordered_map<dof_id_type, SubdomainID> _original_subdomain_ids;
};
