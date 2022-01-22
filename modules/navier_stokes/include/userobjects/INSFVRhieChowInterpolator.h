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
#include "CellCenteredMapFunctor.h"
#include "VectorComponentFunctor.h"
#include "libmesh/vector_value.h"
#include "libmesh/id_types.h"
#include "libmesh/stored_range.h"
#include <unordered_map>
#include <set>
#include <unordered_set>

class MooseMesh;
class INSFVVelocityVariable;
class INSFVPressureVariable;
namespace libMesh
{
class Elem;
class MeshBase;
}

/**
 * This user-object gathers 'a' (on-diagonal velocity coefficients) and 'B' (body-force) data. It
 * performs an interpolation (first overbar operation) and reconstruction (second overbar operation)
 * of the 'B' data before applying it to the momentum residuals as suggested by Moukalled.
 * Additionally it performs one more interpolation on 'B' to the faces and includes the difference
 * of this third overbar operation and the first overbar operation in the computation of the
 * Rhie-Chow velocity, which this object computes and provides to momentum advection kernels. This
 * class also supports computation of an average face velocity although this is generally not
 * encouraged as it will lead to a checkerboard in the pressure field
 */
class INSFVRhieChowInterpolator : public GeneralUserObject,
                                  public TaggingInterface,
                                  public BlockRestrictable
{
public:
  static InputParameters validParams();
  INSFVRhieChowInterpolator(const InputParameters & params);

  /**
   * API that momentum residual objects that have on-diagonals for velocity call
   * @param The element we are adding 'a' coefficient data for
   * @param component The velocity component we are adding 'a' coefficient data for
   * @param value The value of 'a' that we are adding
   */
  void addToA(const libMesh::Elem * elem, unsigned int component, const ADReal & value);

  /**
   * API that momentum residual objects that have body forces call
   * @param The element we are adding 'B' data for
   * @param component The velocity component we are adding 'B' data for
   * @param value The value of 'B' that we are adding
   */
  void addToB(const libMesh::Elem * elem, unsigned int component, const ADReal & value);

  /**
   * Retrieve a face velocity
   * @param m The velocity interpolation method. This is either RhieChow or Average. RhieChow is
   * recommended as it avoids checkboards in the pressure field
   * @param fi The face that we wish to retrieve the velocity for
   * @param tid The thread ID
   * @return The face velocity
   */
  VectorValue<ADReal>
  getVelocity(Moose::FV::InterpMethod m, const FaceInfo & fi, THREAD_ID tid) const;

  /**
   * An API for communicating to the user object that body forces exist on the provided \p sub_ids
   */
  void hasBodyForces(const std::set<SubdomainID> & sub_ids);

  void initialSetup() override;
  void residualSetup() override;
  void meshChanged() override;

  void initialize() override final;
  void execute() override final;
  void finalize() override final;

protected:
  /**
   * @return whether this face is geometrically relevant to us
   */
  bool isFaceGeometricallyRelevant(const FaceInfo & fi) const;

  /**
   * A virtual method that allows us to only implement getVelocity once for free and porous flows
   */
  virtual const Moose::FunctorBase<ADReal> & epsilon(THREAD_ID tid) const;

  /**
   * Whether body forces are present on any portion of this user-object's domain
   */
  bool hasBodyForces() const { return !_sub_ids_with_body_forces.empty(); }

  /**
   * perform the setup of this object
   */
  void insfvSetup();

  /// The \p MooseMesh that this user object operates on
  MooseMesh & _moose_mesh;

  /// The \p libMesh mesh that this object acts on
  const libMesh::MeshBase & _mesh;

  /// The dimension of the mesh, e.g. 3 for hexes and tets, 2 for quads and tris
  const unsigned int _dim;

  /// A functor for computing the (non-RC corrected) velocity
  std::vector<std::unique_ptr<PiecewiseByBlockLambdaFunctor<ADRealVectorValue>>> _vel;

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

  /// All the active and elements local to this process that exist on this object's subdomains
  std::unique_ptr<ConstElemRange> _elem_range;

  /// The faces whose neighboring elements can be evaluated on this process
  std::vector<const FaceInfo *> _evaluable_fi;

  /// The subdomain ids this object operates on
  const std::set<SubdomainID> _sub_ids;

  /// A map from element IDs to 'a' coefficient data
  std::unordered_map<dof_id_type, libMesh::VectorValue<ADReal>> _a;

  /// A map from element IDs to 'B' data. After populating the map, this object can be evaluated at
  /// both element and face centers
  CellCenteredMapFunctor<libMesh::VectorValue<ADReal>,
                         std::unordered_map<dof_id_type, libMesh::VectorValue<ADReal>>>
      _b;

  /// The result of one interpolation and reconstruction of the 'B' data (built using call to
  /// Moose::FV::interpolateReconstruct). After construction, this object can be evaluated at both
  /// element and face centers
  CellCenteredMapFunctor<libMesh::VectorValue<ADReal>,
                         std::unordered_map<dof_id_type, libMesh::VectorValue<ADReal>>>
      _b2;

  /// The subdomains that have body force residual objects
  std::set<SubdomainID> _sub_ids_with_body_forces;

  /// Whether we have performed our initial setup. Ordinarily we would do this in initialSetup but
  /// there are wonky things that happen in other objects initialSetup that affect us, like how
  /// Exodus output gathers all elements to 0 on distributed mesh
  bool _initial_setup_done = false;

private:
  /**
   * Pushes and pulls 'a' coefficient data between processes such that each process has all the 'a'
   * coefficient data necessary to construct any requested Rhie-Chow velocity
   */
  void finalizeAData();

  /**
   * Performs the first and second overbar operations on 'B', e.g. one interpolation and one
   * reconstruction
   */
  void computeFirstAndSecondOverBars();

  /**
   * Applies the interpolated and reconstructed 'B' data to the momentum residuals
   */
  void applyBData();

  /**
   * Pushes and pulls 'B' data between processes such that each process has all the 'B'
   * data necessary to construct any requested Rhie-Chow velocity
   */
  void finalizeBData();

  /// The velocity variable numbers
  std::vector<unsigned int> _var_numbers;

  /// Non-local elements that we should push and pull data for across processes
  std::unordered_set<const Elem *> _elements_to_push_pull;

  /// The nonlinear system
  SystemBase & _sys;

  /// An example datum used to help communicate AD vector information in parallel
  const VectorValue<ADReal> _example;

  /// If this is true, then interpolation and reconstruction operations will not be performed on the
  /// body forces
  const bool _standard_body_forces;

  /**
   * @name Body Force Component Functors
   * These vector component functors are not used anywhere within this class but they can be used
   * for outputting, to auxiliary variables, either the non-interpolated or interpolated body
   * forces
   */
  ///@{
  /// The x-component of the non-interpolated body forces
  VectorComponentFunctor<ADReal> _bx;

  /// The y-component of the non-interpolated body forces
  VectorComponentFunctor<ADReal> _by;

  /// The z-component of the non-interpolated body forces
  VectorComponentFunctor<ADReal> _bz;

  /// The x-component of the interpolated and reconstructed body forces
  VectorComponentFunctor<ADReal> _b2x;

  /// The y-component of the interpolated and reconstructed body forces
  VectorComponentFunctor<ADReal> _b2y;

  /// The z-component of the interpolated and reconstructed body forces
  VectorComponentFunctor<ADReal> _b2z;
  ///@}

  /// Mutex that prevents multiple threads from saving into the 'a' coefficients at the same time
  Threads::spin_mutex _a_mutex;

  /// Mutex that prevents multiple threads from saving into the 'B' data at the same time
  Threads::spin_mutex _b_mutex;

  /// A unity functor used in the epsilon virtual method
  const Moose::ConstantFunctor<ADReal> _unity_functor{1};
};

inline const Moose::FunctorBase<ADReal> & INSFVRhieChowInterpolator::epsilon(THREAD_ID) const
{
  return _unity_functor;
}

inline void
INSFVRhieChowInterpolator::addToA(const Elem * const elem,
                                  const unsigned int component,
                                  const ADReal & value)
{
  Threads::spin_mutex::scoped_lock lock(_a_mutex);

  if (elem->processor_id() != this->processor_id())
    _elements_to_push_pull.insert(elem);

  _a[elem->id()](component) += value;
}

inline void
INSFVRhieChowInterpolator::addToB(const Elem * const elem,
                                  const unsigned int component,
                                  const ADReal & value)
{
  mooseAssert(elem->processor_id() == this->processor_id(), "Sources should be local");

  Threads::spin_mutex::scoped_lock lock(_b_mutex);
  // We have our users write their RC data imagining that they've moved all terms to the LHS, but
  // the balance in Moukalled assumes that the body forces are on the RHS with positive sign, e.g.
  // 0 = -\nabla p + \mathbf{B}, so we must apply a minus sign here
  _b[elem->id()](component) -= value;
}
