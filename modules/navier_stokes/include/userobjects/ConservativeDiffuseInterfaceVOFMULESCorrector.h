//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"
#include "NonADFunctorInterface.h"
#include "BlockRestrictable.h"
#include "FaceCenteredMapFunctor.h"
#include "FaceArgInterface.h"
#include "MooseLinearVariableFV.h"

#include <unordered_map>
#include <vector>

class LinearSystem;
class LinearFVBoundaryCondition;

/**
 * Bounded volume-fraction transport by a semi-implicit MULES scheme (multidimensional universal
 * limiter for explicit solution).
 *
 * The executioner first solves the volume-fraction equation implicitly with donor-cell (upwind)
 * face values and the face flux this object is given. That solution is bounded but diffusive.
 * applyCorrection() then adds an explicit antidiffusive correction on every face: the difference
 * between a target flux and the donor flux, multiplied by a face limiter lambda in [0, 1]. The
 * target face value is the Van Leer limited interpolation of alpha plus the interface compression
 * term
 *   c_alpha sign(phi) alpha_f (1 - alpha_f) (n_interface . n_face),
 * which pushes alpha toward 0 and 1 across the interface and holds it a few cells wide. The
 * limiter is Zalesak's flux-corrected-transport limiter in its multidimensional form: each
 * cell may not leave the range spanned by its neighbours' values, the admissible increase and
 * decrease of every cell are turned into the fraction of its incoming and outgoing corrections it
 * can accept, and every face takes the smaller fraction of its two cells. A few passes tighten
 * the cell fractions against the fluxes the previous pass accepted. The limited correction is
 * applied to alpha directly, so after the sweep alpha is the bounded transported field and the
 * limited face flux is exactly the one that produced it.
 *
 * The object publishes two face functors formed from that flux: alpha_phi_limited, the
 * volume-fraction flux, and rho_phi = rho_g phi + (rho_l - rho_g) alpha_phi_limited, the mixture
 * mass flux consistent with it. The momentum equation advects with rho_phi, so the mass the
 * momentum equation moves is the mass the volume fraction moved, which is what keeps the velocity
 * bounded across the density jump. When the executioner subcycles the volume fraction, both
 * fluxes are accumulated over the subcycles weighted by the subcycle fraction, so they are the
 * time-step averages the momentum equation needs.
 *
 * All face fluxes inside this object are integrated over the face (volume per time); the flux
 * functor it reads is per unit area and is multiplied by the face measure on the way in.
 *
 * Temporal accuracy. The donor solve carries the order of the time integrator, and the explicit
 * correction advances alpha by dt over the integrator's leading weight (dt for implicit Euler,
 * 2 dt / 3 for BDF2 at constant step), so the corrected alpha satisfies the same discrete time
 * derivative the donor solve used. The scheme is nevertheless first order in time on a fixed
 * mesh, whatever the integrator: the correction is evaluated at the donor solution, which
 * differs from the corrected one by the correction itself, an O(dt) quantity, and the correction
 * sweeps re-evaluate only the antidiffusive flux, never the implicit upwind flux or an implicit
 * source. That splitting error is of order dt h, so refining dt with h gives second order (the
 * tanh-interface manufactured solution under test/tests/finite_volume/two_phase/
 * diffuse_interface/mms shows 1.9 to 2.1 in every field with BDF2), while refining dt alone at
 * fixed h shows order 1 in alpha, with a coefficient that extra sweeps halve but do not remove.
 * This is a property of the semi-implicit MULES splitting. Second order in time
 * alone would need the donor system re-solved in every sweep with the accumulated limited
 * antidiffusive flux as a deferred-correction source, which changes the boundedness argument.
 */
