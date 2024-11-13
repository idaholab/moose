//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RhieChowInterpolatorBase.h"
#include "CellCenteredMapFunctor.h"
#include "VectorComponentFunctor.h"
#include "VectorCompositeFunctor.h"

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
 * This user-object gathers 'a' (on-diagonal velocity coefficients) data. Having the gathered 'a'
 * data, this object is responsible for the computation of the Rhie-Chow velocity, which can be used
 * in advection kernels and postprocessors. This class also supports computation of an average face
 * velocity although this is generally not encouraged as it will lead to a checkerboard in the
 * pressure field
 */

class INSFVRhieChowInterpolator : public RhieChowInterpolatorBase
{
public:
  /**
   * Parameters of this object that should be added to the NSFV action that are unique to this
   * object
   */
  static InputParameters uniqueParams();

  /**
   * @returns A list of the parameters that are common between this object and the NSFV action
   */
  static std::vector<std::string> listOfCommonParams();

  static InputParameters validParams();
  INSFVRhieChowInterpolator(const InputParameters & params);

  /**
   * API that momentum residual objects that have on-diagonals for velocity call
   * @param The element we are adding 'a' coefficient data for
   * @param component The velocity component we are adding 'a' coefficient data for
   * @param value The value of 'a' that we are adding
   */
  virtual void
  addToA(const libMesh::Elem * elem, unsigned int component, const ADReal & value) override;

  /**
   * Retrieve a face velocity
   * @param m The velocity interpolation method. This is either Rhie-Chow or Average. Rhie-Chow is
   * recommended as it avoids checkerboards in the pressure field
   * @param fi The face that we wish to retrieve the velocity for
   * @param time The time at which to evaluate the velocity
   * @param tid The thread ID
   * @param subtract_mesh_velocity Whether to subtract the mesh velocity if running on a displaced
   * mesh
   * @return The face velocity
   */
  virtual VectorValue<ADReal> getVelocity(const Moose::FV::InterpMethod m,
                                          const FaceInfo & fi,
                                          const Moose::StateArg & time,
                                          const THREAD_ID tid,
                                          bool subtract_mesh_velocity) const override;

  virtual void initialSetup() override;
  virtual void meshChanged() override;

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

  virtual bool segregated() const override { return false; };

  /**
   * makes sure coefficient data gets communicated on both sides of a given boundary
   */
  virtual void ghostADataOnBoundary(const BoundaryID boundary_id) override;

  /**
   * Whether to pull all 'a' coefficient data from the owning process for all nonlocal elements we
   * have access to (e.g. all of our nonlocal elements we have pointers to)
   */
  void pullAllNonlocal() { _pull_all_nonlocal = true; }

  /**
   * Whether central differencing face interpolations of velocity should include a skewness
   * correction
   * Also used for the face interpolation of the D coefficient
   * and the face interpolation of volumetric forces for for the volume correction method
   * and the face interpolation of porosity
   */
  bool velocitySkewCorrection(THREAD_ID tid) const;

  /**
   * Whether central differencing face interpolations of pressure should include a skewness
   * correction
   */
  bool pressureSkewCorrection(THREAD_ID tid) const;

protected:
  /**
   * perform the setup of this object
   */
  void insfvSetup();

  /// A functor for computing the (non-RC corrected) velocity
  std::vector<std::unique_ptr<PiecewiseByBlockLambdaFunctor<ADRealVectorValue>>> _vel;

  /// All the thread copies of the x-displacement variable
  std::vector<MooseVariableField<Real> *> _disp_xs;

  /// All the thread copies of the y-displacement variable
  std::vector<MooseVariableField<Real> *> _disp_ys;

  /// All the thread copies of the z-displacement variable
  std::vector<MooseVariableField<Real> *> _disp_zs;

  /// A functor for computing the displacement
  std::vector<std::unique_ptr<Moose::VectorCompositeFunctor<ADReal>>> _disps;

