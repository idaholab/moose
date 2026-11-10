//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementUserObject.h"
#include "DenseMatrix.h"

/**
 * DSGShellNodalResultants collects the stress resultants of the DSG shell elements at
 * the nodes. Every element evaluates its resultants at its own nodes, from its own
 * interpolation; this object averages, node by node, the values of the elements that
 * share the node. The result is read by DSGShellNodalResultantsAux.
 *
 * The strain material must be asked for the nodal strains with compute_nodal_strains.
 */
class DSGShellNodalResultants : public ElementUserObject
{
public:
  static InputParameters validParams();

  DSGShellNodalResultants(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & other) override;
  virtual void finalize() override;

  /// Returns the averaged resultant with the given index at a node
  Real nodalValue(const dof_id_type node_id, const unsigned int resultant_index) const;

protected:
  /// Resultants at the nodes of the current element, one row per node, from the material
  const MaterialProperty<DenseMatrix<Real>> & _nodal_resultants;

  /// Sum of the resultants of the elements sharing each node, nine values per node
  std::map<dof_id_type, std::vector<Real>> _sums;

  /// Number of elements sharing each node
  std::map<dof_id_type, unsigned int> _counts;
};
