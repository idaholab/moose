//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

namespace // Anonymous namespace for helpers
{
/**
 * Specific weak ordering for Elem *'s to be used in a set.
 * We use the id, but first sort by level.  This guarantees
 * when traversing the set from beginning to end the lower
 * level (parent) elements are encountered first.
 *
 * This was swiped from libMesh mesh_communication.C, and ought to be
 * replaced with libMesh::CompareElemIdsByLevel just as soon as I refactor to
 * create that - @roystgnr
 */
struct CompareElemsByLevel
{
  bool operator()(const Elem * a, const Elem * b) const
  {
    libmesh_assert(a);
    libmesh_assert(b);
    const unsigned int al = a->level(), bl = b->level();
    const dof_id_type aid = a->id(), bid = b->id();

    return (al == bl) ? aid < bid : al < bl;
  }
};

} // anonymous namespace
