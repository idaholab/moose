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
#include "FaceCenteredMapFunctor.h"
#include "VectorComponentFunctor.h"
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
 * A user object which implements the Rhie Chow interpolation for segregated
 * momentum-pressure systems.
 */
class INSFVRhieChowInterpolatorSegregated : public RhieChowInterpolatorBase
{
public:
  static InputParameters validParams();
  INSFVRhieChowInterpolatorSegregated(const InputParameters & params);

  /// Get the face velocity (used in advection terms)
  VectorValue<ADReal> getVelocity(const Moose::FV::InterpMethod m,
                                  const FaceInfo & fi,
                                  const Moose::StateArg & time,
                                  const THREAD_ID tid,
                                  bool subtract_mesh_velocity) const override;

  /// Initialize the container for face velocities
  void initFaceVelocities();
  /// Update the values of the face velocities in the containers
  void computeFaceVelocity();
  /// Update the cell values of the velocity variables
  void computeCellVelocity();

  /// We disable this for the segregated solver
  void addToA(const libMesh::Elem * /*elem*/,
              unsigned int /*component*/,
              const ADReal & /*value*/) override
  {
    mooseError(
        "addToA function is not implemented for the RhieChow interpolation in segregated solvers.");
  }

  void meshChanged() override;
  void initialize() override;
  void execute() override {}
  void finalize() override {}

  bool segregated() const override { return true; };

  /**
   * Update the momentum system-related information
   * @param momentum_systems Pointers to the momentum systems which are solved for the momentum
   * vector components
   * @param momentum_system_numbers The numbers of these systems
   * @param pressure_gradient_tag The tag which is associated with the pressure gradient kernels.
   * This is needed for separating the pressure contibution from other terms in the momentum
   * systems.
   */
  void linkMomentumSystem(std::vector<NonlinearSystemBase *> momentum_systems,
                          const std::vector<unsigned int> & momentum_system_numbers,
                          const TagID pressure_gradient_tag);

  /**
   * Computes the inverse of the digaonal (1/A) of the system matrix plus the H/A components for the
   * pressure equation plus Rhie-Chow interpolation.
   */
  void computeHbyA(bool verbose);

protected:
  /// Populate the face values of the H/A field
  void populateHbyA(const std::vector<std::unique_ptr<NumericVector<Number>>> & raw_hbya,
                    const std::vector<unsigned int> & var_nums);
  /**
   * A map functor from faces to $HbyA_{ij} = (A_{offdiag}*\mathrm{(predicted~velocity)} -
   * \mathrm{Source})_{ij}/A_{ij}$. So this contains the off-diagonal part of the system matrix
   * multiplied by the predicted velocity minus the source terms from the right hand side of the
   * linearized momentum predictor step.
   */
  FaceCenteredMapFunctor<RealVectorValue, std::unordered_map<dof_id_type, RealVectorValue>> _HbyA;

  /**
   * We hold on to the cell-based HbyA vectors so that we can easily reconstruct the
   * cell velocities as well. This vector might be either of size 1 or DIM depending on if we
   * segregate the velocity components as well.
   */
  std::vector<std::unique_ptr<NumericVector<Number>>> _HbyA_raw;

  /**
   * A map functor from element IDs to $1/A_i$. Where $A_i$ is the diagonal of the system matrix
   * for the momentum equation.
   */
  CellCenteredMapFunctor<RealVectorValue, std::unordered_map<dof_id_type, RealVectorValue>> _Ainv;

  /// A functor for computing the (non-RC corrected) velocity
  std::unique_ptr<PiecewiseByBlockLambdaFunctor<ADRealVectorValue>> _vel;

  /**
   * A map functor from faces to face velocities which are used in the advection terms
   */
  FaceCenteredMapFunctor<RealVectorValue, std::unordered_map<dof_id_type, RealVectorValue>>
      _face_velocity;

  /// Pointers to the nonlinear system(s) corresponding to the momentum equation(s)
  std::vector<NonlinearSystemBase *> _momentum_systems;

  /// Numbers of the momentum system(s)
  std::vector<unsigned int> _momentum_system_numbers;

  /// Pointers to the momentum equation implicit system(s)
  std::vector<libMesh::NonlinearImplicitSystem *> _momentum_implicit_systems;

  /// Residual tag corresponding to the pressure gradient contribution
  TagID _pressure_gradient_tag;
};
