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
#include "TaggingInterface.h"
#include "BlockRestrictable.h"
#include "ADReal.h"
#include "MooseTypes.h"
#include "ADFunctorInterface.h"
#include "INSFVPressureVariable.h"
#include "libmesh/vector_value.h"
#include "libmesh/id_types.h"
#include "libmesh/stored_range.h"

class MooseMesh;
class INSFVVelocityVariable;
class INSFVPressureVariable;
namespace libMesh
{
class Elem;
class MeshBase;
}

class RhieChowInterpolatorBase : public GeneralUserObject,
                                 public TaggingInterface,
                                 public BlockRestrictable,
                                 public ADFunctorInterface
{
public:
  static InputParameters validParams();
  RhieChowInterpolatorBase(const InputParameters & params);

  /**
   * API for momentum residual objects that have on-diagonals for velocity call.
   * This is only supposed to be called if we are using an implicit pressure-velocity
   * coupling in a monolithic solve.
   * @param The element we are adding 'a' coefficient data for
   * @param component The velocity component we are adding 'a' coefficient data for
   * @param value The value of 'a' that we are adding
   */
  virtual void addToA(const libMesh::Elem * elem, unsigned int component, const ADReal & value) = 0;

  /**
   * Retrieve a face velocity
   * @param m The velocity interpolation method. This is either Rhie-Chow or Average. Rhie-Chow is
   * recommended as it avoids checkerboards in the pressure field
   * @param fi The face that we wish to retrieve the velocity for
   * @param tid The thread ID
   * @return The face velocity
   */
  virtual VectorValue<ADReal> getVelocity(const FaceInfo & fi,
                                          const Moose::StateArg & time,
                                          THREAD_ID tid,
                                          Moose::FV::InterpMethod m) const = 0;

protected:
  /**
   * A virtual method that allows us to only implement getVelocity once for free and porous flows
   */
  virtual const Moose::FunctorBase<ADReal> & epsilon(THREAD_ID tid) const;

  /// The \p MooseMesh that this user object operates on
  MooseMesh & _moose_mesh;

  /// The \p libMesh mesh that this object acts on
  const libMesh::MeshBase & _mesh;

  /// The dimension of the mesh, e.g. 3 for hexes and tets, 2 for quads and tris
  const unsigned int _dim;

  /// The thread 0 copy of the pressure variable
  INSFVPressureVariable * const _p;

  /// The thread 0 copy of the x-velocity variable
  INSFVVelocityVariable * const _u;

  /// The thread 0 copy of the y-velocity variable (null if the problem is 1D)
  INSFVVelocityVariable * const _v;

  /// The thread 0 copy of the z-velocity variable (null if the problem is not 3D)
  INSFVVelocityVariable * const _w;

  /// The subdomain ids this object operates on
  const std::set<SubdomainID> _sub_ids;

  /// The velocity variable numbers
  std::vector<unsigned int> _var_numbers;

  /// The nonlinear system
  SystemBase & _sys;

private:
  /// A unity functor used in the epsilon virtual method
  const Moose::ConstantFunctor<ADReal> _unity_functor{1};
};

inline const Moose::FunctorBase<ADReal> & RhieChowInterpolatorBase::epsilon(THREAD_ID) const
{
  return _unity_functor;
}
