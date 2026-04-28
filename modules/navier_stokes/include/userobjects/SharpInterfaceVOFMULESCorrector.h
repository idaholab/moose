#pragma once

#include "GeneralUserObject.h"
#include "NonADFunctorInterface.h"
#include "BlockRestrictable.h"
#include "RhieChowMassFlux.h"
#include "FaceCenteredMapFunctor.h"
#include "GradientLimiterType.h"

#include <unordered_map>

class LinearSystem;
/**
 * Applies an explicit bounded correction to a donor/upwind alpha solve, following the same
 * bounded-flux-plus-limited-correction structure used by interFoam's MULES path.
 */
class SharpInterfaceVOFMULESCorrector : public GeneralUserObject,
                                        public NonADFunctorInterface,
                                        public BlockRestrictable
{
public:
  static InputParameters validParams();

  SharpInterfaceVOFMULESCorrector(const InputParameters & params);

  void initialSetup() override;
  void initialize() override {}
  void execute() override {}
  void finalize() override {}

  void resetSubcycleFluxes();
  void applyCorrection(const Real dt, const Real subcycle_fraction = 1.0);

  const SolverSystemName & systemName() const { return _system_name; }
  const VariableName & variableName() const { return _variable_name; }

private:
  struct FaceCorrectionData
  {
    const FaceInfo * face = nullptr;
    dof_id_type elem_dof = DofObject::invalid_id;
    dof_id_type neighbor_dof = DofObject::invalid_id;
    Real donor_flux = 0.0;
    Real high_order_flux = 0.0;
    Real compressive_flux = 0.0;
    Real advective_correction_flux = 0.0;
    Real correction_flux = 0.0;
    bool has_neighbor = false;
  };

  void cacheSystemData();
  void clampSolution() const;
  void publishFaceFluxes(const std::vector<FaceCorrectionData> & face_corrections,
                         const std::vector<Real> & raw_correction_fluxes,
                         const std::vector<Real> & limited_correction_fluxes,
                         const Real subcycle_fraction);
  void dumpFaceDebug(const std::vector<FaceCorrectionData> & face_corrections,
                     const std::vector<Real> & accepted_lambda,
                     const std::unordered_map<dof_id_type, Real> & alpha_before,
                     const std::unordered_map<dof_id_type, Real> & alpha_after,
                     const unsigned int subcycle_index) const;
  Real boundaryValue(const FaceInfo & fi, FaceInfo::VarFaceNeighbors face_type) const;
  Real cellVolume(const ElemInfo & elem_info) const;
  Real faceMeasure(const FaceInfo & fi) const;
  Real cellAlpha(const ElemInfo & elem_info) const;
  Real donorFlux(const FaceInfo & fi) const;
  Real highOrderFlux(const FaceInfo & fi) const;
  Real limitedUpwindFaceValue(const FaceInfo & fi, bool upwind_is_elem) const;
  Real compressionFlux(const FaceInfo & fi, const Real elem_alpha, const Real neighbor_alpha) const;
  Real faceFunctorAverage(const FaceInfo & fi, const Moose::Functor<Real> & functor) const;
  Real rhoPhi(const FaceInfo & fi, const Real limited_alpha_flux) const;
  FaceCorrectionData buildFaceCorrectionData(const FaceInfo & fi) const;

  const SolverSystemName _system_name;
  const VariableName _variable_name;
  const RhieChowMassFlux & _mass_flux_provider;
  const Moose::Functor<Real> & _compression_factor;
  const Moose::Functor<RealVectorValue> & _interface_normal;
  const Moose::Functor<Real> & _liquid_density;
  const Moose::Functor<Real> & _gas_density;
  const unsigned int _num_alpha_corrections;
  const unsigned int _num_limiter_iterations;
  const Real _correction_relaxation;
  const Real _min_value;
  const Real _max_value;
  const bool _debug_dump_subcycle;
  const bool _debug_only_first_subcycle;
  const unsigned int _debug_dump_max_faces;
  const Real _debug_interface_alpha_tolerance;

  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> _alpha_phi_bd;
  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> _alpha_phi_ho;
  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> _alpha_phi_comp;
  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> _alpha_phi_corr_raw;
  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> _alpha_phi_corr;
  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> _alpha_phi_limited;
  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> _rho_phi;

  MooseLinearVariableFVReal * _alpha_var = nullptr;
  LinearSystem * _system = nullptr;
  unsigned int _sys_num = libMesh::invalid_uint;
  unsigned int _var_num = libMesh::invalid_uint;
  mutable unsigned int _subcycle_counter = 0;
};
