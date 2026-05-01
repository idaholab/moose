//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RhieChowFaceFluxProvider.h"
#include "CellCenteredMapFunctor.h"
#include "FaceCenteredMapFunctor.h"
#include "VectorComponentFunctor.h"
#include "LinearFVElementalKernel.h"
#include <string>
#include <unordered_map>
#include <set>
#include <unordered_set>

#include "libmesh/petsc_vector.h"

class MooseMesh;
class INSFVVelocityVariable;
class INSFVPressureVariable;
class LinearFVPressureCorrectionDiffusion;
class Function;
namespace libMesh
{
class Elem;
class MeshBase;
}

/**
 * User object responsible for determining the face fluxes using the Rhie-Chow interpolation in a
 * segregated solver that uses the linear FV formulation.
 */
class RhieChowMassFlux : public RhieChowFaceFluxProvider, public NonADFunctorInterface
{
public:
  struct FaceMassFluxConsistencyAudit
  {
    Real l2_norm = 0.0;
    Real internal_l2_norm = 0.0;
    Real boundary_l2_norm = 0.0;
    Real max_abs_mismatch = 0.0;
    Real max_abs_internal_mismatch = 0.0;
    Real max_abs_boundary_mismatch = 0.0;
    bool has_worst_face = false;
    bool worst_face_is_boundary = false;
    dof_id_type worst_face_id = 0;
    Point worst_face_centroid;
    RealVectorValue worst_face_normal;
  };

  static InputParameters validParams();
  RhieChowMassFlux(const InputParameters & params);

  /// Get the face velocity times density (used in advection terms)
  virtual Real getMassFlux(const FaceInfo & fi) const;

  /// Get the volumetric face flux (used in advection terms)
  virtual Real getVolumetricFaceFlux(const FaceInfo & fi) const;

  virtual Real getVolumetricFaceFlux(const Moose::FV::InterpMethod m,
                                     const FaceInfo & fi,
                                     const Moose::StateArg & time,
                                     const THREAD_ID tid,
                                     bool subtract_mesh_velocity) const override;

  /// Initialize the container for face velocities
  virtual void initFaceMassFlux();
  /// Initialize the coupling fields (HbyA and Ainv)
  void initCouplingField();
  /// Cache the exact pressure-equation face flux from the current solved pressure field.
  void cachePressureEquationFlux();
  /// Evaluate the discrete internal-face pressure-equation flux for a supplied exact pressure.
  Real exactInternalPressureEquationFlux(const FaceInfo & fi, const Function & exact_pressure) const;
  /// Access the currently stored pressure-equation face flux.
  Real storedPressureEquationFlux(const FaceInfo & fi) const;
  /// Signed sum of boundary mass fluxes for audit purposes.
  Real boundaryMassFluxImbalance() const;
  /// Maximum absolute boundary mass flux for audit purposes.
  Real maxBoundaryMassFluxMagnitude() const;
  /// L2 norm of all current face mass fluxes for audit purposes.
  Real faceMassFluxL2Norm() const;
  /// Access the active flow faces used by this Rhie-Chow object for audit postprocessors.
  const std::vector<const FaceInfo *> & flowFacesForAudit() const { return _flow_face_info; }
  /// Compare the stored face mass fluxes against the flux implied by the current cell/boundary U.
  FaceMassFluxConsistencyAudit faceMassFluxConsistencyAudit() const;
  /// Update the values of the face velocities in the containers
  virtual void computeFaceMassFlux();
  /// Update the cell values of the velocity variables
  virtual void computeCellVelocity();
  /// Debug accessor for the current cell HbyA state.
  Real cellHbyARaw(const unsigned int system_i, const dof_id_type dof) const;
  /// Debug accessor for the current cell Ainv state.
  Real cellAinvRaw(const unsigned int system_i, const dof_id_type dof) const;
  /// Cache the assembled/relaxed momentum predictor operator for one component.
  void cacheMomentumPredictorOperator(const unsigned int system_i,
                                      const NumericVector<Number> * rhs_override = nullptr,
                                      const NumericVector<Number> * explicit_force = nullptr,
                                      const NumericVector<Number> * body_force = nullptr);
  /// Invalidate the cached assembled/relaxed momentum predictor operator.
  void clearMomentumPredictorOperatorCache();
  /// Add explicit forcing to the momentum predictor RHS after the base operator assembly.
  virtual void addMomentumPredictorExplicitForcing(const unsigned int system_i,
                                                   NumericVector<Number> & rhs) const;
  /// Add only the non-pressure/body-force portion of the explicit predictor forcing.
  virtual void addMomentumPredictorBodyForceForcing(const unsigned int system_i,
                                                    NumericVector<Number> & rhs) const;
  /// Whether the predictor operator excludes explicit pressure/body-force terms.
  bool splitMomentumPredictorOperator() const { return _split_momentum_predictor_operator; }
  /// Update boundary pressure gradients from the current predictor and boundary velocity state.
  void updatePressureBoundaryNormalGradients(const bool apply_reference_adjustment);
  /// Audit representative top/left pressure-boundary constraint state.
  void auditPressureBoundaryGradientState(const std::string & stage_label) const;
  /// Refresh boundary-face velocity values from the active FV velocity BC objects.
  virtual void updateVelocityBoundaryState();

