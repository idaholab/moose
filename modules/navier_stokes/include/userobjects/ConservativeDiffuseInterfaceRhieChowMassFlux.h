//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RhieChowMassFlux.h"
#include "DerivedFaceFunctor.h"
#include "PetscVectorReader.h"

class ElemInfo;

/**
 * Rhie-Chow face-flux provider of the reduced-pressure diffuse-interface two-phase solve. The
 * momentum predictor is assembled without the pressure, gravity and capillary forces, which this
 * object adds to the predictor and carries into the pressure equation as face fluxes; the face
 * mass flux it publishes is the one the limited volume-fraction transport advanced with.
 *
 * The pressure is the reduced pressure p_rgh = p - rho g.(x - x_ref). With it the momentum
 * equation reads
 *   d(rho u)/dt + div(rho_phi u) = -grad(p_rgh) - g.(x - x_ref) grad(rho) + div(tau) + f_sigma,
 * so that at rest, where grad(p_rgh) = 0, the hydrostatic force and the pressure gradient cancel
 * on each face exactly instead of leaving a discretization residual that would drive spurious
 * currents at the density jump. The two face forces, -gh grad(rho) and the capillary f_sigma,
 * are evaluated on faces as normal components and enter the pressure equation as the flux
 * phig = (-gh dn(rho) + f_sigma.n) (1/A)_f S_f.
 *
 * One outer iteration of the executioner drives this object in the following order:
 * 1. beginMomentumPredictorAssembly / momentumPredictorAssembled, once per velocity component:
 *    the assembled momentum matrix and right-hand side are split into the operator parts
 *    fvMatrix::A() and H() need (see MomentumPredictorOperator); the explicit forces of
 *    addMomentumPredictorExplicitForcing are added to the predictor's right-hand side.
 * 2. computeHbyA: H/A and 1/A per cell from those parts.
 * 3. updateAdditionalPressureFluxFunctors: the face fields of the pressure equation, that is
 *    phiHbyA (predictor flux plus transient correction minus phig), pressure_Ainv (the raw face
 *    1/A) and phig itself, all area-integrated.
 * 4. updateVelocityBoundaryState and updatePressureBoundaryNormalGradients: the boundary
 *    velocity values and the pressure normal gradient a fixed-flux boundary needs so that the
 *    corrected flux there equals the prescribed one.
 * 5. The pressure equation div((1/A)_f grad(p_rgh).S_f) = div(phiHbyA) is solved by the
 *    executioner; cachePressureEquationFlux reads the face flux its diffusion kernel assembled.
 * 6. computeFaceMassFlux and computeCellVelocity: the corrected transport flux
 *    phi = (-phiHbyA + pressure flux) / S_f closes every cell, and the cell velocity is written
 *    back as u = -H/A + (1/A) R(delta phi / (1/A)_n), with R the least-squares reconstruction of
 *    a cell vector from face-normal scalars.
 * 7. freezeLaggedCorrectedFaceFlux at the start of the next outer iteration: the corrected flux
 *    is copied into lagged_corrected_face_phi, which the volume-fraction equation and its
 *    boundary conditions read, so the limited flux and the backflow switches of one outer
 *    iteration are all evaluated from one flux.
 *
 * Sign convention:
 * - The face functors published here are PRESSURE-EQUATION SOURCE FLUXES in the
 *   same sign convention as the stock HbyA functor used by LinearFVDivergence.
 * - The physical face mass-flux correction is therefore the negative of the
 *   published source functor. This mirrors the stock MOOSE relation
 *     face_mass_flux = -HbyA + pressure_diffusion_flux.
 * - Unlike the parent, the face fluxes of the pressure equation are area-integrated
 *   (units of volume per time), and the transport fluxes read back from them are divided by the
 *   face measure again (units of velocity).
 */
class ConservativeDiffuseInterfaceRhieChowMassFlux : public RhieChowMassFlux
{
public:
  static InputParameters validParams();

