//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef RENAMEBOUNDARYGENERATOR_H
#define RENAMEBOUNDARYGENERATOR_H

#include "MeshGenerator.h"

// Forward declarations
class RenameBoundaryGenerator;

template <>
InputParameters validParams<RenameBoundaryGenerator>();

/**
 * MeshGenerator for re-numbering or re-naming boundaries
 */
class RenameBoundaryGenerator : public MeshGenerator
{
public:
  RenameBoundaryGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate();

protected:
  std::unique_ptr<MeshBase> & _input;

  std::vector<boundary_id_type> _old_boundary_id;

  std::vector<BoundaryName> _old_boundary_name;

  std::vector<boundary_id_type> _new_boundary_id;

  std::vector<BoundaryName> _new_boundary_name;

  /**
   * Given a new_boundary_id, provide a boundary name, based
   * on the old boundary names provided (or deduced from
   * the old_boundary_ids provided).
   *
   * Eg, if
   * old_boundary_name = 'boundary1 boundary2'
   * new_boundary_id   = '4      5'
   * Then
   * newBoundaryName(4) = boundary1
   * newBoundaryName(5) = boundary2
   *
   * In the case of merging boundaries, the *first encountered*
   * boundary's name is used.
   * Eg, if
   * old_boundary_name = 'asdf boundary4 boundary1'
   * new_boundary_id   = '3    1      1'
   * then
   * newBoundaryName(1) = boundary4, because we look along
   * new_boundary_id until we first encounter 1, and then get the corresponding name
   * @param new_boundary_id the new boundary's ID number
   * @return the name that will be given to that boundary
   */
  BoundaryName newBoundaryName(const boundary_id_type & new_boundary_id);

  /**
   * Given a new_boundary_name, provide a boundary ID, based
   * on the old boundary IDs provided (or deduced from
   * the old_boundary_names provided).
   *
   * Eg, if
   * old_boundary_id =   '4      5'
   * new_boundary_name = 'boundary1 boundary2'
   * Then
   * newBoundaryID(boundary1) = 4
   * newBoundaryID(boundary2) = 5
   *
   * In the case of merging boundaries, the *first encountered*
   * boundary's ID is used.
   * Eg, if
   * old_boundary_id =   '3    1      1'
   * new_boundary_name = 'asdf boundary4 boundary4'
   * then
   * newBoundaryID(boundary4) = 1, because we look along
   * new_boundary_name until we first encounter boundary4, and then get the corresponding ID
   * @param new_boundary_name the new boundary's name
   * @return the ID number that will be given to that boundary
   */
  BoundaryID newBoundaryID(const BoundaryName & new_boundary_name);
};

#endif // RENAMEBOUNDARYGENERATOR_H
