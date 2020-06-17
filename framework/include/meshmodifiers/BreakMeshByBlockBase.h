//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshModifier.h"

// forward declaration
class BreakMeshByBlockBase;

template <>
InputParameters validParams<BreakMeshByBlockBase>();

class BreakMeshByBlockBase : public MeshModifier
{
public:
  BreakMeshByBlockBase(const InputParameters & parameters);

  // method to override to implement other mesh splitting algorithms
  virtual void modify() override;

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

  /// given the master and secondary blocks this method return the appropriate
  /// boundary id and name
  void findBoundaryNameAndInd(const subdomain_id_type & /*masterBlockID*/,
                              const subdomain_id_type & /*secondaryBlockID*/,
                              std::string & /*boundaryName*/,
                              BoundaryID & /*boundaryID*/,
                              BoundaryInfo & /*boundary_info*/);

  std::set<std::pair<std::string, BoundaryID>> _bName_bID_set;

  /// this method finds the first free boundary id
  BoundaryID findFreeBoundaryId();

private:
  /// this method generate the boundary name by assembling subdomain names
  std::string generateBoundaryName(const subdomain_id_type & /*masterBlockID*/,
                                   const subdomain_id_type & /*secondaryBlockID*/);

  /// this method save the boundary name/id pair
  void mapBoundaryIdAndBoundaryName(BoundaryID & /*boundaryID*/,
                                    const std::string & /*boundaryName*/);
};

