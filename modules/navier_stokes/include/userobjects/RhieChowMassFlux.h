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
#include "PetscVectorReader.h"
#include <unordered_map>
#include <set>

#include "libmesh/petsc_vector.h"

class MooseMesh;
class ElemInfo;
class PetscVectorReader;
class INSFVVelocityVariable;
class INSFVPressureVariable;
class LinearFVGradientReader;
class FVReconstructedPressureGradient;
class LinearFVPressureCorrectionDiffusion;
class LinearFVBoundaryCondition;
namespace libMesh
{
class Elem;
class MeshBase;
template <typename T>
class SparseMatrix;
}

/**
 * User object responsible for determining the face fluxes using the Rhie-Chow interpolation in a
 * segregated solver that uses the linear FV formulation.
 */
class RhieChowMassFlux : public RhieChowFaceFluxProvider, public NonADFunctorInterface
{
public:
  static InputParameters validParams();
  RhieChowMassFlux(const InputParameters & params);
  ~RhieChowMassFlux() override;

  /// Get the face velocity times density (used in advection terms)
  virtual Real getMassFlux(const FaceInfo & fi) const;

  /// Get the volumetric face flux (used in advection terms)
  virtual Real getVolumetricFaceFlux(const FaceInfo & fi) const;

  /**
   * Check the block consistency between the passed in \p var and us
   */
  template <typename VarType>
  void checkBlocks(const VarType & var) const;

  /// Get the registered pressure gradient field used by compatible momentum pressure kernels.
  const LinearFVGradientReader & pressureGradientField() const;

  /// Get the base pressure gradient field used to seed reconstructed gradient updates.
  const LinearFVGradientReader & basePressureGradientField() const;

  /// Get the base-pressure gradient component vectors used to seed reconstructed updates.
  const std::vector<NumericVector<Number> *> & basePressureGradientComponents() const;

  /// Get the momentum-layout H/A component vectors.
  const std::vector<std::unique_ptr<NumericVector<Number>>> & HbyAComponents() const;

  /// Get the momentum-layout 1/A component vectors.
  const std::vector<std::unique_ptr<NumericVector<Number>>> & AinvComponents() const;

  /// Variable number of the pressure variable reconstructed by this object.
  unsigned int pressureVariableNumber() const;

  /// Number of spatial dimensions of the coupled momentum/pressure systems.
  unsigned int dimension() const { return _dim; }

  /// Velocity variable for one spatial component.
  MooseLinearVariableFVReal & velocityVariable(unsigned int component) { return *_vel[component]; }

  /// Velocity variable for one spatial component.
  const MooseLinearVariableFVReal & velocityVariable(unsigned int component) const
  {
    return *_vel[component];
  }

  /// Momentum linear system for one spatial component.
  LinearSystem & momentumSystem(unsigned int component) { return *_momentum_systems[component]; }

  /// Momentum linear system for one spatial component.
  const LinearSystem & momentumSystem(unsigned int component) const
  {
    return *_momentum_systems[component];
  }

  /// Pressure linear system.
  const LinearSystem & pressureSystem() const { return *_pressure_system; }

  /**
   * Producer counter for the conservative face mass flux: incremented every time
   * computeFaceMassFlux() runs. Used to verify reconstruction consumes the face flux produced by
   * the current pressure corrector, not one left over from a preceding corrector.
   */
  dof_id_type faceMassFluxGeneration() const { return _face_mass_flux_generation; }

  virtual Real getVolumetricFaceFlux(const Moose::FV::InterpMethod m,
                                     const FaceInfo & fi,
                                     const Moose::StateArg & time,
                                     const THREAD_ID tid,
                                     bool subtract_mesh_velocity) const override;

  /// Initialize the container for face velocities
  virtual void initFaceMassFlux();
  /// Initialize the coupling fields (HbyA and Ainv)
  virtual void initCouplingField();
  /// Cache the pressure-equation face flux from the current solved pressure field.
  void cachePressureEquationFlux();
  /// Update the values of the face velocities in the containers
  virtual void computeFaceMassFlux();
  /**
   * Writes the pressure-corrected cell velocity. This is the seam the corrector goes through, so
   * that a provider which reconstructs that velocity differently is given the chance to; the
   * arithmetic for the ordinary case lives in updateCellVelocity.
   */
  virtual void computeCellVelocity();
  /**
   * The momentum predictor assembly seams. The executioner announces each component's assembly,
   * hands over the assembled, not yet relaxed operator, and finally offers the relaxed right hand
   * side for explicit forcing. A provider that assembles its predictor without the pressure and
   * body forces (see splitMomentumPredictorOperator) keeps the operator here and adds those
   * forces itself; this provider needs none of it.
   */
  virtual void beginMomentumPredictorAssembly(const unsigned int /*system_i*/) {}
  /// Receives the assembled operator of component \p system_i before relaxation
  virtual void momentumPredictorAssembled(const unsigned int /*system_i*/,
                                          libMesh::SparseMatrix<Number> & /*matrix*/,
                                          const NumericVector<Number> & /*rhs*/,
                                          const Real /*relaxation_parameter*/)
  {
  }
  /// Adds the explicit pressure and body forces to the relaxed right hand side
  virtual void addMomentumPredictorExplicitForcing(const unsigned int system_i,
                                                   NumericVector<Number> & rhs) const;
  /**
   * Whether the momentum predictor is assembled without its explicit pressure and body forces.
   * The flow physics then adds no momentum pressure kernel, and the coupling pressure gradient is
   * requested here rather than read off that kernel.
   */
  virtual bool splitMomentumPredictorOperator() const { return false; }

