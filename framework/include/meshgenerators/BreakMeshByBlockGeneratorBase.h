//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshGenerator.h"

/**
 *
 */
class BreakMeshByBlockGeneratorBase : public MeshGenerator
{
public:
  static InputParameters validParams();

  BreakMeshByBlockGeneratorBase(const InputParameters & parameters);

protected:
  /// the file_name from whence this mesh came
  std::string _file_name;
  /// the name of the new interface
  std::string _interface_name;

  /// the flag to split the interface by block
  bool _split_interface;

  /// check that if split_interface==true interface_id and interface_name are
  /// not set by the user. It also check that the provided interface_id is not
  /// already used
  void checkInputParameter();

  /// given the primary and secondary blocks this method return the appropriate
  /// boundary id and name
  void findBoundaryNameAndInd(MeshBase & mesh,
                              const subdomain_id_type & /*primaryBlockID*/,
                              const subdomain_id_type & /*secondaryBlockID*/,
                              std::string & /*boundaryName*/,
                              boundary_id_type & /*boundaryID*/,
                              BoundaryInfo & /*boundary_info*/);

  std::set<std::pair<std::string, BoundaryID>> _bName_bID_set;

  /// this method finds the first free boundary id
  BoundaryID findFreeBoundaryId(MeshBase & mesh);

private:
  /// this method generate the boundary name by assembling subdomain names
  std::string generateBoundaryName(MeshBase & mesh,
                                   const subdomain_id_type & /*primaryBlockID*/,
                                   const subdomain_id_type & /*secondaryBlockID*/);

  /// this method save the boundary name/id pair
  void mapBoundaryIdAndBoundaryName(boundary_id_type & /*boundaryID*/,
                                    const std::string & /*boundaryName*/);
};
