/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef MORTARPERIODICMESH_H
#define MORTARPERIODICMESH_H

#include "GeneratedMesh.h"

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
