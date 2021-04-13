//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RayTracingStudy.h"

// MOOSE includes
#include "MooseObject.h"
#include "SetupInterface.h"
#include "FunctionInterface.h"
#include "PostprocessorInterface.h"
#include "VectorPostprocessorInterface.h"
#include "MeshChangedInterface.h"
#include "TransientInterface.h"
#include "UserObjectInterface.h"
#include "DependencyResolverInterface.h"

// Local includes
#include "ElemExtrema.h"

// Forward declarations
class AuxiliarySystem;
class NonlinearSystemBase;

/**
 * Base class for a MooseObject used in ray tracing.
 */
class RayTracingObject : public MooseObject,
                         public SetupInterface,
                         public FunctionInterface,
                         public PostprocessorInterface,
                         public VectorPostprocessorInterface,
                         public MeshChangedInterface,
                         public TransientInterface,
                         public UserObjectInterface,
                         public DependencyResolverInterface
{
public:
  RayTracingObject(const InputParameters & params);

  static InputParameters validParams();

  const std::set<std::string> & getRequestedItems() override { return _depend_names; }
  const std::set<std::string> & getSuppliedItems() override { return _supplied_names; }

  /**
   * The RayTracingStudy associated with this object
   */
  RayTracingStudy & study() { return _study; }
  /**
   * The RayTracingStudy associated with this object
   */
  const RayTracingStudy & study() const { return _study; }

  /**
   * Insertion point called immediately before executing the RayTracingStudy.
   */
  virtual void preExecuteStudy(){};
  /**
   * Insertion point called immediately after executing the RayTracingStudy.
   */
  virtual void postExecuteStudy(){};

protected:
  /**
   * Gets the current Ray that this is working on
   */
  const std::shared_ptr<Ray> & currentRay() const { return *_current_ray; }

  /**
   * Add an object of this classes base type that this class depends on.
   */
  void dependsOn(const std::string name) { _depend_names.insert(name); }

  /// The FEProblemBase
  FEProblemBase & _fe_problem;

  /// The RayTracingStudy associated with this object
  RayTracingStudy & _study;

  /// The nonlinear system
  NonlinearSystemBase & _nl;
  /// The aux system
  AuxiliarySystem & _aux;

  /// The MooseMesh
  MooseMesh & _mesh;

  /// The thread id
  const THREAD_ID _tid;

  /// The current Elem that _current_ray is tracing through
  const Elem * const & _current_elem;
  /// The subdomain ID of the _current_elem that the ray is tracing through
  const SubdomainID & _current_subdomain_id;
  /// The side that _current_ray intersects (if any)
  const unsigned short & _current_intersected_side;
  /// The elem extrema (vertex/edge) that _current_ray intersects (if any)
  const ElemExtrema & _current_intersected_extrema;

private:
  /// The names of the objects this depends on
  std::set<std::string> _depend_names;
  /// The names of the supplied objects: just this one
  std::set<std::string> _supplied_names;

  /// The current Ray that the action is being applied to
  const std::shared_ptr<Ray> * const & _current_ray;
};

#define usingRayTracingObjectMembers                                                               \
  usingMooseObjectMembers;                                                                         \
  usingFunctionInterfaceMembers;                                                                   \
  usingPostprocessorInterfaceMembers;                                                              \
  usingTransientInterfaceMembers;                                                                  \
  using RayTracingObject::currentRay;                                                              \
  using RayTracingObject::errorPrefix