  ConservativeDiffuseInterfaceRhieChowMassFlux(const InputParameters & params);

  void meshChanged() override;
  void initialSetup() override;
  void beginMomentumPredictorAssembly(const unsigned int system_i) override;
  /// Keeps the component's assembled, unrelaxed operator for the split H/A and 1/A
  void momentumPredictorAssembled(const unsigned int system_i,
                                  libMesh::SparseMatrix<Number> & matrix,
                                  const NumericVector<Number> & rhs,
                                  const Real relaxation_parameter) override;
  /// The predictor is assembled without the pressure and body forces, which
  /// addMomentumPredictorExplicitForcing supplies from the reduced pressure, gravity and
  /// capillarity; H/A then comes from the operator as assembled, with the boundary
  /// coefficients kept in A and H as the assembly placed them.
  bool splitMomentumPredictorOperator() const override { return true; }
  /// The reduced-pressure gradient, hydrostatic and capillary forces, which the predictor omits
  void addMomentumPredictorExplicitForcing(const unsigned int system_i,
                                           NumericVector<Number> & rhs) const override;
  /// The parent's H/A and 1/A; a new predictor invalidates every face field derived from the old
  void computeHbyA(const bool verbose) override;
  /// rho_f times the corrected transport flux, kept for the parent and for restart; momentum
  /// advection reads getMassFlux instead
  void computeFaceMassFlux() override;
  /// Writes the pressure-corrected cell velocity into the momentum systems' solutions
  void computeCellVelocity() override;
  /// The mass flux the limited volume-fraction transport advanced with, rho_phi, per unit area
  Real getMassFlux(const FaceInfo & fi) const override;
  /// The corrected transport flux, per unit area, in the direction of the face normal
  Real getVolumetricFaceFlux(const FaceInfo & fi) const override;
  // Keep the interpolation-method overload, which forwards to the one above, visible
  using RhieChowMassFlux::getVolumetricFaceFlux;
  /// Store the corrected flux and the boundary pressure state of a step that was accepted
  void commitAcceptedTimestepTransportHistory();
  /**
   * Seed the accepted-step transport history from the current corrected flux, only if it has
   * never been written. It is otherwise written once per accepted step, so a step solved again
   * and a recovered run both keep the history of the last step that was actually accepted.
   */
  void seedTransportHistoryOnce();
  /**
   * Discard the face state derived from the last solve. Every attempt at a timestep must start
   * from state rebuilt out of the restored solution, not from the attempt that was rejected.
   */
  virtual void timestepSetup() override;
  /**
   * Whether this process holds the cell behind \p fi without owning it. The flux on such a face
   * cannot be formed here: the face value a boundary condition needs there is defined through the
   * very flux being formed, so recomputing it locally closes a cycle that the lagged flux exists
   * to break. The owning process has already formed it over its own faces.
   */
  bool isHaloFace(const FaceInfo & fi) const;
  /**
   * Fill the faces this process holds but does not own with the values their owners computed.
   * A no-op in serial, where there are no such faces.
   */
  void synchronizeHaloFaceFlux(
      FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> & field) const;
  /**
   * Copy the corrected transport flux into lagged_corrected_face_phi, the flux the volume
   * fraction is transported with during the coming outer iteration. The first outer iteration of
   * a step copies the flux of the last accepted step instead, so that a step solved again starts
   * from the same flux as its first attempt.
   */
  void freezeLaggedCorrectedFaceFlux(const bool use_previous_timestep_flux = false);

  /// Update the additional pressure-equation source-flux functors before the pressure solve.
  void updateAdditionalPressureFluxFunctors();
  /// Update boundary pressure gradients from the current predictor and boundary velocity state.
  void updatePressureBoundaryNormalGradients();
  /// Refresh boundary-face velocity values from the active FV velocity BC objects.
  void updateVelocityBoundaryState();
  /// Maximum face-flux Courant number over active flow cells for the supplied timestep.
  Real maxCourant(const Real dt) const;

