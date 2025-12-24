//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObject.h"
#include "FaceCenteredMapFunctor.h"
#include "SystemBase.h"
#include "NS.h"

class LinearFVFluxKernel;
class LinearFVBoundaryCondition;

namespace NS
{
namespace FV
{

/**
 * This class provides an interface for managing  conjugate heat transfer (CHT)
 * between fluid and solid domains.
 */
class CHTHandler : public MooseObject
{
public:
  /// Constructor with initialization parameters
  CHTHandler(const InputParameters & parameters);

  static InputParameters validParams();

  /// Link energy systems
  void linkEnergySystems(SystemBase * solid_energy_system,
                         SystemBase * fluid_energy_system,
                         std::vector<SystemBase *> pm_radiation_systems);

  /// Set up the boundary condition pairs, functor maps, and every other necessary
  /// structure for the conjugate heat transfer routines
  void setupConjugateHeatTransferContainers();

  /// Run error checks and make sure everything works
  void deduceCHTBoundaryCoupling();

  /// Update the coupling fields for \param side
  void updateCHTBoundaryCouplingFields(const NS::CHTSide side);

  /// Initialize the coupling fields for the conjugate heat transfer routines
  void initializeCHTCouplingFields();

  /// Check if CHT iteration converged
  bool converged() const;

  /// Reset the convergence data
  void resetCHTConvergence();

  /// Increment CHT iterators in the loop
  void incrementCHTIterators();

  /// Sum the integrated fluxes over all processors
  void sumIntegratedFluxes();

  /// Print the integrated heat fluxes
  void printIntegratedFluxes() const;

  /// Reset the heat fluxes to 0
  void resetIntegratedFluxes();

  /// Check if CHT treatment is needed
  virtual bool enabled() const override final;

protected:
  /// Reference to FEProblem
  FEProblemBase & _problem;

  /// Mesh
  MooseMesh & _mesh;

  /// The energy system
  SystemBase * _energy_system;

  /// The solid energy system
  SystemBase * _solid_energy_system;

  /// The solid energy system
  std::vector<SystemBase *> _pm_radiation_systems;

  /// The names of the CHT boundaries
  std::vector<BoundaryName> _cht_boundary_names;

  /// The IDs of the CHT boundaries
  std::vector<BoundaryID> _cht_boundary_ids;

  /// Maximum number of CHT fixed point iterations.
  const unsigned int _max_cht_fpi;

  /// Tolerance for heat flux at the CHT interfaces
  const Real _cht_heat_flux_tolerance;

  /// The relaxation factors for flux fields for the CHT boundaries
  /// first index is solid/fluid second is the interface
  std::vector<std::vector<Real>> _cht_flux_relaxation_factor;

  /// The relaxation factors for temperature fields for the CHT boundaries
  /// first index is solid/fluid second is the interface
  std::vector<std::vector<Real>> _cht_temperature_relaxation_factor;

  /// The solid (0) and fluid (1) system numbers.
  std::vector<unsigned int> _cht_system_numbers;

  /// The participating media radiation system numbers.
  std::vector<unsigned int> _cht_pm_radiation_system_numbers;

  /// The subset of the FaceInfo objects that belong to the given boundaries.
  std::vector<std::vector<const FaceInfo *>> _cht_face_info;

  /// The conduction kernels from the solid/fluid domains. Can't be const, considering we are updating the inner structures for every face.
  std::vector<LinearFVFluxKernel *> _cht_conduction_kernels;

  /// The conduction radiation kernels from the fluid domains.
  std::vector<LinearFVFluxKernel *> _cht_pm_radiation_kernels;

  /// Vector of boundary conditions that describe the conjugate heat transfer from each side.
  std::vector<std::vector<LinearFVBoundaryCondition *>> _cht_boundary_conditions;

  /// Vector of boundary conditions that describe the radiation pm bcs from each side.
  std::vector<std::vector<LinearFVBoundaryCondition *>> _cht_pm_radiation_boundary_conditions;

  /// Functors describing the heat flux on the conjugate heat transfer interfaces.
  /// Two functors per sideset, first is solid second is fluid.
  std::vector<std::vector<FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>>>>
      _boundary_heat_flux;

  /// Integrated flux for the boundaries, first index is the boundary second is solid/fluid.
  std::vector<std::vector<Real>> _integrated_boundary_heat_flux;

  /// Functors describing the heat flux on the conjugate heat transfer interfaces.
  /// Two functors per sideset, first is solid second is fluid.
  std::vector<std::vector<FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>>>>
      _boundary_temperature;

private:
  /// CHT fixed point iteration counter
  unsigned int _fpi_it;
};

inline bool
CHTHandler::enabled() const
{
  return !_cht_boundary_names.empty();
}

inline void
CHTHandler::resetCHTConvergence()
{
  _fpi_it = 0;
}

inline void
CHTHandler::incrementCHTIterators()
{
  _fpi_it++;
}

} // End FV namespace
} // End Moose namespace
