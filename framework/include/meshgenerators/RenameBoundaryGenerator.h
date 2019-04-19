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

  std::unique_ptr<MeshBase> generate() override;

protected:
  std::unique_ptr<MeshBase> & _input;

  std::vector<boundary_id_type> _old_boundary_id;

  std::vector<BoundaryName> _old_boundary_name;

  std::vector<boundary_id_type> _new_boundary_id;

  std::vector<BoundaryName> _new_boundary_name;
};

#endif // RENAMEBOUNDARYGENERATOR_H
