/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef ADDSIDESETSBASE_H
#define ADDSIDESETSBASE_H

#include "MeshModifier.h"

// libMesh includes
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
