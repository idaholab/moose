//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ScopedVectorTagAssociation.h"
#include "NonlinearSystemBase.h"

#include "libmesh/numeric_vector.h"

ScopedVectorTagAssociation::ScopedVectorTagAssociation(NonlinearSystemBase & nl, TagID tag)
  : _nl(nl),
    _tag(tag),
    _previous(nl.hasVector(tag) ? &nl.getVector(tag) : nullptr),
    _restored(false)
{
}

ScopedVectorTagAssociation::~ScopedVectorTagAssociation() { restore(); }

void
ScopedVectorTagAssociation::associate(libMesh::NumericVector<Number> & vector)
{
  _restored = false;
  _nl.associateVectorToTag(vector, _tag);
}

void
ScopedVectorTagAssociation::restore()
{
  if (_restored)
    return;

  if (_previous)
    _nl.associateVectorToTag(*_previous, _tag);
  else if (_nl.hasVector(_tag))
    _nl.disassociateVectorFromTag(_tag);

  _restored = true;
}