  /**
   * Whether this provider's pressure-correction face fluxes carry the face measure, rather than
   * being flux densities. The pressure equation that corrects the flux must match it, so the
   * flow physics asks here when it builds that equation, and the face area handed to the
   * pressure diffusion kernel derives from the same answer so the two cannot disagree.
   */
  virtual bool pressureCorrectionFluxIsIntegrated() const { return false; }

  /// Whether the registered pressure gradient field is produced by the reconstructed method.
  bool usingReconstructedPressureGradientMethod() const;

  /// Snapshot the fields read by the next momentum predictor.
  void prepareMomentumPredictor();

  /// Capture the lagged velocity gradients computed while assembling the momentum predictor.
  void finalizeMomentumPredictor();

  /// Capture the lagged velocity gradient for another corrector using the current predictor.
  void preparePISOCorrector();

  /**
   * Before pressure relaxation, form the conservative reconstructed candidate from the corrected
   * face flux and update the cell velocity from that unrelaxed candidate. No-op for ordinary
   * pressure-gradient methods.
   */
  void preparePressureRelaxation();

  /**
   * After pressure relaxation, refresh every registered pressure gradient and publish relaxed
   * reconstructed feedback for the next momentum predictor. Ordinary methods also update the cell
   * velocity here, preserving their existing relaxed-pressure behavior.
   */
  void finalizePressureCorrector();

  /// Compute every registered pressure gradient once, before the first momentum predictor of a
  /// SIMPLE iteration sequence has anything to read.
  void initPressureGradient();

  virtual void meshChanged() override;
  virtual void initialize() override;
  virtual void execute() override {}
  virtual void finalize() override {}
  virtual void initialSetup() override;

  /**
   * Prepare reconstructed-gradient state once per attempted time step. Accepted coupling feedback
   * is retained for the next time step and restored when an attempt is retried, while candidate
   * fields and sequencing counters are reset for every attempt.
   */
  virtual void timestepSetup() override;

  /**
   * Update the momentum system-related information
   * @param momentum_systems Pointers to the momentum systems which are solved for the momentum
   * vector components
   * @param pressure_system Reference to the pressure system
   * @param momentum_system_numbers The numbers of these systems
   */
  virtual void
  linkMomentumPressureSystems(const std::vector<LinearSystem *> & momentum_systems,
                              LinearSystem & pressure_system,
                              const std::vector<unsigned int> & momentum_system_numbers);

  /**
   * Computes the inverse of the diagonal (1/A) of the system matrix plus the H/A components for the
   * pressure equation plus Rhie-Chow interpolation.
   */
  virtual void computeHbyA(bool verbose);

protected:
  /// Update cell velocity from the supplied momentum-coupling pressure gradient.
  template <typename GradientComponent>
  void updateCellVelocity(const std::vector<GradientComponent> & pressure_gradient);

  /// Get the registered pressure gradient component vectors.
  const std::vector<NumericVector<Number> *> & pressureGradientComponents() const;

  /// Access the reconstructed gradient method when it is selected.
  FVReconstructedPressureGradient & reconstructedGradientMethod();

  /// Access the reconstructed gradient method when it is selected.
  const FVReconstructedPressureGradient & reconstructedGradientMethod() const;

  /// Check the single-variable system layout assumed by reconstructed pressure-gradient vector ops.
  void checkReconstructedPressureGradientCompatibility() const;

  /// Compute the pressure-gradient flux contribution for a single face
  Real computeFacePressureGradientFlux(const FaceInfo & fi, PetscVectorReader & p_reader) const;

  /// The predictor quantities of one face that the pressure equation's fields are built from
  struct FaceMomentumPredictorState
  {
    /// The interpolated face density, or the boundary density on a boundary face
    Real face_rho = 0.0;
    /// H/A on the face, with the sign it is stored in: u = -(H/A) - Ainv grad(p). A consumer that
    /// wants the physical predictor velocity negates it at the point of use.
    RealVectorValue face_hbya;
    /// face_rho times face_hbya, the mass-flux form the stock pressure equation uses
    RealVectorValue density_weighted_face_hbya;
    /// The face value of rho/A per component, interpolated as a product of the cell values
    RealVectorValue face_density_weighted_ainv;
  };

