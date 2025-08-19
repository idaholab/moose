//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SingleMatrixPreconditioner.h"

namespace libMesh
{
class StaticCondensation;
class StaticCondensationDofMap;
}

/**
 * Static condensation preconditioner
 */
class MooseStaticCondensationPreconditioner : public SingleMatrixPreconditioner
{
public:
  static InputParameters validParams();

  MooseStaticCondensationPreconditioner(const InputParameters & params);

  virtual void initialSetup() override;

protected:
  libMesh::StaticCondensationDofMap & scDofMap();
  const libMesh::StaticCondensationDofMap & scDofMap() const;

  libMesh::StaticCondensation & scSysMat();
  const libMesh::StaticCondensation & scSysMat() const;

  std::string prefix() const;

private:
  /// Pointer to the libMesh static condensation dof map object
  libMesh::StaticCondensationDofMap * _sc_dof_map;

  /// Pointer to the libMesh static condensation system matrix object
  libMesh::StaticCondensation * _sc_system_matrix;
};

inline libMesh::StaticCondensationDofMap &
MooseStaticCondensationPreconditioner::scDofMap()
{
  libmesh_assert(_sc_dof_map);
  return *_sc_dof_map;
}

inline const libMesh::StaticCondensationDofMap &
MooseStaticCondensationPreconditioner::scDofMap() const
{
  return const_cast<MooseStaticCondensationPreconditioner *>(this)->scDofMap();
}

inline libMesh::StaticCondensation &
MooseStaticCondensationPreconditioner::scSysMat()
{
  libmesh_assert(_sc_system_matrix);
  return *_sc_system_matrix;
}

inline const libMesh::StaticCondensation &
MooseStaticCondensationPreconditioner::scSysMat() const
{
  return const_cast<MooseStaticCondensationPreconditioner *>(this)->scSysMat();
}
