//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"

class NonlinearSystemBase;

namespace libMesh
{
template <typename T>
class NumericVector;
}

/**
 * Temporarily associate a nonlinear-system vector tag with a vector and restore the previous
 * association when the object goes out of scope.
 */
class ScopedVectorTagAssociation
{
public:
  ScopedVectorTagAssociation(NonlinearSystemBase & nl, TagID tag);
  ~ScopedVectorTagAssociation();

  /// Associate the scoped tag with \p vector until restore() or destruction.
  void associate(libMesh::NumericVector<Number> & vector);

  /// Restore the tag association that was active when this object was constructed.
  void restore();

private:
  NonlinearSystemBase & _nl;
  const TagID _tag;
  libMesh::NumericVector<Number> * const _previous;
  bool _restored;
};
