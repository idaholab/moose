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

#include <unordered_map>

class LinearSystem;
class LinearFVBoundaryCondition;

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
  void refreshPublishedRhoPhi();
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

  struct AlphaFluxData
  {
    Real donor_flux = 0.0;
    Real correction_flux = 0.0;
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

  void cacheSystemData();
  void initializeFluxStorage();
  void publishFaceFluxes(const std::vector<FaceCorrectionData> & face_corrections,
                         const std::vector<Real> & accumulated_alpha_fluxes,
                         const Real subcycle_fraction);
  bool partitionFace(const FaceCorrectionData & data) const;
  bool locallyOwnedCell(const ElemInfo & elem_info) const;
  bool synchronizePartitionFaceLimiters(const std::vector<FaceCorrectionData> & face_corrections,
                                        std::vector<Real> & accepted_lambda) const;
  FaceTransportData faceTransportData(const FaceInfo & fi) const;
  Real boundaryValue(const FaceInfo & fi, const FaceTransportData & face_data) const;
  bool hasFaceSide(const FaceInfo & fi, bool fi_elem_side) const override;
  Real cellVolume(const ElemInfo & elem_info) const;
  Real faceMeasure(const FaceInfo & fi) const;
  Real cellAlpha(const ElemInfo & elem_info) const;
  Real boundedAlpha(Real value) const;
  Real donorFlux(const FaceInfo & fi, const FaceTransportData & face_data, Real elem_alpha) const;
  Real highOrderFaceValue(const FaceInfo & fi,
                          const FaceTransportData & face_data,
                          Real elem_alpha) const;
  Real sharedVanLeerFaceValue(const FaceInfo & fi, bool upwind_is_elem) const;
  BoundaryFaceKind classifyBoundaryFace(const FaceInfo & fi,
                                        FaceInfo::VarFaceNeighbors face_type,
                                        Real volumetric_flux,
                                        const LinearFVBoundaryCondition * bc) const;
  AlphaFluxData buildAlphaFlux(const FaceInfo & fi,
                               Real elem_alpha,
                               Real neighbor_alpha,
                               const FaceTransportData & face_data) const;
  Real faceFunctorAverage(const FaceInfo & fi, const Moose::Functor<Real> & functor) const;
  Real vofTransportVolumetricFaceFlux(const FaceInfo & fi) const;
  Real integratedVofTransportFaceFlux(const FaceInfo & fi) const;
  Real rhoPhi(const FaceInfo & fi, const Real limited_alpha_flux) const;
  FaceCorrectionData buildFaceCorrectionData(const FaceInfo & fi) const;
  std::vector<FaceCorrectionData> collectFaceCorrectionData() const;

  const SolverSystemName _system_name;
  const VariableName _variable_name;
  const Moose::Functor<Real> & _face_flux;
  const Moose::Functor<Real> & _compression_factor;
  const Moose::Functor<RealVectorValue> & _interface_normal;
  const Moose::Functor<Real> & _liquid_density;
  const Moose::Functor<Real> & _gas_density;
  const unsigned int _num_alpha_corrections;
  const unsigned int _num_limiter_iterations;

  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> _alpha_phi_limited;
  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> _rho_phi;

  MooseLinearVariableFVReal * _alpha_var = nullptr;
  LinearSystem * _system = nullptr;
  unsigned int _sys_num = libMesh::invalid_uint;
  unsigned int _var_num = libMesh::invalid_uint;
};
