//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/petsc_vector.h"
#include "libmesh/id_types.h"
#include "libmesh/libmesh_common.h"
#include "MooseError.h"

using libMesh::Number;
using libMesh::numeric_index_type;
using libMesh::PetscVector;

class PetscVectorReader
{
public:
  PetscVectorReader(PetscVector<Number> & vec);
  PetscVectorReader(NumericVector<Number> & vec);
  ~PetscVectorReader();

  void restore();
  bool readable() const { return _raw_value != nullptr; }

  PetscScalar operator()(const numeric_index_type i) const;

private:
  PetscVector<Number> & _vec;
  const PetscScalar * _raw_value;
};

inline PetscScalar
PetscVectorReader::operator()(const numeric_index_type i) const
{
  mooseAssert(readable(), "Not readable");
  const numeric_index_type local_index = _vec.map_global_to_local_index(i);
  return _raw_value[local_index];
}
