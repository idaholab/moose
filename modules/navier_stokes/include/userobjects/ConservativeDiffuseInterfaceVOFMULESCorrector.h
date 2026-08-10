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

#include <memory>
#include <unordered_map>
#include <vector>

class LinearSystem;
class SystemBase;
class LinearFVBoundaryCondition;

/**
 * Applies an explicit bounded correction to a donor/upwind alpha solve, following the same
 * bounded-flux-plus-limited-correction structure used by the diffuse-interface VOF path.
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

  void resetSubcycleFluxes();
  void refreshPublishedRhoPhi();
  void cachePreSubcycleAlpha();
  void applyCorrection(const Real dt, const Real subcycle_fraction = 1.0);

  const SolverSystemName & systemName() const { return _system_name; }
  const VariableName & variableName() const { return _variable_name; }

private:
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
    Real elem_alpha = 0.0;
    Real neighbor_alpha = 0.0;
    Real donor_flux = 0.0;
    Real correction_flux = 0.0;
    bool has_neighbor = false;
    BoundaryFaceKind boundary_kind = BoundaryFaceKind::Internal;
  };

  struct FaceTransportData
  {
    FaceInfo::VarFaceNeighbors face_type = FaceInfo::VarFaceNeighbors::NEITHER;
    LinearFVBoundaryCondition * boundary_condition = nullptr;
    Real volumetric_flux = 0.0;
    Real integrated_flux = 0.0;
    bool upwind_is_elem = true;
    BoundaryFaceKind boundary_kind = BoundaryFaceKind::Closed;
  };

  struct ConfinedScalarData
  {
    VariableName variable_name;
    MooseLinearVariableFVReal * variable = nullptr;
    LinearSystem * system = nullptr;
    unsigned int sys_num = libMesh::invalid_uint;
    unsigned int var_num = libMesh::invalid_uint;
  };

  void cacheSystemData();
  void initializeFluxStorage();
  void publishFaceFluxes(const std::vector<FaceCorrectionData> & face_corrections,
                         const std::vector<Real> & accumulated_alpha_fluxes,
                         const Real subcycle_fraction);
  bool partitionFace(const FaceCorrectionData & data) const;
  bool locallyOwnedCell(const ElemInfo & elem_info) const;
  bool synchronizePartitionFaceLimiters(const std::vector<FaceCorrectionData> & face_corrections,
                                        std::vector<Real> & accepted_lambda) const;
  FaceInfo::VarFaceNeighbors transportFaceType(const FaceInfo & fi) const;
  FaceTransportData faceTransportData(const FaceInfo & fi) const;
  Real boundaryValue(const FaceInfo & fi, const FaceTransportData & face_data) const;
  bool hasFaceSide(const FaceInfo & fi, bool fi_elem_side) const override;
  Real cellVolume(const ElemInfo & elem_info) const;
  Real faceMeasure(const FaceInfo & fi) const;
  Real cellAlpha(const ElemInfo & elem_info) const;
  Real oldCellAlpha(const ElemInfo & elem_info) const;
  Real boundedAlpha(Real value) const;
  Real cellRhoCp(const ElemInfo & elem_info, Real alpha) const;
  Real thermalEnergyTemperature(const ElemInfo & elem_info) const;
  Real donorFlux(const FaceInfo & fi, const FaceTransportData & face_data, Real elem_alpha) const;
  Real highOrderFaceValue(const FaceInfo & fi,
                          const FaceTransportData & face_data,
                          Real elem_alpha) const;
  Real sharedVanLeerFaceValue(const FaceInfo & fi, bool upwind_is_elem) const;
  BoundaryFaceKind classifyBoundaryFace(const FaceInfo & fi,
                                        FaceInfo::VarFaceNeighbors face_type,
                                        Real volumetric_flux,
                                        const LinearFVBoundaryCondition * bc) const;
  Real rhoPhi(const FaceInfo & fi, const Real limited_alpha_flux) const;
  Real rhoCpPhi(const FaceInfo & fi, const Real limited_alpha_flux) const;
  FaceCorrectionData buildFaceCorrectionData(const FaceInfo & fi) const;
  std::vector<FaceCorrectionData> collectFaceCorrectionData() const;
  Real confinedScalarConcentration(const ConfinedScalarData & scalar,
                                   const ElemInfo & elem_info) const;
  void applyConfinedScalarTransport(const std::vector<FaceCorrectionData> & face_corrections,
                                    const std::vector<Real> & limited_alpha_fluxes,
                                    Real dt);
  void applyThermalEnergyTransport(const std::vector<FaceCorrectionData> & face_corrections,
                                   const std::vector<Real> & limited_alpha_fluxes,
                                   Real dt);

  const SolverSystemName _system_name;
  const VariableName _variable_name;
  const std::vector<VariableName> _confined_scalar_variable_names;
  const VariableName _thermal_energy_variable_name;
  const Moose::Functor<Real> & _face_flux;
  const Moose::Functor<Real> & _compression_factor;
  const Moose::Functor<RealVectorValue> & _interface_normal;
  const Moose::Functor<Real> & _liquid_density;
  const Moose::Functor<Real> & _gas_density;
  const Moose::Functor<Real> * const _liquid_specific_heat;
  const Moose::Functor<Real> * const _gas_specific_heat;
  const Moose::Functor<Real> * const _thermal_energy_temperature;
  const Moose::Functor<Real> & _confined_scalar_backflow_concentration;
  const Moose::Functor<Real> & _thermal_energy_backflow_temperature;
  const Real _confined_scalar_alpha_floor;
  const Real _confined_scalar_concentration_min;
  const Real _confined_scalar_concentration_max;
  const unsigned int _num_alpha_corrections;
  const unsigned int _num_limiter_iterations;

  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> _alpha_phi_limited;
  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> _rho_phi;
  std::unique_ptr<FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>>> _rho_cp_phi;

  MooseLinearVariableFVReal * _alpha_var = nullptr;
  LinearSystem * _system = nullptr;
  unsigned int _sys_num = libMesh::invalid_uint;
  unsigned int _var_num = libMesh::invalid_uint;
  std::vector<ConfinedScalarData> _confined_scalars;
  std::unordered_map<dof_id_type, Real> _pre_subcycle_alpha_by_dof;
  std::unordered_map<dof_id_type, Real> _pre_subcycle_temperature_by_dof;
  MooseLinearVariableFVReal * _thermal_energy_var = nullptr;
  LinearSystem * _thermal_energy_system = nullptr;
  unsigned int _thermal_energy_sys_num = libMesh::invalid_uint;
  unsigned int _thermal_energy_var_num = libMesh::invalid_uint;
  MooseLinearVariableFVReal * _temperature_var = nullptr;
  SystemBase * _temperature_system = nullptr;
  unsigned int _temperature_sys_num = libMesh::invalid_uint;
  unsigned int _temperature_var_num = libMesh::invalid_uint;
};
