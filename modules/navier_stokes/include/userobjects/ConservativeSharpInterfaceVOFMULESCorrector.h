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
#include "GradientLimiterType.h"
#include "FaceArgInterface.h"

#include <unordered_map>

class LinearSystem;
class LinearFVBoundaryCondition;
class ConservativeSharpInterfaceCurvatureCalculator;
/**
 * Applies an explicit bounded correction to a donor/upwind alpha solve, following the same
 * bounded-flux-plus-limited-correction structure used by the sharp-interface VOF path.
 */
class ConservativeSharpInterfaceVOFMULESCorrector : public GeneralUserObject,
                                                    public NonADFunctorInterface,
                                                    public BlockRestrictable,
                                                    public FaceArgProducerInterface
{
public:
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

  struct AlphaFluxData
  {
    Real donor_flux = 0.0;
    Real high_order_flux = 0.0;
    Real compressive_flux = 0.0;
    Real total_flux = 0.0;
    Real correction_flux = 0.0;
    Real interface_normal_alignment = 0.0;
  };

  void cacheSystemData();
  void initializeFluxStorage();
  void invalidatePreviousCorrectionFluxes();
  void publishFaceFluxes(const std::vector<FaceCorrectionData> & face_corrections,
                         const std::vector<Real> & raw_correction_fluxes,
                         const std::vector<Real> & accumulated_alpha_fluxes,
                         const std::vector<Real> & accumulated_correction_fluxes,
                         const Real subcycle_fraction);
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
  Real cellAlpha(const ElemInfo & elem_info) const;
  Real boundedAlpha(Real value) const;
  Real donorFlux(const FaceInfo & fi) const;
  Real highOrderFaceValue(const FaceInfo & fi) const;
  Real venkatakrishnanFaceValue(const FaceInfo & fi, bool upwind_is_elem) const;
  Real sharedVanLeerFaceValue(const FaceInfo & fi, bool upwind_is_elem) const;
  BoundaryFaceKind classifyBoundaryFace(const FaceInfo & fi,
                                        FaceInfo::VarFaceNeighbors face_type,
                                        Real volumetric_flux) const;
  const char * boundaryFaceKindName(BoundaryFaceKind kind) const;
  AlphaFluxData buildAlphaFlux(const FaceInfo & fi,
                               Real elem_alpha,
                               Real neighbor_alpha,
                               BoundaryFaceKind boundary_kind) const;
  Real faceFunctorAverage(const FaceInfo & fi, const Moose::Functor<Real> & functor) const;
  Real vofTransportVolumetricFaceFlux(const FaceInfo & fi) const;
  Real rhoPhi(const FaceInfo & fi, const Real limited_alpha_flux) const;
  FaceCorrectionData buildFaceCorrectionData(const FaceInfo & fi) const;

  const SolverSystemName _system_name;
  const VariableName _variable_name;
  const Moose::Functor<Real> & _face_flux;
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

  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> _alpha_phi_bd;
  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> _alpha_phi_ho;
  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> _alpha_phi_comp;
  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> _alpha_phi_corr_raw;
  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> _alpha_phi_corr;
  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> _alpha_phi_limited;
  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> _rho_phi;
  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> _rho_phi_mass_flux_density;
  std::unordered_map<dof_id_type, Real> _alpha_phi_corr_prev;

  MooseLinearVariableFVReal * _alpha_var = nullptr;
  LinearSystem * _system = nullptr;
  unsigned int _sys_num = libMesh::invalid_uint;
  unsigned int _var_num = libMesh::invalid_uint;
  mutable unsigned int _subcycle_counter = 0;
  bool _previous_correction_flux_valid = false;
};
