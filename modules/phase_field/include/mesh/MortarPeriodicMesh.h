//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MORTARPERIODICMESH_H
#define MORTARPERIODICMESH_H

#include "GeneratedMesh.h"
#include "MultiMooseEnum.h"

class MortarPeriodicMesh;

template <>
InputParameters validParams<MortarPeriodicMesh>();

/**
 * Mesh generated from parameters with additional subdomains for mortar interfaces
 * to enforce periodicity constraints
 */
class MortarPeriodicMesh : public GeneratedMesh
{
public:
  MortarPeriodicMesh(const InputParameters & parameters);
  MortarPeriodicMesh(const MortarPeriodicMesh & other_mesh);
  virtual ~MortarPeriodicMesh();

  virtual MooseMesh & clone() const;

  ///{@ public interfaces for the mortar periodicity action
  const std::vector<SubdomainID> & getMortarSubdomains() const { return _mortar_subdomains; }
  const MultiMooseEnum & getPeriodicDirections() const { return _periodic_dirs; }
  ///@}

protected:
  virtual void buildMesh();

  /// periodic directions
  MultiMooseEnum _periodic_dirs;

  // subdomain IDs for the mortar interfaces for each dimension
  std::vector<SubdomainID> _mortar_subdomains;
};

#endif // MORTARPERIODICMESH_H
