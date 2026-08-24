//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/partitioner.h"

class AbaqusUELMesh;

/**
 * Partitioner for the AbaqusUELMesh.
 *
 * The libMesh mesh backing an AbaqusUELMesh consists of disconnected NodeElems (one per UEL
 * node), so the standard face-neighbor dual graph is empty and Metis/ParMETIS have nothing to
 * cut. This partitioner instead builds the dual graph from the UEL connectivity: two
 * node-elements are adjacent if they are nodes of a common UEL element. The resulting graph is
 * handed to ParMETIS through PETSc's MatPartitioning interface
 * (PetscExternalPartitioner::partitionGraph), which is the same machinery the
 * PetscExternalPartitioner uses.
 */
class AbaqusUELPartitioner : public libMesh::Partitioner
{
public:
  AbaqusUELPartitioner(AbaqusUELMesh & uel_mesh);

  virtual std::unique_ptr<libMesh::Partitioner> clone() const override;

  using libMesh::Partitioner::partition;
  virtual void partition(libMesh::MeshBase & mesh, const unsigned int n) override;

protected:
  virtual void _do_partition(libMesh::MeshBase & mesh, const unsigned int n) override;

private:
  AbaqusUELMesh & _uel_mesh;
};