  /**
   * While the startup continuity projection runs, the pressure equation carries no transient,
   * hydrostatic or capillary source and the predictor force carries no hydrostatic part: that
   * projection repairs the face flux alone, and leaves the user's reduced pressure untouched.
   */
  void setStartupProjectionActive(const bool active) { _startup_projection_active = active; }
  /**
   * The explicit force density of the momentum predictor in a cell: -grad(p_rgh) - gh grad(rho)
   * + f_sigma, reconstructed by least squares from the face-normal components of the same three
   * terms on the cell's faces, so that the force the predictor sees is the one the pressure
   * equation's face fluxes carry.
   */
  RealVectorValue
  reducedPressureMomentumPredictorForceDensity(const ElemInfo & elem_info,
                                               const Moose::StateArg & time_arg) const;

protected:
  using FaceScalarField = FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>>;
  struct MomentumPredictorOperator;

  /// H/A and 1/A of one velocity component from the split operator of MomentumPredictorOperator
  void computePredictorOperator(const unsigned int system_i,
                                NumericVector<Number> & HbyA,
                                NumericVector<Number> & Ainv) override;

  /// The normal component of the pressure equation's face diffusion coefficient
  Real faceNormalPressureAinv(const FaceInfo & fi) const;
  bool pressureCorrectionFluxIsIntegrated() const override { return true; }
  /// The boundary velocity cached from the velocity boundary conditions while that cache matches
  /// the current cell velocity, the parent's boundary value otherwise
  Real boundaryVelocityValue(const FaceInfo & fi,
                             const unsigned int component,
                             const Moose::StateArg & time_arg) const override;
  /// Compute the normal component of a face diagonal Ainv vector.
  Real computeFaceNormalAinv(const RealVectorValue & face_ainv,
                             const RealVectorValue & face_normal) const;
  /// Interpolate per-component momentum-system cell readers to a face vector.
  RealVectorValue
  interpolateMomentumVectorReadersToFace(const FaceInfo & fi,
                                         const std::vector<PetscVectorReader> & readers,
                                         const Moose::FV::InterpMethod interp_method) const;

  /// Key every face field over all local faces and mark every derived state stale
  void initializeAdditionalPressureFluxStorage();
  /// u = -H/A + 1/A times the reconstructed face correction, into every momentum system; faces
  /// whose face-normal 1/A is below \p degenerate_normal_pressure_ainv_tol contribute nothing
  void
  writePressureCorrectedVelocityToMomentumSolution(const Real degenerate_normal_pressure_ainv_tol);
  /// The predictor flux of the pressure equation: the value of _phiHbyA_flux on a face
  Real phiHbyAFlux(const FaceInfo & fi) const;
  /// The face-normal velocity correction the pressure equation produced, divided by the face
  /// coefficient it was produced through, for the cell-centered velocity writeback.
  Real writebackReconstructionScalar(const FaceInfo & fi,
                                     const Real degenerate_normal_pressure_ainv_tol) const;

  /// The face density: interpolated on internal and constrained-velocity faces, the cell value
  /// on the other boundary faces, where the boundary pressure sets the flux
  Real interpolateFaceDensity(const FaceInfo & fi, const Moose::StateArg & time_arg) const;
  /// The face flux of the cell velocity interpolated to the face, with the boundary value on a
  /// boundary face
  Real interpolatedPhysicalFaceFlux(const FaceInfo & fi, const Moose::StateArg & time_arg) const;

