/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef XFEMMARKERUSEROBJECT_H
#define XFEMMARKERUSEROBJECT_H

#include "ElementUserObject.h"

class XFEM;

/**
 * Coupled auxiliary value
 */
class XFEMMarkerUserObject : public ElementUserObject
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  XFEMMarkerUserObject(const InputParameters & parameters);

  virtual ~XFEMMarkerUserObject() {}

  virtual void initialize();
  virtual void execute();
  virtual void threadJoin(const UserObject & y);
  virtual void finalize();

protected:
  virtual bool doesElementCrack(RealVectorValue & direction);

private:
  MooseMesh & _mesh;
  std::vector<BoundaryID> _initiation_boundary_ids;
  bool _secondary_cracks;
  MooseSharedPointer<XFEM> _xfem;
  std::map<unsigned int, RealVectorValue> _marked_elems;
  std::set<unsigned int> _marked_frags;
  std::map<unsigned int, unsigned int> _marked_elem_sides;
};

template <>
InputParameters validParams<XFEMMarkerUserObject>();

#endif // XFEMMARKERUSEROBJECT_H