  /// Whether a boundary face should use constrained boundary values for the predictor state.
  virtual bool useConstrainedBoundaryPredictorState(const FaceInfo & fi) const;
  /// Flow-side element information for a boundary face.
  const ElemInfo & boundaryElemInfo(const FaceInfo & fi) const;
  /// Orientation multiplier from flow-side boundary values to face normal convention.
  Real boundaryNormalMultiplier(const FaceInfo & fi) const;
  /// The boundary face value of a velocity component: the value of its Dirichlet boundary
  /// condition if the face has one, otherwise the variable's own face value.
  virtual Real boundaryVelocityValue(const FaceInfo & fi,
                                     const unsigned int component,
                                     const Moose::StateArg & time_arg) const;

  /// Compute the cell volumes on the mesh
  void setupMeshInformation();

  /// Populate the face values of the H/A and 1/A fields
  void
  populateCouplingFunctors(const std::vector<std::unique_ptr<NumericVector<Number>>> & raw_hbya,
                           const std::vector<std::unique_ptr<NumericVector<Number>>> & raw_Ainv);

  /// Build the face predictor state shared by stock and diffuse-interface pressure coupling.
  FaceMomentumPredictorState
  buildMomentumPredictorFaceState(const FaceInfo & fi,
                                  const std::vector<PetscVectorReader> & hbya_reader,
                                  const std::vector<PetscVectorReader> & ainv_reader,
                                  const Moose::StateArg & time_arg) const;

  /**
   * Form H(u) - b and the diagonal A of one momentum component, in the momentum system's layout.
   * The stock predictor carries the pressure force in its right hand side, so it is removed again
   * here with the coupling gradient snapshot the momentum pressure source used.
   */
  virtual void computePredictorOperator(const unsigned int system_i,
                                        NumericVector<Number> & HbyA,
                                        NumericVector<Number> & Ainv);

  /// Build the base predictor operator vectors M*u - A*u - rhs and the relaxed diagonal.
  void computePredictorOperatorBase(const unsigned int system_i,
                                    NumericVector<Number> & base_raw,
                                    NumericVector<Number> & diagonal_raw,
                                    const NumericVector<Number> * rhs_override = nullptr) const;

  virtual bool supportMeshVelocity() const override { return false; }

  /// The \p MooseMesh that this user object operates on
  const MooseMesh & _moose_mesh;

  /// The \p libMesh mesh that this object acts on
  const libMesh::MeshBase & _mesh;

  /// The dimension of the mesh, e.g. 3 for hexes and tets, 2 for quads and tris
  const unsigned int _dim;

  /// The thread 0 copy of the pressure variable
  const MooseLinearVariableFVReal * const _p;

  /// Thread 0 copies of the velocity variables; non-const to allow writing
  /// cell velocities in updateCellVelocity().
  std::vector<MooseLinearVariableFVReal *> _vel;

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

  /// Coupling pressure gradient captured before the current momentum predictor is assembled.
  std::vector<std::unique_ptr<NumericVector<Number>>> _grad_p_current;
  /**
   * Exact face flux contribution from the solved pressure equation, in the same sign convention
   * used in face_mass_flux = -predictor_flux + pressure_equation_flux.
   */
  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> _pressure_equation_flux;

  /// Indicates whether _pressure_equation_flux matches the latest solved pressure field.
  bool _pressure_equation_flux_valid = false;

  /**
   * Producer counter for the conservative face mass flux: incremented every time
   * computeFaceMassFlux() runs. Exposed read-only through faceMassFluxGeneration() so the
   * reconstructed gradient can verify it is reconstructing from the face flux produced by the
   * current pressure corrector, not one left over from a preceding corrector.
   */
  dof_id_type _face_mass_flux_generation = 0;

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
  LinearSystem * _pressure_system;

  /// Registered pressure gradient field used by Rhie-Chow and compatible momentum pressure kernels.
  const LinearFVGradientReader * _pressure_gradient_field;

  /// Base pressure gradient field used to seed reconstructed gradient updates.
  const LinearFVGradientReader * _base_pressure_gradient_field;

  /// Global number of the pressure system
  unsigned int _global_pressure_system_number;

  /**
   * Fluid volume of each cell, used to rapidly scale the pressure-gradient force in H/A and 1/A.
   * This is the cell volume, or the cell volume times porosity for porous flow.
   */
  std::unique_ptr<NumericVector<Number>> _cell_fluid_volumes;

  /// Enumerator for the method used for pressure projection
  const MooseEnum _pressure_projection_method;

  /// Interpolation method used for the pressure diffusion coefficient on faces
  const Moose::FV::InterpMethod _pressure_diffusion_interp_method;

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
