//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RhieChowFaceFluxProvider.h"
#include "TaggingInterface.h"
#include "INSFVPressureVariable.h"
#include "ADReal.h"
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

class RhieChowInterpolatorBase : public RhieChowFaceFluxProvider,
                                 public TaggingInterface,
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
  virtual VectorValue<ADReal> getVelocity(const Moose::FV::InterpMethod m,
                                          const FaceInfo & fi,
                                          const Moose::StateArg & time,
                                          const THREAD_ID tid,
                                          bool subtract_mesh_velocity) const = 0;

  virtual Real getVolumetricFaceFlux(const Moose::FV::InterpMethod m,
                                     const FaceInfo & fi,
                                     const Moose::StateArg & time,
                                     const THREAD_ID tid,
                                     bool subtract_mesh_velocity) const override;

  /// Return the interpolation method used for velocity
  Moose::FV::InterpMethod velocityInterpolationMethod() const { return _velocity_interp_method; }

  /**
   * makes sure coefficient data gets communicated on both sides of a given boundary. This
   * is a virtual function, mostly used for monolithic approaches.
   */
  virtual void ghostADataOnBoundary(const BoundaryID /*boundary_id*/) {}

  /**
   * @return The pressure variable corresponding to the provided thread ID
   */
  const INSFVPressureVariable & pressure(THREAD_ID tid) const;

  const INSFVVelocityVariable * vel() const { return _u; }

  /// Bool of the Rhie Chow user object is used in monolithic/segregated approaches
  virtual bool segregated() const = 0;

protected:
  /**
   * A virtual method that allows us to only implement getVelocity once for free and porous flows
   */
  virtual const Moose::FunctorBase<ADReal> & epsilon(THREAD_ID tid) const;

  /**
   * Fill the passed-in variable container with the thread copies of \p var_name
   */
  template <typename Container>
  void fillContainer(const std::string & var_name, Container & container);

  /**
   * Check the block consistency between the passed in \p var and us
   */
  template <typename VarType>
  void checkBlocks(const VarType & var) const;

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

  /// All the thread copies of the pressure variable
  std::vector<MooseVariableFVReal *> _ps;

  /// All the thread copies of the x-velocity variable
  std::vector<MooseVariableFVReal *> _us;

  /// All the thread copies of the y-velocity variable
  std::vector<MooseVariableFVReal *> _vs;

  /// All the thread copies of the z-velocity variable
  std::vector<MooseVariableFVReal *> _ws;

  /// The velocity variable numbers
  std::vector<unsigned int> _var_numbers;

  /// The nonlinear system
  SystemBase & _sys;

  /// The interpolation method to use for the velocity
  Moose::FV::InterpMethod _velocity_interp_method;

  /// Whether this object is operating on the displaced mesh
  const bool _displaced;

private:
  /// A unity functor used in the epsilon virtual method
  const Moose::ConstantFunctor<ADReal> _unity_functor{1};
};

inline const Moose::FunctorBase<ADReal> &
RhieChowInterpolatorBase::epsilon(THREAD_ID) const
{
  return _unity_functor;
}

inline const INSFVPressureVariable &
RhieChowInterpolatorBase::pressure(const THREAD_ID tid) const
{
  mooseAssert(tid < _ps.size(), "Attempt to access out-of-bounds in pressure variable container");
  return *static_cast<INSFVPressureVariable *>(_ps[tid]);
}

template <typename Container>
void
RhieChowInterpolatorBase::fillContainer(const std::string & name, Container & container)
{
  typedef typename Container::value_type ContainedType;
  for (const auto tid : make_range(libMesh::n_threads()))
  {
    auto * const var = static_cast<ContainedType>(
        &UserObject::_subproblem.getVariable(tid, getParam<VariableName>(name)));
    container[tid] = var;
  }
}

template <typename VarType>
void
RhieChowInterpolatorBase::checkBlocks(const VarType & var) const
{
  const auto & var_blocks = var.blockIDs();
  const auto & uo_blocks = blockIDs();

  // Error if this UO has any blocks that the variable does not
  std::set<SubdomainID> uo_blocks_minus_var_blocks;
  std::set_difference(uo_blocks.begin(),
                      uo_blocks.end(),
                      var_blocks.begin(),
                      var_blocks.end(),
                      std::inserter(uo_blocks_minus_var_blocks, uo_blocks_minus_var_blocks.end()));
  if (uo_blocks_minus_var_blocks.size() > 0)
    mooseError("Block restriction of interpolator user object '",
               this->name(),
               "' (",
               Moose::stringify(blocks()),
               ") includes blocks not in the block restriction of variable '",
               var.name(),
               "' (",
               Moose::stringify(var.blocks()),
               ")");

  // Get the blocks in the variable but not this UO
  std::set<SubdomainID> var_blocks_minus_uo_blocks;
  std::set_difference(var_blocks.begin(),
                      var_blocks.end(),
                      uo_blocks.begin(),
                      uo_blocks.end(),
                      std::inserter(var_blocks_minus_uo_blocks, var_blocks_minus_uo_blocks.end()));

  // For each block in the variable but not this UO, error if there is connection
  // to any blocks on the UO.
  for (auto & block_id : var_blocks_minus_uo_blocks)
  {
    const auto connected_blocks = _moose_mesh.getBlockConnectedBlocks(block_id);
    std::set<SubdomainID> connected_blocks_on_uo;
    std::set_intersection(connected_blocks.begin(),
                          connected_blocks.end(),
                          uo_blocks.begin(),
                          uo_blocks.end(),
                          std::inserter(connected_blocks_on_uo, connected_blocks_on_uo.end()));
    if (connected_blocks_on_uo.size() > 0)
      mooseError("Block restriction of interpolator user object '",
                 this->name(),
                 "' (",
                 Moose::stringify(uo_blocks),
                 ") doesn't match the block restriction of variable '",
                 var.name(),
                 "' (",
                 Moose::stringify(var_blocks),
                 ")");
  }
}
