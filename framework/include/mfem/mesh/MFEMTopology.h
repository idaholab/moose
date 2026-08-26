//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "DiscreteSymmetry.h"

/**
 Interface for MFEM mesh objects providing methods querying topological information about an
 mfem::ParMesh
 */
class MFEMTopology
{
public:
  static InputParameters validParams();

  MFEMTopology(const InputParameters & parameters);

  /**
   * Return the map between pairs of topologically equivalent vertices in the mesh,
   * each replicated vertex @a i is paired with a primary vertex @a vertex_map[i].
   * Largely copies mfem::Mesh::CreatePeriodicVertexMapping but supports a broader class of
   * symmetry maps between equivalent vertices.
   */
  std::vector<int> CreateTopologicallyEquivalentVertexMap(const mfem::Mesh & mfem_mesh) const;

  /**
   * Declare a translational symmetry via a translation vector between pairs of equivalent vertices
   */
  void DeclareTranslationalSymmetry(const mfem::Vector & translation);

  /**
   * Declare a rotational symmetry via the order of rotational symmetry about the z axis
   */
  void DeclareRotationalSymmetry(const unsigned int rotational_symmetry_order);

  /**
   * Return whether the mesh has periodicity applited
   */
  bool isPeriodic() const { return _periodic; }

private:
  // Stores whether any periodicity has been applied to the mesh
  bool _periodic{false};
  // Vector of translation vectors that pair periodically equivalent vertices
  std::vector<mfem::Vector> _lattice_vectors;
  // Order of rotational symmetry exhibited by the mesh about the z direction
  const unsigned int _rotational_symmetry_order;
  // Container to store set of discrete symmetries added to this object
  std::vector<std::shared_ptr<Moose::MFEM::DiscreteSymmetry>> _symmetry_transforms;
};

#endif
