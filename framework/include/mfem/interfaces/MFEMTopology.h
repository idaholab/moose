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

#include "GeneralUserObject.h"
#include "libmesh/ignore_warnings.h"
#include "mfem/miniapps/common/mfem-common.hpp"
#include "libmesh/restore_warnings.h"

/**
 Virtual base class for representing discrete symmetry transforms between equivalent vertices in a
 mesh
 */
class DiscreteSymmetry
{
public:
  DiscreteSymmetry() = default;
  virtual void ApplyTransform(const mfem::Vector & coord_in, mfem::Vector & coord_out) = 0;
};

class TranslationalSymmetry : public DiscreteSymmetry
{
public:
  TranslationalSymmetry(const mfem::Vector & lattice_vector)
    : DiscreteSymmetry(), _lattice_vector(lattice_vector){};

  virtual void ApplyTransform(const mfem::Vector & coord_in, mfem::Vector & coord_out) override
  {
    mooseAssert((coord_in.Size() == _lattice_vector.Size()),
                "Size of lattice vector doesn't match the space dimension");
    add(coord_in, _lattice_vector, coord_out);
  };

private:
  const mfem::Vector & _lattice_vector;
};

class RotationalSymmetry : public DiscreteSymmetry
{
public:
  RotationalSymmetry(const unsigned int rotational_symmetry_order)
    : DiscreteSymmetry(),
      _rotational_symmetry_order(rotational_symmetry_order),
      _rotation_angle(2 * pi / rotational_symmetry_order){};

  virtual void ApplyTransform(const mfem::Vector & coord_in, mfem::Vector & coord_out) override
  {
    // x' =  x cos phi + y sin phi
    // y' = -x sin phi + y cos phi
    // z' = z
    coord_out[0] = coord_in[0] * cos(_rotation_angle) + coord_in[1] * sin(_rotation_angle);
    coord_out[1] = -coord_in[0] * sin(_rotation_angle) + coord_in[1] * cos(_rotation_angle);
    coord_out[2] = coord_in[2];
  };

private:
  const unsigned int _rotational_symmetry_order;
  const mfem::real_t _rotation_angle; // radians
};

/**
 Interface for MFEM mesh objects providing methods querying topological information about an
 mfem::ParMesh
 */
class MFEMTopology
{
public:
  static InputParameters validParams();

  MFEMTopology(const InputParameters & parameters);

  /// Return the map between pairs of topologically equivalent vertices in the mesh,
  /// each replicated vertex @a i is paired with a primary vertex @a vertex_map[i].
  /// Largely replicates mfem::Mesh::CreatePeriodicVertexMapping but supports a broader class of
  /// symmetry maps between equivalent vertices.
  std::vector<int> CreateTopologicallyEquivalentVertexMap(const mfem::Mesh & mfem_mesh) const;

  // Make `r` and all of `r`'s replicas be replicas of `p`. Delete `r` from the
  // list of primary vertices.
  void DeclareTranslationalSymmetry(const mfem::Vector & translation);

  void DeclareRotationalSymmetry(const unsigned int rotational_symmetry_order);

protected:
  bool _periodic{false};

private:
  std::vector<std::vector<mfem::real_t>> _input_lattice_vectors;
  std::vector<mfem::Vector> _lattice_vectors;
  const unsigned int _rotational_symmetry_order;
  std::vector<std::shared_ptr<DiscreteSymmetry>> _symmetry_transforms;
};

#endif
