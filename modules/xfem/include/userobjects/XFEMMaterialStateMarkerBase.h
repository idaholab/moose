//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementUserObject.h"

class XFEM;

/**
 * Coupled auxiliary value
 */
class XFEMMaterialStateMarkerBase : public ElementUserObject
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  static InputParameters validParams();

  XFEMMaterialStateMarkerBase(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & y) override;
  virtual void finalize() override;

protected:
  /**
   * Determine whether the current element should be cut by a new crack.
   * @param direction Normal direction of crack if it is cracked
   * @return bool true if element cracks
   */
  virtual bool doesElementCrack(RealVectorValue & direction);

private:
  MooseMesh & _mesh;
  std::vector<BoundaryID> _initiation_boundary_ids;
  bool _secondary_cracks;
  std::shared_ptr<XFEM> _xfem;
  std::map<unsigned int, RealVectorValue> _marked_elems;
  std::set<unsigned int> _marked_frags;
  std::map<unsigned int, unsigned int> _marked_elem_sides;
};