  virtual void meshChanged() override;
  virtual void initialize() override;
  virtual void execute() override {}
  virtual void finalize() override {}
  virtual void initialSetup() override;

  /**
   * Update the momentum system-related information
   * @param momentum_systems Pointers to the momentum systems which are solved for the momentum
   * vector components
   * @param pressure_system Reference to the pressure system
   * @param momentum_system_numbers The numbers of these systems
   */
  void linkMomentumPressureSystems(const std::vector<LinearSystem *> & momentum_systems,
                                   const LinearSystem & pressure_system,
                                   const std::vector<unsigned int> & momentum_system_numbers);

  /**
   * Computes the inverse of the diagonal (1/A) of the system matrix plus the H/A components for the
   * pressure equation plus Rhie-Chow interpolation.
   */
  void computeHbyA(const bool with_updated_pressure, const bool verbose);

protected:
  /// Predictor-side face flux entering the pressure equation. Defaults to the HbyA contribution.
  virtual Real pressurePredictorFlux(const FaceInfo * fi) const;
  /// Populate the explicit face state used by the pressure corrector.
  void updatePressurePredictorFaceState();
  /// Optional per-face adjustment added to the predictor flux for reference-pressure compatibility.
  Real pressurePredictorFluxAdjustment(const FaceInfo * fi) const;
  /// Evaluate or fetch the boundary face value for a velocity component.
  Real boundaryVelocityValue(const FaceInfo * fi,
                            const unsigned int component,
                            const Moose::StateArg & time_arg) const;
  /// Compute the target physical boundary mass flux rho U_b.n at a face.
  Real boundaryMassFluxTarget(const FaceInfo * fi, const Moose::StateArg & time_arg) const;
  /// Compute the normal component of the face diffusion coefficient used by pressure BCs.
  Real boundaryNormalAinv(const FaceInfo * fi) const;
  /// Determine whether a boundary face belongs to an adjustable fixed-flux pressure patch.
  bool isAdjustablePressureBoundaryFace(const FaceInfo * fi) const;

  /// Accessor for the cached set of flow faces used by this Rhie-Chow object.
  const std::vector<const FaceInfo *> & flowFaceInfo() const { return _flow_face_info; }

  /// Select the right pressure gradient field and return a reference to the container
  std::vector<std::unique_ptr<NumericVector<Number>>> &
  selectPressureGradient(const bool updated_pressure);

  /// Compute the cell volumes on the mesh
  void setupMeshInformation();

