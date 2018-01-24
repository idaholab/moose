//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADDSIDESETSBASE_H
#define ADDSIDESETSBASE_H

#include "MeshModifier.h"

#include "libmesh/fe_base.h"

// Forward declarations
class AddSideSetsBase;

// libMesh forward declarations
namespace libMesh
{
class QGauss;
}

template <>
InputParameters validParams<AddSideSetsBase>();

class AddSideSetsBase : public MeshModifier
{
public:
  AddSideSetsBase(const InputParameters & parameters);
  virtual ~AddSideSetsBase(); // dtor required for unique_ptr with forward declarations

protected:
  /**
   * This method is used to construct the FE object so we can compute
   * normals of faces.  Additionaly this method also grabs the sideset
   * ids from the mesh given a list of sideset names.
   */
  void setup();

  /**
   * This method finalizes the object, setting names back in the
   * boundary_info object and releasing memory.
   */
  void finalize();

  /**
   * This method implements a recursive flood routine to paint a sideset of
   * mesh to neighboring faces given a starting element and normal.
   */
  void flood(const Elem * elem, Point normal, BoundaryID side_id);

  BoundaryID getNextBoundaryID() const;

  Real _variance;
  bool _fixed_normal;

  std::unique_ptr<FEBase> _fe_face;
  std::unique_ptr<QGauss> _qface;
  std::map<BoundaryID, std::set<const Elem *>> _visited;
};

#endif /* ADDSIDESETSBASE_H */