class ConservativeDiffuseInterfaceVOFMULESCorrector : public GeneralUserObject,
                                                      public NonADFunctorInterface,
                                                      public BlockRestrictable,
                                                      public FaceArgProducerInterface
{
public:
  static InputParameters validParams();

  ConservativeDiffuseInterfaceVOFMULESCorrector(const InputParameters & params);

  void initialSetup() override;
  void meshChanged() override;
  void initialize() override {}
  void execute() override {}
  void finalize() override {}

  /// Zero the accumulated fluxes at the start of an outer iteration's first subcycle
  void resetSubcycleFluxes();
  /**
   * Keep the transported fluxes of the step that was just accepted. The momentum equation reads
   * them at the start of the next step, so an attempt at a step that is solved again must read
   * the fluxes of the last accepted step, not those of the attempt that was rejected.
   */
  void commitAcceptedTimestepFluxes();
  /// Start every attempt at a step from the accepted fluxes.
  void timestepSetup() override;
  /// Recompute rho_phi from the accumulated alpha flux and the flux functor at the current
  /// (global) time, once the executioner has restored the time after subcycling
  void refreshPublishedRhoPhi();
  /**
   * One bounded correction of the donor solution the executioner has just produced. \p dt is
   * the advance of alpha per unit face flux: the time step of the current subcycle divided by
   * the leading weight of the time integrator the donor solve used. \p subcycle_fraction is the
   * subcycle's share of the full step, the weight with which the resulting fluxes enter the
   * published time-step averages.
   */
  void applyCorrection(const Real dt, const Real subcycle_fraction = 1.0);

  const SolverSystemName & systemName() const { return _system_name; }
  const VariableName & variableName() const { return _variable_name; }

private:
  /// How a face takes part in the transport and its correction
  enum class BoundaryFaceKind : unsigned char
  {
    /// A face between two cells of the transported variable
    Internal,
    /// A boundary face with a prescribed value and inflow: the prescribed value is advected in
    DirichletInflow,
    /// A boundary face with a prescribed value and outflow: the prescribed value leaves, so the
    /// face carries the full high-order flux and counts toward the cell's bounds
    DirichletOutflow,
    /// An outflow face without a prescribed value: the cell value leaves and is not corrected
    OpenOutflow,
    /// No flux through the face: walls, closed boundaries and faces below the flux floor
    Closed
  };

  /// Everything the limiter needs about one face, gathered once per correction sweep
  struct FaceCorrectionData
  {
    /// The face; null marks a face the transport does not see
    const FaceInfo * face = nullptr;
    dof_id_type elem_dof = DofObject::invalid_id;
    dof_id_type neighbor_dof = DofObject::invalid_id;
    Real elem_alpha = 0.0;
    /// The neighbour's value, or the boundary value on a boundary face
    Real neighbor_alpha = 0.0;
    /// The integrated donor-cell flux of alpha, what the implicit solve transported
    Real donor_flux = 0.0;
    /// Target flux minus donor flux: the antidiffusive correction before limiting
    Real correction_flux = 0.0;
    bool has_neighbor = false;
    /// Whether this process owns the element-side cell
    bool elem_owned = false;
    /// Whether this process owns the neighbor-side cell (false without a neighbor)
    bool neighbor_owned = false;
    BoundaryFaceKind boundary_kind = BoundaryFaceKind::Internal;
  };

  /// The flux through one face and which side it comes from
  struct FaceTransportData
  {
    /// Which sides of the face carry the variable; NEITHER leaves the other members unset
    FaceInfo::VarFaceNeighbors face_type = FaceInfo::VarFaceNeighbors::NEITHER;
    /// The variable's boundary condition on the face, null on internal faces and where none is set
    LinearFVBoundaryCondition * boundary_condition = nullptr;
    /// Per unit area, in the direction of the face normal
    Real volumetric_flux = 0.0;
    /// Times the face measure
    Real integrated_flux = 0.0;
    /// Whether the flux leaves the element side, so the element is the donor
    bool upwind_is_elem = true;
    BoundaryFaceKind boundary_kind = BoundaryFaceKind::Closed;
  };

  /// Look up the system and variable; they do not exist yet when the object is constructed
  void cacheSystemData();
  /// Reject a two-term expansion in the volume fraction's boundary conditions, which would enter
  /// the upwind solve unbounded; outflow_two_term_expansion applies it as a limited correction
  void checkBoundaryConditions() const;
  /// Key the published flux functors over every face, so they are total before the first sweep
  void initializeFluxStorage();
  /// Accumulate the limited alpha flux and the matching rho_phi into the published functors
  void publishFaceFluxes(const std::vector<FaceCorrectionData> & face_corrections,
                         const std::vector<Real> & accumulated_alpha_fluxes,
                         const Real subcycle_fraction);
  bool locallyOwnedCell(const ElemInfo & elem_info) const;
  /**
   * Copy the cell limiters of the owned cell on each partition face to the process that ghosts
   * it. A process sees every face of a cell it owns but only the partition faces of a ghost cell,
   * so the limiters are computed by owners only and the ghost entries are filled here.
   */
  void exchangePartitionCellLimiters(const std::vector<FaceCorrectionData> & face_corrections,
                                     std::unordered_map<dof_id_type, Real> & lambda_plus,
                                     std::unordered_map<dof_id_type, Real> & lambda_minus) const;
  /// Which sides of a face carry the variable on this object's blocks; NEITHER excludes the face
  FaceInfo::VarFaceNeighbors transportFaceType(const FaceInfo & fi) const;
  /// The flux through a face, its upwind side and its boundary classification
  FaceTransportData faceTransportData(const FaceInfo & fi) const;
  /// The largest face flux of the variable, area-integrated, over the whole mesh
  Real fluxScale() const;
  /// The boundary condition's value clamped to [0, 1], or the cell value without a condition
  Real boundaryValue(const FaceInfo & fi, const FaceTransportData & face_data) const;
  bool hasFaceSide(const FaceInfo & fi, bool fi_elem_side) const override;
  /// The integrated flux of the upwind value: the cell's, the neighbour's or the boundary's
  Real donorFlux(const FaceInfo & fi, const FaceTransportData & face_data, Real elem_alpha) const;
  /// Classify a face by its boundary condition type and the sign of its flux
  BoundaryFaceKind classifyBoundaryFace(const FaceInfo & fi,
                                        FaceInfo::VarFaceNeighbors face_type,
                                        Real volumetric_flux,
                                        const LinearFVBoundaryCondition * bc) const;
  /// The donor flux and the target-minus-donor correction of one face, compression included
  FaceCorrectionData buildFaceCorrectionData(const FaceInfo & fi) const;
  /// buildFaceCorrectionData() of every face the transport sees, in mesh face order
  std::vector<FaceCorrectionData> collectFaceCorrectionData() const;

  const SolverSystemName _system_name;
  const VariableName _variable_name;
  /// The volumetric face flux per unit area the volume fraction is transported with
  const Moose::Functor<Real> & _face_flux;
  /// c_alpha of the interface compression term; zero switches the compression off
  const Moose::Functor<Real> & _compression_factor;
  /// A face flux or flux correction at or below this counts as zero; set from the largest face
  /// flux at the start of each correction
  Real _flux_floor = 0.0;
  /// The interface unit normal grad(alpha) / |grad(alpha)|, evaluated on faces
  const Moose::Functor<RealVectorValue> & _interface_normal;
  const Moose::Functor<Real> & _liquid_density;
  const Moose::Functor<Real> & _gas_density;
  /// Number of correction sweeps per subcycle; zero publishes the donor flux unchanged
  const unsigned int _num_alpha_corrections;
  /// The limiter of each face from the previous outer iteration of this step, per subcycle. The
  /// limiter may only tighten across the outer loop: left free, a face can alternate between two
  /// limiter states from one outer iteration to the next and the step never converges.
  std::vector<std::unordered_map<dof_id_type, Real>> _outer_iteration_lambda_bound;
  /// Index of the subcycle being corrected since the last resetSubcycleFluxes()
  unsigned int _subcycle_index = 0;
  /// Maximum number of limiter tightening passes per sweep; the passes stop early once no face
  /// limiter changes
  const unsigned int _num_limiter_iterations;
  /// Whether an open outflow face is corrected from the cell value to the linear extrapolation
  /// alpha_P + grad(alpha).d
  const bool _outflow_two_term_expansion;
  /// Whether checkBoundaryConditions has run
  bool _boundary_conditions_checked = false;

  /// The limited volume-fraction face flux of the step, integrated over the face and averaged
  /// over the subcycles, published as alpha_phi_limited
  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> _alpha_phi_limited;
  /// The mixture mass flux rho_g phi + (rho_l - rho_g) alpha_phi_limited, published as rho_phi
  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> _rho_phi;

  /// Transported fluxes of the last accepted step. Restartable, so a recovered run continues
  /// from the step the checkpoint was written on.
  std::unordered_map<dof_id_type, Real> & _accepted_alpha_phi_limited;
  std::unordered_map<dof_id_type, Real> & _accepted_rho_phi;
  /// Whether a step has been accepted yet.
  bool & _accepted_fluxes_valid;

  /// The transported variable and its system, cached by cacheSystemData()
  MooseLinearVariableFVReal * _alpha_var = nullptr;
  LinearSystem * _system = nullptr;
  unsigned int _sys_num = libMesh::invalid_uint;
  unsigned int _var_num = libMesh::invalid_uint;
};