  /// Populate the face values of the H/A and 1/A fields
  void
  populateCouplingFunctors(const std::vector<std::unique_ptr<NumericVector<Number>>> & raw_hbya,
                           const std::vector<std::unique_ptr<NumericVector<Number>>> & raw_Ainv);

  /// Build the base predictor operator vectors M*u - A*u - rhs and the relaxed diagonal.
  void computePredictorOperatorBase(const unsigned int system_i,
                                    NumericVector<Number> & base_raw,
                                    NumericVector<Number> & diagonal_raw,
                                    const NumericVector<Number> * rhs_override = nullptr) const;

  /// Whether the cached predictor-operator state is complete and can be consumed.
  bool canUseCachedMomentumPredictorOperator() const;

  /**
   * Check the block consistency between the passed in \p var and us
   */
  template <typename VarType>
  void checkBlocks(const VarType & var) const;

  virtual bool supportMeshVelocity() const override { return false; }

  /// The \p MooseMesh that this user object operates on
  const MooseMesh & _moose_mesh;

  /// The \p libMesh mesh that this object acts on
  const libMesh::MeshBase & _mesh;

  /// The dimension of the mesh, e.g. 3 for hexes and tets, 2 for quads and tris
  const unsigned int _dim;

  /// The thread 0 copy of the pressure variable
  const MooseLinearVariableFVReal * const _p;

  /// The thread 0 copy of the x-velocity variable
  std::vector<const MooseLinearVariableFVReal *> _vel;

  /// Pointer to the pressure diffusion term in the pressure Poisson equation
  LinearFVPressureCorrectionDiffusion * _p_diffusion_kernel;

  /**
   * A map functor from faces to $HbyA_{ij} = (A_{offdiag}*\mathrm{(predicted~velocity)} -
   * \mathrm{Source})_{ij}/A_{ij}$. So this contains the off-diagonal part of the system matrix
   * multiplied by the predicted velocity minus the source terms from the right hand side of the
   * linearized momentum predictor step.
   */
  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> _HbyA_flux;

  /**
   * Explicit predictor-side face flux entering the pressure equation, i.e. the local analog of
   * OpenFOAM's phiHbyA after any local predictor-source additions.
   */
  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> _phiHbyA_flux;

  /**
   * Explicit face-force contribution entering the pressure corrector, i.e. the local analog of
   * OpenFOAM's phig term.
   */
  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> _phig_flux;

  /**
   * We hold on to the cell-based HbyA vectors so that we can easily reconstruct the
   * cell velocities as well.
   */
  std::vector<std::unique_ptr<NumericVector<Number>>> _HbyA_raw;

  /**
   * A map functor from faces to $(1/A)_f$. Where $A_i$ is the diagonal of the system matrix
   * for the momentum equation.
   */
  FaceCenteredMapFunctor<RealVectorValue, std::unordered_map<dof_id_type, RealVectorValue>> _Ainv;

  /**
   * We hold on to the cell-based 1/A vectors so that we can easily reconstruct the
   * cell velocities as well.
   */
  std::vector<std::unique_ptr<NumericVector<Number>>> _Ainv_raw;

  std::unique_ptr<NumericVector<Number>> _A_avg;

  /**
   * A map functor from faces to mass fluxes which are used in the advection terms.
   */
  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> & _face_mass_flux;

  /**
   * Exact face flux contribution from the solved pressure equation, in the same sign convention
   * used in face_mass_flux = -predictor_flux + pressure_equation_flux.
   */
  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> _pressure_equation_flux;

  /**
   * Cached boundary-normal pressure gradient used to emulate OpenFOAM's constrainPressure /
   * fixedFluxPressure patch update against the current predictor flux.
   */
  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>>
      _pressure_boundary_normal_gradient;

  /// Indicates whether _pressure_equation_flux matches the latest solved pressure field.
  bool _pressure_equation_flux_valid = false;

