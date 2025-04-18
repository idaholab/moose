//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
}

/**
 * Static condensation preconditioner
 */
class MooseStaticCondensationPreconditioner : public SingleMatrixPreconditioner
{
public:
  static InputParameters validParams();

  MooseStaticCondensationPreconditioner(const InputParameters & params);

protected:
  libMesh::StaticCondensation & sc();
  const libMesh::StaticCondensation & sc() const;

  std::string prefix() const;

private:
  /// Pointer to the libMesh static condensation object
  libMesh::StaticCondensation * _sc;
};

inline libMesh::StaticCondensation &
MooseStaticCondensationPreconditioner::sc()
{
  libmesh_assert(_sc);
  return *_sc;
}

inline const libMesh::StaticCondensation &
MooseStaticCondensationPreconditioner::sc() const
{
  return const_cast<MooseStaticCondensationPreconditioner *>(this)->sc();
}
