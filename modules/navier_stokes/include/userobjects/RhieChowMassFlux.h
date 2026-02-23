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
#include "LinearFVAnisotropicDiffusion.h"
#include "LinearFVElementalKernel.h"
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <utility>

#include "libmesh/petsc_vector.h"

class MooseMesh;
class INSFVVelocityVariable;
class INSFVPressureVariable;
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

  /// Get interpolation coefficients for the advected quantity (scaled for porosity if used)
  std::pair<Real, Real> getAdvectedInterpolationCoeffs(const FaceInfo & fi,
                                                       Moose::FV::InterpMethod method,
                                                       Real face_mass_flux,
                                                       bool apply_porosity_scaling = true) const;

  /// Get porosity on a face side (returns 1 if porosity is constant/unset)
  Real getFaceSidePorosity(const FaceInfo & fi,
                           bool elem_side,
                           const Moose::StateArg & time) const;

  /// Get the signed baffle jump on a face side (returns 0 if not a baffle face)
  Real getSignedBaffleJump(const FaceInfo & fi, bool elem_side) const;

  /// Get the corrected pressure gradient component at an element center
  Real pressureGradient(const ElemInfo & elem_info, unsigned int component) const;
  /// Get the raw (uncorrected) pressure gradient component at an element center
  Real rawPressureGradient(const ElemInfo & elem_info, unsigned int component) const;

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
  /// Update the cell values of the velocity variables
  void computeCellVelocity();

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
  /// Select the right pressure gradient field and return a reference to the container
  std::vector<std::unique_ptr<NumericVector<Number>>> &
  selectPressureGradient(const bool updated_pressure);

  /// Compute the cell volumes on the mesh
  void setupMeshInformation();

  /// Update baffle jump values based on current face mass fluxes
  void updateBaffleJumps();

  /// Compute corrected pressure gradient values (Gauss) with baffle jumps
  void computeCorrectedPressureGradient();

  /// Check whether a face is a pressure baffle face
  bool isBaffleFace(const FaceInfo & fi) const;

  /// Determine whether the FaceInfo elem side is the baffle owner side
  bool elemIsBaffleOwner(const FaceInfo & fi) const;

  /// Check whether a face should be limited for pressure gradient construction
  bool isPressureGradientLimited(const FaceInfo & fi) const;

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

  /// The thread 0 copy of the x-velocity variable
  std::vector<const MooseLinearVariableFVReal *> _vel;

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

  /// Face superficial velocity reconstructed from the superficial mass flux
  FaceCenteredMapFunctor<RealVectorValue, std::unordered_map<dof_id_type, RealVectorValue>>
      _face_velocity;

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

  /**
   * Functor describing the porosity of the porous medium (defaults to 1 for non-porous)
   */
  const Moose::Functor<Real> & _eps;

  /// The pressure baffle boundary IDs
  std::unordered_set<BoundaryID> _pressure_baffle_boundary_ids;

  /// Boundary IDs on which to limit the pressure gradient construction
  std::unordered_set<BoundaryID> _pressure_gradient_limiter_ids;

  /// Under-relaxation factor for baffle jump updates
  const Real _pressure_baffle_relaxation;

  /// Debug flag for baffle output
  const bool _debug_baffle;

  /// Use oscillation-free reconstruction for the cell velocity based on face fluxes
  const bool _use_flux_velocity_reconstruction;

  /// Under-relaxation for flux-based velocity reconstruction
  const Real _flux_velocity_reconstruction_relaxation;

  /// Whether to use the corrected (baffle-adjusted) pressure gradient in HbyA
  const bool _use_corrected_pressure_gradient;

  /// Whether to use harmonic interpolation for pressure-coupling coefficients
  const bool _use_harmonic_Ainv_interp;

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

  /// We will hold a vector of cell porosity values for fast access
  std::unique_ptr<NumericVector<Number>> _cell_porosity;

  /// The corrected pressure gradient field (with baffle jumps accounted for)
  std::vector<std::unique_ptr<NumericVector<Number>>> _grad_p_corrected;

  /// Stored gradients of the superficial velocity from the previous corrector iteration
  std::vector<std::vector<std::unique_ptr<NumericVector<Number>>>> _grad_w_prev;

  /// Baffle jump storage (neighbor minus owner)
  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> & _baffle_jump;

  /// Enumerator for the method used for pressure projection
  const MooseEnum _pressure_projection_method;

private:
  /// The subset of the FaceInfo objects that actually cover the subdomains which the
  /// flow field is defined on. Cached for performance optimization.
  std::vector<const FaceInfo *> _flow_face_info;

  /// Update face superficial velocities from the current superficial mass flux
  void updateFaceVelocityFromMassFlux();

  /// Update stored superficial velocity gradients from current face velocities
  void updateGradPrevFromFaceVelocity();

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