  /**
   * The transient Rhie-Chow correction on an internal face: for each old
   * state the time integrator reads, the part of its corrected flux that its interpolated
   * velocity does not reproduce, weighted as the integrator weights that state in the time
   * derivative and scaled by \p normal_density_weighted_ainv, the face value of rho/A, which is
   * the coefficient the old states enter the predictor with. It keeps the converged step
   * independent of the time step size, which the plain Rhie-Chow interpolation of a transient
   * predictor is not, and using the integrator's own weights keeps it at the integrator's order.
   *
   * rho/A has to be interpolated as a product: rho/A is close to dt in every cell, so its face
   * value is close to dt on every face, while the separately interpolated face density times the
   * separately interpolated face 1/A is about rho_l / (4 rho_g) times dt on an interface face.
   * That mismatch turned a small old-flux mismatch into a predictor flux of meters per second on
   * the faces between gas and water cells, which the pressure solve absorbed and the velocity
   * writeback then reported as single-cell velocity spikes in the gas.
   */
  Real computeDefaultTransientProjectionVolumetricFlux(const FaceInfo & fi,
                                                       const Moose::StateArg & time_arg,
                                                       const Real normal_density_weighted_ainv);
  /// The face-normal density gradient evaluated from the density functor, uncached
  Real evaluateFaceNormalDensityGradient(const FaceInfo & fi,
                                         const Moose::StateArg & time_arg) const;
  /// Fill _face_normal_density_gradient for every flow face at the current state
  void updateFaceNormalDensityGradients();
  /**
   * The face-normal gradient of p_rgh the predictor force is built from: the two-point
   * difference on an internal face, and on a boundary face the gradient the pressure equation's
   * flux implies, or the one its boundary condition prescribes before a pressure solve exists.
   */
  Real computeFaceNormalPressureGradient(const FaceInfo & fi,
                                         const Moose::StateArg & time_arg) const;
  /// The hydrostatic face force -gh dn(rho) of the reduced-pressure formulation, with
  /// gh = g.(x_f - x_ref); zero while the startup projection runs, which carries no such force
  Real
  computeDiscreteHydrostaticPredictorFaceNormalForceDensity(const FaceInfo & fi,
                                                            const Moose::StateArg & time_arg) const;
  /// The normal component of the capillary force density sigma kappa grad(alpha) on a face
  Real surfaceTensionFaceNormalForceDensity(const FaceInfo & fi,
                                            const Moose::StateArg & time_arg) const;
  /// The cell velocity change the pressure correction implies, (1/A) times the reconstruction of
  /// the face scalars of writebackReconstructionScalar
  RealVectorValue
  reconstructPressureCoupledCellVelocityDelta(const ElemInfo & elem_info,
                                              const Real degenerate_normal_pressure_ainv_tol) const;
  /**
   * Whether a boundary face carries a velocity condition that fixes the face velocity: a
   * Dirichlet velocity, or a condition with a gradient coefficient such as a wall. The pressure
   * inlet-outlet velocity condition is the exception, there the pressure sets the flux. On a
   * constrained face the predictor flux is the prescribed velocity, not the extrapolated H/A.
   */
  bool useConstrainedBoundaryPredictorState(const FaceInfo & fi) const override;