  /// Per-face predictor source-flux adjustments used for the local adjustPhi analogue.
  std::unordered_map<dof_id_type, Real> _pressure_predictor_flux_adjustment;

  /// Unadjusted predictor-side face flux prior to the local adjustPhi analogue.
  std::unordered_map<dof_id_type, Real> _pressure_predictor_base_flux;

  /// Indicates whether _pressure_boundary_normal_gradient matches the latest predictor state.
  bool _pressure_boundary_normal_gradient_valid = false;

  /// Indicates whether _phiHbyA_flux/_phig_flux match the latest predictor state.
  bool _pressure_predictor_face_state_valid = false;

  /// Boundary-face velocity values refreshed from the current FV velocity BCs.
  std::vector<std::unordered_map<dof_id_type, Real>> _boundary_velocity_face_values;

  /// Indicates whether _boundary_velocity_face_values matches the latest cell-centered velocity.
  bool _velocity_boundary_state_valid = false;

  /// Whether to consume a cached assembled predictor operator instead of rebuilding HbyA live.
  const bool _use_cached_momentum_predictor_operator;

  /// Whether the momentum predictor operator already excludes explicit pressure/body-force terms.
  const bool _split_momentum_predictor_operator;

  /// Cached base predictor operator M*u - A*u - rhs for each momentum component.
  std::vector<std::unique_ptr<NumericVector<Number>>> _cached_predictor_operator_base_raw;

  /// Cached relaxed predictor diagonal for each momentum component.
  std::vector<std::unique_ptr<NumericVector<Number>>> _cached_predictor_diagonal_raw;

  /// Cached explicit predictor forcing applied after the pressure-free operator assembly.
  std::vector<std::unique_ptr<NumericVector<Number>>> _cached_predictor_explicit_force_raw;

  /// Cached predictor body-force forcing, excluding the explicit pressure-gradient term.
  std::vector<std::unique_ptr<NumericVector<Number>>> _cached_predictor_body_force_raw;

  /// Indicates whether the cached predictor operator has been populated for every component.
  bool _cached_predictor_operator_valid = false;

  /// Pointer to the body force terms
  std::vector<std::vector<LinearFVElementalKernel *>> _body_force_kernels;
  /// Vector of body force term names
  std::vector<std::vector<std::string>> _body_force_kernel_names;

  /**
   * for a PISO iteration we need to hold on to the original pressure gradient field.
   * Should not be used in other conditions.
   */
  std::vector<std::unique_ptr<NumericVector<Number>>> _grad_p_current;

  /**
   * Functor describing the density of the fluid
   */
  const Moose::Functor<Real> & _rho;

  /// Pointers to the linear system(s) in moose corresponding to the momentum equation(s)
  std::vector<LinearSystem *> _momentum_systems;

  /// Numbers of the momentum system(s)
  std::vector<unsigned int> _momentum_system_numbers;

  /// Global numbers of the momentum system(s)
  std::vector<unsigned int> _global_momentum_system_numbers;

  /// Pointers to the momentum equation implicit system(s) from libmesh
  std::vector<libMesh::LinearImplicitSystem *> _momentum_implicit_systems;

  /// Pointer to the pressure system
  const LinearSystem * _pressure_system;

  /// Global number of the pressure system
  unsigned int _global_pressure_system_number;

  /// We will hold a vector of cell volumes to make sure we can do volume corrections rapidly
  std::unique_ptr<NumericVector<Number>> _cell_volumes;

  /// Enumerator for the method used for pressure projection
  const MooseEnum _pressure_projection_method;

  /// Interpolation method used for the pressure diffusion coefficient on faces
  const Moose::FV::InterpMethod _pressure_diffusion_interp_method;

private:
  /// The subset of the FaceInfo objects that actually cover the subdomains which the
  /// flow field is defined on. Cached for performance optimization.
  std::vector<const FaceInfo *> _flow_face_info;
};

template <typename VarType>
void
RhieChowMassFlux::checkBlocks(const VarType & var) const
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
