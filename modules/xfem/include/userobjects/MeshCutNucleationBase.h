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

class MeshCutNucleationBase : public ElementUserObject
{
public:
  static InputParameters validParams();

  MeshCutNucleationBase(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & y) override;
  virtual void finalize() override;
  std::map<unsigned int, std::pair<RealVectorValue, RealVectorValue>> getNucleatedElemsMap() const
  {
    return _nucleated_elems;
  }
  Real getNucleationRadius() const { return _nucleation_radius; }

protected:
  /**
   * Determine whether the current element should be cut by a new crack.
   * @param cutterElemNodes nodes of line segment that will be used to create the cutter mesh
   * @return bool true if element cracks
   */
  virtual bool doesElementCrack(std::pair<RealVectorValue, RealVectorValue> & cutterElemNodes) = 0;

private:
  MooseMesh & _mesh;
  Real _nucleation_radius;
  std::shared_ptr<XFEM> _xfem;
  std::vector<BoundaryID> _initiation_boundary_ids;
  std::map<unsigned int, std::pair<RealVectorValue, RealVectorValue>> _nucleated_elems;
};
