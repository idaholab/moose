#pragma once

#include "GeneralUserObject.h"
#include "NonADFunctorInterface.h"
#include "BlockRestrictable.h"
#include "RhieChowMassFlux.h"
#include "FaceCenteredMapFunctor.h"
#include "GradientLimiterType.h"
#include "FaceArgInterface.h"

#include <unordered_map>
#include <unordered_set>

class LinearSystem;
class LinearFVBoundaryCondition;
class ConservativeSharpInterfaceCurvatureCalculator;
/**
 * Applies an explicit bounded correction to a donor/upwind alpha solve, following the same
 * bounded-flux-plus-limited-correction structure used by interFoam's MULES path.
 */
class ConservativeSharpInterfaceVOFMULESCorrector : public GeneralUserObject,
                                        public NonADFunctorInterface,
                                        public BlockRestrictable,
                                        public FaceArgProducerInterface
{
public:
  struct LiquidVolumeAudit
  {
    Real current = 0.0;
    Real timestep_old = 0.0;
    Real previous_outer = 0.0;
    bool has_previous_outer = false;
  };

  struct RhoPhiConsistencyAudit
  {
    Real l2_norm = 0.0;
    Real max_abs_mismatch = 0.0;
    bool has_worst_face = false;
    dof_id_type worst_face_id = DofObject::invalid_id;
    Point worst_face_centroid;
    Real stored_rho_phi = 0.0;
    Real reconstructed_rho_phi = 0.0;
    Real volumetric_phi = 0.0;
    Real limited_alpha_flux = 0.0;
    Real gas_density = 0.0;
    Real liquid_density = 0.0;
  };

  static InputParameters validParams();

  ConservativeSharpInterfaceVOFMULESCorrector(const InputParameters & params);

  void initialSetup() override;
  void meshChanged() override;
  void initialize() override {}
  void execute() override {}
  void finalize() override {}

  void resetSubcycleFluxes();
  void invalidateOuterCorrectionFluxSeed();
  void refreshPublishedRhoPhi();
  void applyCorrection(const Real dt,
                       const Real subcycle_fraction = 1.0,
                       ConservativeSharpInterfaceCurvatureCalculator * curvature = nullptr);
  RhoPhiConsistencyAudit rhoPhiConsistencyAudit() const;
  Real alphaPhiWorkingBeforeIntegrated(const FaceInfo & fi) const;
  Real alphaPhiTargetIntegrated(const FaceInfo & fi) const;
  Real alphaPhiRawCorrectionIntegrated(const FaceInfo & fi) const;
  Real alphaPhiLimitedDeltaIntegrated(const FaceInfo & fi) const;
  Real alphaPhiAcceptedLambda(const FaceInfo & fi) const;

  const SolverSystemName & systemName() const { return _system_name; }
  const VariableName & variableName() const { return _variable_name; }

private:
  enum class HighOrderCorrectionScheme : unsigned char
  {
    Venkatakrishnan,
    VanLeer
  };

  enum class BoundaryFaceKind : unsigned char
  {
    Internal,
    DirichletInflow,
    DirichletOutflow,
    OpenOutflow,
    Closed
  };

  struct FaceCorrectionData
  {
    const FaceInfo * face = nullptr;
    dof_id_type elem_dof = DofObject::invalid_id;
    dof_id_type neighbor_dof = DofObject::invalid_id;
    Real volumetric_flux = 0.0;
    Real elem_alpha = 0.0;
    Real neighbor_alpha = 0.0;
    Real interface_normal_alignment = 0.0;
    Real donor_flux = 0.0;
    Real high_order_flux = 0.0;
    Real compressive_flux = 0.0;
    Real advective_correction_flux = 0.0;
    Real correction_flux = 0.0;
    bool has_neighbor = false;
    bool boundary_face = false;
    BoundaryFaceKind boundary_kind = BoundaryFaceKind::Internal;
  };

  void cacheSystemData();
  void initializeFluxStorage();
  void invalidatePreviousCorrectionFluxes();
  void publishFaceFluxes(const std::vector<FaceCorrectionData> & face_corrections,
                         const std::vector<Real> & raw_correction_fluxes,
                         const std::vector<Real> & accumulated_alpha_fluxes,
                         const std::vector<Real> & accumulated_correction_fluxes,
                         const Real subcycle_fraction);
  void dumpCandidateFaceDebug(const std::vector<FaceCorrectionData> & face_corrections,
                              const std::vector<Real> & raw_correction_fluxes,
                              const std::vector<Real> & accepted_lambda,
                              const unsigned int subcycle_index) const;
  void dumpFaceDebug(const std::vector<FaceCorrectionData> & face_corrections,
                     const std::vector<Real> & raw_correction_fluxes,
                     const std::vector<Real> & accepted_lambda,
                     const std::unordered_map<dof_id_type, Real> & alpha_before,
                     const std::unordered_map<dof_id_type, Real> & alpha_after,
                     const unsigned int subcycle_index) const;
  bool partitionFace(const FaceCorrectionData & data) const;
  bool locallyOwnedCell(const ElemInfo & elem_info) const;
  bool synchronizePartitionFaceLimiters(const std::vector<FaceCorrectionData> & face_corrections,
                                        std::vector<Real> & accepted_lambda) const;
  LinearFVBoundaryCondition * boundaryCondition(const FaceInfo & fi) const;
  Real boundaryValue(const FaceInfo & fi, FaceInfo::VarFaceNeighbors face_type) const;
  bool hasFaceSide(const FaceInfo & fi, bool fi_elem_side) const override;
  Moose::FaceArg functorFaceArg(const Moose::Functor<Real> & functor, const FaceInfo & fi) const;
  Real cellVolume(const ElemInfo & elem_info) const;
  Real faceMeasure(const FaceInfo & fi) const;
  Real solutionAlpha(const NumericVector<Number> & solution, const ElemInfo & elem_info) const;
  Real integrateLiquidVolume(const NumericVector<Number> & solution) const;
  Real cellAlpha(const ElemInfo & elem_info) const;
  Real boundedAlpha(Real value) const;
  Real donorFlux(const FaceInfo & fi) const;
  Real highOrderFaceValue(const FaceInfo & fi) const;
  Real highOrderFlux(const FaceInfo & fi) const;
  Real venkatakrishnanFaceValue(const FaceInfo & fi, bool upwind_is_elem) const;
  Real sharedVanLeerFaceValue(const FaceInfo & fi, bool upwind_is_elem) const;
  BoundaryFaceKind classifyBoundaryFace(const FaceInfo & fi,
                                        FaceInfo::VarFaceNeighbors face_type,
                                        Real volumetric_flux) const;
  const char * boundaryFaceKindName(BoundaryFaceKind kind) const;
  Real compressionFlux(const FaceInfo & fi, const Real elem_alpha, const Real neighbor_alpha) const;
  Real compressionAlignment(const FaceInfo & fi) const;
  Real faceFunctorAverage(const FaceInfo & fi, const Moose::Functor<Real> & functor) const;
  Real rhoPhi(const FaceInfo & fi, const Real limited_alpha_flux) const;
  FaceCorrectionData buildFaceCorrectionData(const FaceInfo & fi) const;
  bool shouldDebugFace(const FaceCorrectionData & data) const;

  const SolverSystemName _system_name;
  const VariableName _variable_name;
  const RhieChowMassFlux & _mass_flux_provider;
  const Moose::Functor<Real> & _compression_factor;
  const Moose::Functor<RealVectorValue> & _interface_normal;
  const Moose::Functor<Real> & _liquid_density;
  const Moose::Functor<Real> & _gas_density;
  const HighOrderCorrectionScheme _high_order_correction_scheme;
  const unsigned int _num_alpha_corrections;
  const unsigned int _num_limiter_iterations;
  const Real _correction_relaxation;
  const Real _later_correction_relaxation;
  const Real _min_value;
  const Real _max_value;
  const bool _alpha_apply_prev_corr;
  const bool _use_cell_summed_mules_limiter;
  const bool _use_local_mules_bounds;
  const bool _debug_dump_subcycle;
  const bool _debug_only_first_subcycle;
  const unsigned int _debug_dump_max_faces;
  const Real _debug_interface_alpha_tolerance;
  const std::unordered_set<dof_id_type> _debug_face_ids;

  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> _alpha_phi_bd;
  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> _alpha_phi_ho;
  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> _alpha_phi_comp;
  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> _alpha_phi_corr_raw;
  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> _alpha_phi_corr;
  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> _alpha_phi_limited;
  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> _rho_phi;
  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> _rho_phi_mass_flux_density;
  std::unordered_map<dof_id_type, Real> _alpha_phi_working_before_debug;
  std::unordered_map<dof_id_type, Real> _alpha_phi_target_debug;
  std::unordered_map<dof_id_type, Real> _alpha_phi_limited_delta_debug;
  std::unordered_map<dof_id_type, Real> _alpha_phi_lambda_debug;
  std::unordered_map<dof_id_type, Real> _alpha_phi_corr_prev;

  MooseLinearVariableFVReal * _alpha_var = nullptr;
  LinearSystem * _system = nullptr;
  unsigned int _sys_num = libMesh::invalid_uint;
  unsigned int _var_num = libMesh::invalid_uint;
  mutable unsigned int _subcycle_counter = 0;
  bool _previous_correction_flux_valid = false;
};
