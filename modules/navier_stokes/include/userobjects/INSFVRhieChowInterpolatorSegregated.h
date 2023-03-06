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
 * Something nice here at some point
 */
class INSFVRhieChowInterpolatorSegregated : public RhieChowInterpolatorBase
{
public:
  static InputParameters validParams();
  INSFVRhieChowInterpolatorSegregated(const InputParameters & params);

  VectorValue<ADReal> getVelocity(cconst FaceInfo & fi,
                                  const Moose::StateArg & time,
                                  THREAD_ID tid,
                                  Moose::FV::InterpMethod m) const override;

  void initFaceVelocities();

  void computeVelocity();

  void addToA(const libMesh::Elem * /*elem*/,
              unsigned int /*component*/,
              const ADReal & /*value*/) override
  {
    mooseError(
        "addToA function is not implemented for the RhieChow interpolation in segregated solvers.");
  }

  void meshChanged() override;

  void initialize() override;
  void execute() override;
  void finalize() override{};

  /// Bool of the Rhie Chow user object is used in monolithic/segregated approaches
  bool segregated() const override { return true; };

  void linkMomentumSystem(NonlinearSystemBase & momentum_system, const TagID & momentum_tag);

protected:
  /**
   * Computes the inverse of the digaonal (1/A) of the system matrix plus the H/A components for the
   * pressure equation plus Rhie-Chow interpolation. This should nly be used with segregated
   * solvers.
   */
  void computeHbyA();

  /**
   * A map from element IDs to $HbyA_{ij} = (A_{offdiag}*\mathrm{(predicted~velocity)} -
   * \mathrm{Source})_{ij}/A_{ij}$. So this contains the offdiagonal part of the system matrix
   * multiplied by the predicted velocity minus the source terms from the right hand side of the
   * linearized momentum predictor stem.
   */
  CellCenteredMapFunctor<ADRealVectorValue, std::unordered_map<dof_id_type, RealVectorValue>> _HbyA;

  /**
   * A map from element IDs to $1/A_ij$. ADD MORE
   */
  CellCenteredMapFunctor<ADRealVectorValue, std::unordered_map<dof_id_type, RealVectorValue>> _Ainv;

  /// A functor for computing the (non-RC corrected) velocity
  std::unique_ptr<PiecewiseByBlockLambdaFunctor<ADRealVectorValue>> _vel;

  FaceCenteredMapFunctor<ADRealVectorValue, std::unordered_map<dof_id_type, RealVectorValue>>
      _face_velocity;

  /// Reference to the nonlinear system corresponding to the momentum equation
  NonlinearSystemBase * _momentum_sys;
  /// Reference to the nonlinear system corresponding to the pressure equation
  TagID _momentum_tag;
};
