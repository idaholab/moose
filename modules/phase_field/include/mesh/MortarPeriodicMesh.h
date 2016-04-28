/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef MORTARPERIODICMESH_H
#define MORTARPERIODICMESH_H

#include "MooseMesh.h"

class MortarPeriodicMesh;

template<>
InputParameters validParams<MortarPeriodicMesh>();

/**
 * Mesh generated from parameters with additional subdomains for mortar interfaces
 * to enforce periodicity constraints
 */
class MortarPeriodicMesh : public MooseMesh
{
public:
  MortarPeriodicMesh(const InputParameters & parameters);
  MortarPeriodicMesh(const MortarPeriodicMesh & other_mesh);
  virtual ~MortarPeriodicMesh();

  virtual MooseMesh & clone() const;

protected:
  virtual void buildMesh();

  void addLineMesh(unsigned int nelem, Real x0, Real y0, Real x1, Real y1, subdomain_id_type id);

  /// The dimension of the mesh
  MooseEnum _dim;

  /// Number of elements in x, y, z direction
  int _nx, _ny, _nz;

  /// The min/max values for x,y,z component
  Real _xmin, _xmax, _ymin, _ymax, _zmin, _zmax;

  /// block IDs for the mortar interface blocks
  subdomain_id_type _block_x, _block_y, _block_z;

  /// reference to the underlying unstructured mesh object
  UnstructuredMesh & _us_mesh;
};

#endif //MORTARPERIODICMESH_H
