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
using libMesh::NumericVector;
using libMesh::PetscVector;

/**
 * A class which helps with repeated reading from a petsc vector.
 * Its main purpose is to avoid unnecessary calls to the get_array() function
 * in the wrapper.
 */
class PetscVectorReader
{
public:
  /// Construct using a pets vector
  PetscVectorReader(PetscVector<Number> & vec);

  /// Construct using a numeric vector
  PetscVectorReader(NumericVector<Number> & vec);

  /// Destructor to make sure the vector is restored every time this
  /// goes out of scope
  ~PetscVectorReader();

  /// Restore the array, usually upon going out of scope
  void restore();

  /// Access a value in the petsc vector
  PetscScalar operator()(const numeric_index_type i) const;

private:
  /// Check if this vector is readable
  bool readable() const { return _raw_value != nullptr; }

  /// Reference to the petsc vector whose values shall be read
  PetscVector<Number> & _vec;

  /// The raw values in the vector
  const PetscScalar * _raw_value;
};

inline PetscScalar
PetscVectorReader::operator()(const numeric_index_type i) const
{
  mooseAssert(readable(), "Not readable");
  const numeric_index_type local_index = _vec.map_global_to_local_index(i);
  return _raw_value[local_index];
}