  /// All the active and elements local to this process that exist on this object's subdomains
  std::unique_ptr<ConstElemRange> _elem_range;

  /// A map from element IDs to 'a' coefficient data
  CellCenteredMapFunctor<ADRealVectorValue, std::unordered_map<dof_id_type, ADRealVectorValue>> _a;

  /**
   * @name 'a' component functors
   * These vector component functors are not used anywhere within this class but they can be used
   * for outputting, to auxiliary variables, the on-diagonal 'a' coefficients for use in
   * visualization or transfer to other applications
   */
  ///@{
  /// The x-component of 'a'
  Moose::VectorComponentFunctor<ADReal> _ax;

  /// The y-component of 'a'
  Moose::VectorComponentFunctor<ADReal> _ay;

  /// The z-component of 'a'
  Moose::VectorComponentFunctor<ADReal> _az;
  ///@}

  /// The number of the nonlinear system in which the monolithic momentum and continuity equations are located
  const unsigned int _momentum_sys_number;

private:
  /**
   * Fills the _a_read data member at construction time with the appropriate functors. _a_read will
   * be used later when computing the Rhie-Chow velocity
   */
  void fillARead();

  /**
   * Whether we need 'a' coefficient computation
   */
  bool needAComputation() const;

  /// Non-local elements that we should push and pull data for across processes
  std::unordered_set<const Elem *> _elements_to_push_pull;

  /// An example datum used to help communicate AD vector information in parallel
  const VectorValue<ADReal> _example;

  /// Mutex that prevents multiple threads from saving into the 'a' coefficients at the same time
  Threads::spin_mutex _a_mutex;

  /// A vector sized according to the number of threads that holds the 'a' data we will read from
  /// when computing the Rhie-Chow velocity
  std::vector<const Moose::FunctorBase<VectorValue<ADReal>> *> _a_read;

  /// A vector sized according to the number of threads that holds vector composites of 'a'
  /// component functors. This member is leveraged when advecting velocities are auxiliary variables
  /// and the 'a' data has been transferred from another application
  std::vector<std::unique_ptr<Moose::FunctorBase<VectorValue<ADReal>>>> _a_aux;

  /// Whether 'a' data has been provided by the user. This can happen if we are running in an
  /// application solving precursor advection, and another application has computed the fluid flow
  /// field
  bool _a_data_provided;

  /// Whether we want to pull all nonlocal 'a' coefficient data
  bool _pull_all_nonlocal;

  /// Correct Rhie-Chow coefficients for volumetric force flag
  const bool & _bool_correct_vf;

  /// -- Method used for computing the properties average
  const MooseEnum _volume_force_correction_method;

  /// Names of the functors storing the volumetric forces
  const std::vector<MooseFunctorName> * _volumetric_force_functors;

  /// Values of the functors storing the volumetric forces
  std::vector<const Moose::Functor<Real> *> _volumetric_force;

  /// Minimum absolute RC force over the domain
  Real _baseline_volume_force;

  /// A zero functor potentially used in _a_read
  const Moose::ConstantFunctor<ADReal> _zero_functor{0};
};

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

inline bool
INSFVRhieChowInterpolator::needAComputation() const
{
  // We dont check for "a"s being in another nonlinear system here, only being auxiliary
  return !_a_data_provided && _velocity_interp_method == Moose::FV::InterpMethod::RhieChow;
}

inline bool
INSFVRhieChowInterpolator::velocitySkewCorrection(const THREAD_ID tid) const
{
  const auto * const u = _us[tid];
  return (u->faceInterpolationMethod() == Moose::FV::InterpMethod::SkewCorrectedAverage);
}

inline bool
INSFVRhieChowInterpolator::pressureSkewCorrection(const THREAD_ID tid) const
{
  const auto * const p = _ps[tid];
  return (p->faceInterpolationMethod() == Moose::FV::InterpMethod::SkewCorrectedAverage);
}