  /**
   * Explicit predictor-side face flux entering the pressure equation in the native
   * pressure-correction sign convention. The accepted transport-face flux is therefore formed as
   * -_phiHbyA_flux + _pressure_equation_flux.
   */
  DerivedFaceFunctor<Real> _phiHbyA_flux;
  /// Explicit face-force contribution entering the pressure corrector.
  FaceScalarField _phig_flux;
  /// Face diffusion tensor of the pressure equation: the raw face rAUf.
  FaceCenteredMapFunctor<RealVectorValue, std::unordered_map<dof_id_type, RealVectorValue>>
      _pressure_Ainv;
  /// Boundary-normal pressure gradient the fixed-flux pressure boundaries impose
  FaceScalarField _pressure_boundary_normal_gradient;
  /// phiHbyA on each face: the predictor-side face flux, in the transport convention, minus the
  /// explicit face force _phig_flux; read through phiHbyAFlux
  std::unordered_map<dof_id_type, Real> _phiHbyA_face_values;
  /// Boundary-face velocity values refreshed from the current FV velocity BCs.
  std::vector<std::unordered_map<dof_id_type, Real>> _boundary_velocity_face_values;
  /// Whether _boundary_velocity_face_values matches the latest cell-centered velocity.
  bool _velocity_boundary_state_valid = false;
  /// Whether _pressure_boundary_normal_gradient matches the latest predictor state.
  bool _pressure_boundary_normal_gradient_valid = false;
  /// Face-normal density gradient at the current state. The density only moves when the volume
  /// fraction is solved, which precedes the predictor, so it is filled once per predictor assembly
  /// and read by the hydrostatic face force and the pressure-flux functors, which otherwise asked
  /// the density functor for a Green-Gauss gradient several times per face per outer iteration.
  std::unordered_map<dof_id_type, Real> _face_normal_density_gradient;
  /// Whether _face_normal_density_gradient matches the latest volume fraction
  bool _face_normal_density_gradient_valid = false;
  /// The explicit predictor force density per cell, reconstructed once when the first momentum
  /// component is assembled and read by the other components of the same assembly.
  mutable std::unordered_map<dof_id_type, RealVectorValue> _predictor_force_density;
  /// Whether _phiHbyA_flux/_phig_flux match the latest predictor state.
  bool _pressure_predictor_face_state_valid = false;

  /// The corrected transport flux, per unit area, computed from the predictor and pressure
  /// fluxes on every read so it can never be stale
  DerivedFaceFunctor<Real> _corrected_face_phi;
  /// The corrected face flux of the last accepted step. Restartable, so a recovered run continues
  /// from the history of the step the checkpoint was written on rather than re-deriving it.
  FaceScalarField & _previous_timestep_corrected_face_phi;
  /// The corrected face flux of the step before that, which a two-step time integrator weighs
  /// into the transient projection. Restartable with the previous one.
  FaceScalarField & _older_timestep_corrected_face_phi;
  /// Whether the history has ever been written. Restartable with it.
  bool & _transport_history_seeded;
  /// The boundary-normal pressure gradient and face 1/A of the last accepted step, which the first
  /// predictor of the next step reads for its boundary pressure force: the face 1/A turns the
  /// pressure flux of a fixed-value boundary back into a gradient. Without them that predictor
  /// falls back on the boundary conditions' own normal gradients, which differ from the ones the
  /// pressure equation imposed, and a converged step is knocked out of balance at every step
  /// start. Restartable with the flux history.
  std::unordered_map<dof_id_type, Real> & _accepted_pressure_boundary_normal_gradient;
  /// The face 1/A paired with _accepted_pressure_boundary_normal_gradient
  std::unordered_map<dof_id_type, RealVectorValue> & _accepted_pressure_Ainv;

  /// The corrected flux frozen for one outer iteration, read by the volume-fraction transport
  FaceScalarField _lagged_corrected_face_phi;

  const RealVectorValue _gravity;
  /// x_ref of the reduced pressure p_rgh = p - rho g.(x - x_ref)
  const Point _reference_pressure_point;
  /// The density in the gravity force
  const Moose::Functor<Real> & _buoyancy_rho;

  /// Name of the face mass flux the volume-fraction corrector publishes
  const MooseFunctorName _vof_rho_phi_name;
  /// Name of the capillary force density functor, empty without surface tension
  const MooseFunctorName _surface_tension_force_name;

  /// The face mass flux named by _vof_rho_phi_name, looked up in initialSetup()
  const Moose::Functor<Real> * _vof_rho_phi;
  /// The capillary force density, null without surface tension
  const Moose::Functor<RealVectorValue> * _surface_tension_force = nullptr;
  /// See setStartupProjectionActive
  bool _startup_projection_active = false;

  /// Predictor operators for each momentum component.
  std::vector<std::unique_ptr<MomentumPredictorOperator>> _momentum_predictor_operators;

  /// Whether every component operator has been assembled and finalized since the last begin
  bool _predictor_ready = false;
};
