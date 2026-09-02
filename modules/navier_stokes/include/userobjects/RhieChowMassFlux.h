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
#include <unordered_map>
#include <set>

#include "libmesh/petsc_vector.h"

class MooseMesh;
class INSFVVelocityVariable;
class INSFVPressureVariable;
class LinearFVGradientReader;
class FVReconstructedPressureGradient;
class LinearFVAnisotropicDiffusion;
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
  static InputParameters validParams();
  RhieChowMassFlux(const InputParameters & params);

  /// Get the face velocity times density (used in advection terms)
  Real getMassFlux(const FaceInfo & fi) const;

  /// Get the volumetric face flux (used in advection terms)
  Real getVolumetricFaceFlux(const FaceInfo & fi) const;

  /// Get the registered pressure gradient field used by compatible momentum pressure kernels.
  const LinearFVGradientReader & pressureGradientField() const;

  /// Get the base pressure gradient field used to seed reconstructed gradient updates.
  const LinearFVGradientReader & basePressureGradientField() const;

  /// Get the base-pressure gradient component vectors used to seed reconstructed updates.
  const std::vector<std::unique_ptr<NumericVector<Number>>> & basePressureGradientComponents() const;

  /// Get the momentum-layout H/A component vectors.
  const std::vector<std::unique_ptr<NumericVector<Number>>> & HbyAComponents() const;

  /// Get the momentum-layout 1/A component vectors.
  const std::vector<std::unique_ptr<NumericVector<Number>>> & AinvComponents() const;

  /// Variable number of the pressure variable reconstructed by this object.
  unsigned int pressureVariableNumber() const;

  /// Number of spatial dimensions of the coupled momentum/pressure systems.
  unsigned int dimension() const { return _dim; }

  /// Velocity variable for one spatial component.
  const MooseLinearVariableFVReal & velocityVariable(unsigned int component) const
  {
    return *_vel[component];
  }

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
  void initFaceMassFlux();
  /// Initialize the coupling fields (HbyA and Ainv)
  void initCouplingField();
  /// Update the values of the face velocities in the containers
  void computeFaceMassFlux();

  /// Whether the registered pressure gradient field is produced by the reconstructed method.
  bool usingReconstructedPressureGradientMethod() const;

  /// Capture the lagged velocity gradient used by the Aguerre reconstruction, if in use. No-op
  /// otherwise. Called once before the momentum predictor and between PISO correctors.
  void prepareMomentumPredictor();

  /**
   * Before pressure relaxation, form the conservative reconstructed candidate from the corrected
   * face flux and update the cell velocity from that unrelaxed candidate. No-op for ordinary
   * pressure-gradient methods.
   */
  void preparePressureRelaxation();

  /**
   * After pressure relaxation, refresh the base pressure gradient and publish relaxed
   * reconstructed feedback for the next momentum predictor. Ordinary methods also update the
   * cell velocity here, preserving their existing relaxed-pressure behavior.
   */
  void finalizePressureCorrector();

  /// Compute the base and coupling pressure gradients once, before the first momentum predictor
  /// of a SIMPLE iteration sequence has anything to read.
  void initPressureGradient();

  virtual void meshChanged() override;
  virtual void initialize() override;
  virtual void execute() override {}
  virtual void finalize() override {}
  virtual void initialSetup() override;

  /**
   * Reset reconstructed-gradient feedback state once per attempted time step. This is called by
   * FEProblemBase::timestepSetup(), which TransientBase::takeStep() invokes once before the
   * segregated solve for every attempt (including retries after a rejected/cut-back step), and
   * which SteadyBase::execute() invokes exactly once before the entire outer SIMPLE iteration
   * sequence. That distinction is what keeps steady solves from losing their accumulated feedback
   * every SIMPLE iteration while still making transient time steps (continuous, recovered, or
   * retried) start the momentum predictor from the same state.
   */
  virtual void timestepSetup() override;

  /**
   * Update the momentum system-related information
   * @param momentum_systems Pointers to the momentum systems which are solved for the momentum
   * vector components
   * @param pressure_system Reference to the pressure system
   * @param momentum_system_numbers The numbers of these systems
   */
  void linkMomentumPressureSystems(const std::vector<LinearSystem *> & momentum_systems,
                                   LinearSystem & pressure_system,
                                   const std::vector<unsigned int> & momentum_system_numbers);

  /**
   * Computes the inverse of the diagonal (1/A) of the system matrix plus the H/A components for the
   * pressure equation plus Rhie-Chow interpolation.
   */
  void computeHbyA(const bool with_updated_pressure, const bool verbose);

protected:
  /// Update cell velocity from the supplied momentum-coupling pressure gradient.
  void updateCellVelocity(
      const std::vector<std::unique_ptr<NumericVector<Number>>> & pressure_gradient);

  /// Select the right pressure gradient field and return a reference to the container
  std::vector<std::unique_ptr<NumericVector<Number>>> &
  selectPressureGradient(const bool updated_pressure);

  /// Get the registered pressure gradient component vectors.
  const std::vector<std::unique_ptr<NumericVector<Number>>> & pressureGradientComponents() const;

  /// Access the reconstructed gradient method when it is selected.
  const FVReconstructedPressureGradient & reconstructedGradientMethod() const;

  /// Check the single-variable system layout assumed by reconstructed pressure-gradient vector ops.
  void checkReconstructedPressureGradientCompatibility() const;

  /// Check that the gradient removed in H/A matches the coupling pressure gradient field.
  void checkCouplingPressureGradientIdentity() const;

  /// Compute the cell volumes on the mesh
  void setupMeshInformation();

  /// Populate the face values of the H/A and 1/A fields
  void
  populateCouplingFunctors(const std::vector<std::unique_ptr<NumericVector<Number>>> & raw_hbya,
                           const std::vector<std::unique_ptr<NumericVector<Number>>> & raw_Ainv);

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

  /// Thread 0 copies of the velocity variables; non-const to allow writing
  /// cell velocities in updateCellVelocity().
  std::vector<MooseLinearVariableFVReal *> _vel;

  /// Pointer to the pressure diffusion term in the pressure Poisson equation
  LinearFVAnisotropicDiffusion * _p_diffusion_kernel;

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

  /**
   * for a PISO iteration we need to hold on to the original pressure gradient field.
   * Should not be used in other conditions.
   */
  std::vector<std::unique_ptr<NumericVector<Number>>> _grad_p_current;

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

  /// We will hold a vector of cell volumes to make sure we can do volume corrections rapidly
  std::unique_ptr<NumericVector<Number>> _cell_volumes;

  /// Enumerator for the method used for pressure projection
  const MooseEnum _pressure_projection_method;

  /// Interpolation method used for the pressure diffusion coefficient on faces
  const Moose::FV::InterpMethod _pressure_diffusion_interp_method;

  /// Whether to enforce the discrete identity between the coupling pressure gradient and
  /// the gradient removed while constructing H/A.
  const bool _enforce_coupling_pressure_gradient_identity;

  /// Tolerance for the discrete identity check between coupling and removed pressure gradients.
  const Real _coupling_pressure_gradient_identity_tolerance;

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
