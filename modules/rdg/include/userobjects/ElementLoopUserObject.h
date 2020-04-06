//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"
#include "Coupleable.h"
#include "ScalarCoupleable.h"
#include "MooseVariableDependencyInterface.h"
#include "MooseVariable.h"
#include "SubProblem.h"
#include "NonlinearSystem.h"
#include "Assembly.h"
#include "FEProblem.h"
#include "MooseMesh.h"

#include "libmesh/elem.h"
#include "libmesh/parallel_algebra.h"

// Forward Declarations

namespace libMesh
{
class Elem;
class QBase;
}

/**
 * A base class that loops over elements and do things
 *
 * Notes:
 *
 *   1. this class is designed to enable more than one element-loop in the execution of user
 * objects.
 *      It is necessary because in many numerical schemes, the data required in one element-loop
 *      should be pre-computed from another element-loop.
 *
 *      For example, in the workflow of a cell-centered finite volume method,
 *      two element-loops are required in a specific sequence in user objects:
 *
 *      First, an element-loop is requried to calculate the in-cell gradients of variables
 *      using a piecewise linear reconstruction scheme.
 *
 *      Second, another element-loop is required to calculate the limited in-cell gradients of
 * variables
 *      based on the reconstructed gradients from the element and its face-neighboring elements.
 */
class ElementLoopUserObject : public GeneralUserObject,
                              public BlockRestrictable,
                              public Coupleable,
                              public MooseVariableDependencyInterface
{
public:
  static InputParameters validParams();

  ElementLoopUserObject(const InputParameters & parameters);
  ElementLoopUserObject(ElementLoopUserObject & x, Threads::split split);
  virtual ~ElementLoopUserObject();

  virtual void initialize();
  virtual void execute();
  virtual void finalize();

  virtual void pre();
  virtual void preElement(const Elem * elem);
  virtual void onElement(const Elem * elem);
  virtual void onBoundary(const Elem * elem, unsigned int side, BoundaryID bnd_id);
  virtual void onInternalSide(const Elem * elem, unsigned int side);
  virtual void onInterface(const Elem * elem, unsigned int side, BoundaryID bnd_id);
  virtual void post();
  virtual void subdomainChanged();
  virtual bool keepGoing() { return true; }

  virtual void meshChanged();

  void join(const ElementLoopUserObject & /*y*/);

protected:
  virtual void caughtMooseException(MooseException & e);

  MooseMesh & _mesh;

  const Elem * _current_elem;
  const Real & _current_elem_volume;
  unsigned int _current_side;
  const Elem * _current_neighbor;

  const MooseArray<Point> & _q_point;
  const QBase * const & _qrule;
  const MooseArray<Real> & _JxW;
  const MooseArray<Real> & _coord;

  /// true if we have cached interface elements, false if they need to be cached. We want to (re)cache only when mesh changed
  bool _have_interface_elems;
  /// List of element IDs that are on the processor boundary and need to be send to other processors
  std::set<dof_id_type> _interface_elem_ids;

  /// The subdomain for the current element
  SubdomainID _subdomain;

  /// The subdomain for the last element
  SubdomainID _old_subdomain;

  virtual void computeElement();
  virtual void computeBoundary();
  virtual void computeInternalSide();
  virtual void computeInterface();
};
