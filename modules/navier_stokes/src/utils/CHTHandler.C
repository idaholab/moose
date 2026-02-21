//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Moose includes
#include "CHTHandler.h"
#include "LinearFVFluxKernel.h"
#include "LinearFVDiffusion.h"
#include "LinearFVAnisotropicDiffusion.h"
#include "LinearFVBoundaryCondition.h"
#include "LinearFVCHTBCInterface.h"
#include "FEProblemBase.h"

namespace NS
{
namespace FV
{

InputParameters
CHTHandler::validParams()
{
  auto params = emptyInputParameters();
  params.addParam<std::vector<BoundaryName>>(
      "cht_interfaces",
      {},
      "The interfaces where we would like to add conjugate heat transfer handling.");

  params.addRangeCheckedParam<unsigned int>(
      "max_cht_fpi",
      1,
      "max_cht_fpi >= 1",
      "Number of maximum fixed point iterations (FPI). Currently only applied to"
      " conjugate heat transfer simulations. The default value of 1 essentially keeps"
      " the FPI feature turned off. CHT iteration ends after this number of iteration even if the "
      "tolerance is not met.");

  params.addRangeCheckedParam<Real>(
      "cht_heat_flux_tolerance",
      1e-5,
      "cht_heat_flux_tolerance > 0 & cht_heat_flux_tolerance <= 1.0",
      "The relative tolerance for terminating conjugate heat transfer iteration before the maximum "
      "number of CHT iterations. Relative tolerance is ignore if the maximum number of CHT "
      "iterations is reached.");

  params.addParam<std::vector<Real>>(
      "cht_fluid_temperature_relaxation",
      {},
      "The relaxation factors for the boundary temperature when being updated on the fluid side.");
  params.addParam<std::vector<Real>>(
      "cht_solid_temperature_relaxation",
      {},
      "The relaxation factors for the boundary temperature when being updated on the solid side.");
  params.addParam<std::vector<Real>>(
      "cht_fluid_flux_relaxation",
      {},
      "The relaxation factors for the boundary flux when being updated on the fluid side.");
  params.addParam<std::vector<Real>>(
      "cht_solid_flux_relaxation",
      {},
      "The relaxation factors for the boundary flux when being updated on the solid side.");

  params.addParamNamesToGroup(
      "cht_interfaces max_cht_fpi cht_heat_flux_tolerance cht_fluid_temperature_relaxation "
      "cht_solid_temperature_relaxation cht_fluid_flux_relaxation "
      "cht_solid_flux_relaxation",
      "Conjugate Heat Transfer");

  return params;
}

CHTHandler::CHTHandler(const InputParameters & params)
  : MooseObject(params),
    _problem(*getCheckedPointerParam<FEProblemBase *>(
        "_fe_problem_base", "This might happen if you don't have a mesh")),
    _mesh(_problem.mesh()),
    _cht_boundary_names(getParam<std::vector<BoundaryName>>("cht_interfaces")),
    _cht_boundary_ids(_mesh.getBoundaryIDs(_cht_boundary_names)),
    _max_cht_fpi(getParam<unsigned int>("max_cht_fpi")),
    _cht_heat_flux_tolerance(getParam<Real>("cht_heat_flux_tolerance"))
{
  if (isParamSetByUser("cht_interfaces") && !_cht_boundary_names.size())
    paramError("cht_interfaces", "You must declare at least one interface!");
}

void
CHTHandler::linkEnergySystems(SystemBase * solid_energy_system,
                              SystemBase * fluid_energy_system,
                              std::vector<SystemBase *> pm_radiation_systems)
{
  _energy_system = fluid_energy_system;
  _solid_energy_system = solid_energy_system;
  _pm_radiation_systems = pm_radiation_systems;

  if (!_energy_system || !_solid_energy_system)
    paramError("cht_interfaces",
               "You selected to do conjugate heat transfer treatment, but it needs two energy "
               "systems: a solid and a fluid. One of these systems is missing.");
}

void
CHTHandler::deduceCHTBoundaryCoupling()
{
  if (_solid_energy_system->nVariables() != 1)
    mooseError("We should have only one variable in the solid energy system: ",
               _energy_system->name(),
               "! Right now we have: ",
               Moose::stringify(_solid_energy_system->getVariableNames()));
  if (_energy_system->nVariables() != 1)
    mooseError("We should have only one variable in the fluid energy system: ",
               _energy_system->name(),
               "! Right now we have: ",
               Moose::stringify(_energy_system->getVariableNames()));
  const std::vector<std::string> solid_fluid({"solid", "fluid"});

  // We do some setup at the beginning to make sure the container sizes are good
  _cht_system_numbers =
      std::vector<unsigned int>({_solid_energy_system->number(), _energy_system->number()});
  _cht_conduction_kernels = std::vector<LinearFVFluxKernel *>({nullptr, nullptr});
  _cht_boundary_conditions.clear();
  _cht_boundary_conditions.resize(_cht_boundary_names.size(), {nullptr, nullptr});

  // Populate the PM radiation system numbers
  if (!_pm_radiation_systems.empty())
  {
    for (const auto sys_i : index_range(_pm_radiation_systems))
      _cht_pm_radiation_system_numbers.push_back(_pm_radiation_systems[sys_i]->number());

    // Reserve space for _cht_pm_radiation_kernels based on the size of
    // _cht_pm_radiation_system_numbers
    _cht_pm_radiation_kernels.reserve(_cht_pm_radiation_system_numbers.size());
    // Reserve space for pm radiation boundary conditions
    _cht_pm_radiation_boundary_conditions.clear();
    _cht_pm_radiation_boundary_conditions.resize(
        _cht_boundary_names.size(),
        std::vector<LinearFVBoundaryCondition *>(_cht_pm_radiation_system_numbers.size(), nullptr));
  }

  const auto flux_relaxation_param_names =
      std::vector<std::string>({"cht_solid_flux_relaxation", "cht_fluid_flux_relaxation"});
  const auto temperature_relaxation_param_names = std::vector<std::string>(
      {"cht_solid_temperature_relaxation", "cht_fluid_temperature_relaxation"});
  _cht_flux_relaxation_factor.clear();
  _cht_flux_relaxation_factor.resize(2, std::vector<Real>(_cht_boundary_names.size(), 1.0));
  _cht_temperature_relaxation_factor.clear();
  _cht_temperature_relaxation_factor.resize(2, std::vector<Real>(_cht_boundary_names.size(), 1.0));

  for (const auto region_index : index_range(solid_fluid))
  {
    // First thing, we fetch the relaxation parameter values
    const auto & flux_param_value =
        getParam<std::vector<Real>>(flux_relaxation_param_names[region_index]);
    if (flux_param_value.empty() || (flux_param_value.size() != _cht_boundary_names.size()))
      paramError(flux_relaxation_param_names[region_index],
                 "The number of relaxation factors is not the same as the number of interfaces!");

    _cht_flux_relaxation_factor[region_index] = flux_param_value;
    // We have to do the range check here because the intput parameter check errors if the vector is
    // empty
    for (const auto param : _cht_flux_relaxation_factor[region_index])
      if (param <= 0 || param > 1.0)
        paramError(flux_relaxation_param_names[region_index],
                   "The relaxation parameter should be between 0 and 1!");

    const auto & temperature_param_value =
        getParam<std::vector<Real>>(temperature_relaxation_param_names[region_index]);
    if (temperature_param_value.empty() ||
        (temperature_param_value.size() != _cht_boundary_names.size()))
      paramError(temperature_relaxation_param_names[region_index],
                 "The number of relaxation factors is not the same as the number of interfaces!");

    _cht_temperature_relaxation_factor[region_index] = temperature_param_value;
    // We have to do the range check here because the intput parameter check errors if the vector is
    // empty
    for (const auto param : _cht_temperature_relaxation_factor[region_index])
      if (param <= 0 || param > 1.0)
        paramError(temperature_relaxation_param_names[region_index],
                   "The relaxation parameter should be between 0 and 1!");

    // We then fetch the conduction kernels
    std::vector<LinearFVFluxKernel *> flux_kernels;
    _app.theWarehouse()
        .query()
        .template condition<AttribSystem>("LinearFVFluxKernel")
        .template condition<AttribVar>(0)
        .template condition<AttribSysNum>(_cht_system_numbers[region_index])
        .queryInto(flux_kernels);

    // We then fetch the radiation conduction kernels in the fluid region
    if (!_pm_radiation_systems.empty() && region_index == 1)
      for (const auto sys_i : index_range(_pm_radiation_systems))
      {
        // We then fetch the radiation conduction kernels
        std::vector<LinearFVFluxKernel *> radiation_kernels;
        _app.theWarehouse()
            .query()
            .template condition<AttribSystem>("LinearFVFluxKernel")
            .template condition<AttribVar>(0)
            .template condition<AttribSysNum>(_cht_pm_radiation_system_numbers[sys_i])
            .queryInto(radiation_kernels);

        if (radiation_kernels.size() > 1)
          mooseError(
              "We already have a kernel that describes the participating media radiation diffusion "
              "with the name: ",
              radiation_kernels[0]->name(),
              ". Make sure that you have only one conduction kernel.");
        else if (radiation_kernels.empty())
          mooseError("We did not find a diffusion kernel for the participating media radiation "
                     "diffusion to compute the "
                     "radiative heat flux. Please add a diffusion kernel.");
        else
          _cht_pm_radiation_kernels.push_back(radiation_kernels[0]);
      }

    for (auto kernel : flux_kernels)
    {
      auto check_diff = dynamic_cast<LinearFVDiffusion *>(kernel);
      auto check_aniso_diff = dynamic_cast<LinearFVAnisotropicDiffusion *>(kernel);
      if (_cht_conduction_kernels[region_index] && (check_diff || check_aniso_diff))
        mooseError("We already have a kernel that describes the heat conduction for the ",
                   solid_fluid[region_index],
                   " domain: ",
                   _cht_conduction_kernels[region_index]->name(),
                   " We found another one with the name: ",
                   (check_diff ? check_diff->name() : check_aniso_diff->name()),
                   " Make sure that you have only one conduction kernel on the ",
                   solid_fluid[region_index],
                   " side!");

      if (check_diff || check_aniso_diff)
        _cht_conduction_kernels[region_index] = kernel;
    }

    // Then we check the boundary conditions, to make sure at least there is something defined
    // from both sides
    for (const auto bd_index : index_range(_cht_boundary_names))
    {
      const auto & boundary_name = _cht_boundary_names[bd_index];
      const auto boundary_id = _cht_boundary_ids[bd_index];

      std::vector<LinearFVBoundaryCondition *> bcs;
      _problem.getMooseApp()
          .theWarehouse()
          .query()
          .template condition<AttribSystem>("LinearFVBoundaryCondition")
          .template condition<AttribVar>(0)
          .template condition<AttribSysNum>(_cht_system_numbers[region_index])
          .template condition<AttribBoundaries>(boundary_id)
          .queryInto(bcs);

      // We then fetch the radiation conduction bcs in the fluid region (i.e MarshakBC in P1)
      if (!_pm_radiation_systems.empty() && region_index == 1)
        for (const auto sys_i : index_range(_pm_radiation_systems))
        {
          std::vector<LinearFVBoundaryCondition *> rad_bcs;
          _app.theWarehouse()
              .query()
              .template condition<AttribSystem>("LinearFVBoundaryCondition")
              .template condition<AttribVar>(0)
              .template condition<AttribSysNum>(_cht_pm_radiation_system_numbers[sys_i])
              .template condition<AttribBoundaries>(boundary_id)
              .queryInto(rad_bcs);

          if (!rad_bcs.empty())
            _cht_pm_radiation_boundary_conditions[bd_index][sys_i] = rad_bcs[0];
          else
            mooseError("No LinearFVBoundaryCondition found for the given boundary or system.");
        }

      if (bcs.size() != 1)
        mooseError("We found multiple or no boundary conditions for solid energy on boundary ",
                   boundary_name,
                   " (ID: ",
                   boundary_id,
                   "). Make sure you define exactly one for conjugate heat transfer applications!");
      _cht_boundary_conditions[bd_index][region_index] = bcs[0];

      if (!dynamic_cast<LinearFVCHTBCInterface *>(_cht_boundary_conditions[bd_index][region_index]))
        mooseError("The selected boundary condition cannot be used with CHT problems! Make sure it "
                   "inherits from LinearFVCHTBCInterface!");
    }
  }
}

void
CHTHandler::setupConjugateHeatTransferContainers()
{
  // We already error in initialSetup if we have more variables
  const auto * fluid_variable =
      dynamic_cast<const MooseLinearVariableFVReal *>(&_energy_system->getVariable(0, 0));
  const auto * solid_variable =
      dynamic_cast<const MooseLinearVariableFVReal *>(&_solid_energy_system->getVariable(0, 0));

  _cht_face_info.clear();
  _cht_face_info.resize(_cht_boundary_ids.size());
  _boundary_heat_flux.clear();
  _boundary_temperature.clear();
  _integrated_boundary_heat_flux.clear();

  for (const auto bd_index : index_range(_cht_boundary_ids))
  {
    const auto bd_id = _cht_boundary_ids[bd_index];
    const auto & bd_name = _cht_boundary_names[bd_index];

    // We populate the face infos for every interface
    auto & bd_fi_container = _cht_face_info[bd_index];
    for (auto & fi : _problem.mesh().faceInfo())
      if (fi->boundaryIDs().count(bd_id))
        bd_fi_container.push_back(fi);

    // We do this because the coupling functors should be evaluated on both sides
    // of the interface and there are rigorous checks if the functors don't support a subdomain
    std::set<SubdomainID> combined_set;
    std::set_union(solid_variable->blockIDs().begin(),
                   solid_variable->blockIDs().end(),
                   fluid_variable->blockIDs().begin(),
                   fluid_variable->blockIDs().end(),
                   std::inserter(combined_set, combined_set.begin()));

    // We instantiate the coupling fuctors for heat flux and temperature
    FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> solid_bd_flux(
        _problem.mesh(), combined_set, "heat_flux_to_solid_" + bd_name);
    FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> fluid_bd_flux(
        _problem.mesh(), combined_set, "heat_flux_to_fluid_" + bd_name);

    _boundary_heat_flux.push_back(
        std::vector<FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>>>(
            {std::move(solid_bd_flux), std::move(fluid_bd_flux)}));
    auto & flux_container = _boundary_heat_flux.back();

    _integrated_boundary_heat_flux.push_back(std::vector<Real>({0.0, 0.0}));

    FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> solid_bd_temperature(
        _problem.mesh(), combined_set, "interface_temperature_solid_" + bd_name);
    FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> fluid_bd_temperature(
        _problem.mesh(), combined_set, "interface_temperature_fluid_" + bd_name);

    _boundary_temperature.push_back(
        std::vector<FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>>>(
            {std::move(solid_bd_temperature), std::move(fluid_bd_temperature)}));
    auto & temperature_container = _boundary_temperature.back();

    // Time to register the functors on all of the threads
    for (const auto tid : make_range(libMesh::n_threads()))
    {
      _problem.addFunctor("heat_flux_to_solid_" + bd_name, flux_container[NS::CHTSide::SOLID], tid);
      _problem.addFunctor("heat_flux_to_fluid_" + bd_name, flux_container[NS::CHTSide::FLUID], tid);
      _problem.addFunctor(
          "interface_temperature_solid_" + bd_name, temperature_container[NS::CHTSide::SOLID], tid);
      _problem.addFunctor(
          "interface_temperature_fluid_" + bd_name, temperature_container[NS::CHTSide::FLUID], tid);
    }

    // Initialize the containers, they will be filled with correct values soon.
    // Before any solve happens.
    for (const auto region_index : make_range(2))
      for (auto & fi : bd_fi_container)
      {
        flux_container[region_index][fi->id()] = 0.0;
        temperature_container[region_index][fi->id()] = 0.0;
      }
  }
}

void
CHTHandler::initializeCHTCouplingFields()
{
  for (const auto bd_index : index_range(_cht_boundary_ids))
  {
    const auto & bd_fi_container = _cht_face_info[bd_index];
    auto & temperature_container = _boundary_temperature[bd_index];

    for (const auto region_index : make_range(2))
    {
      // Can't be const considering we will update members from here
      auto bc = _cht_boundary_conditions[bd_index][region_index];
      for (const auto & fi : bd_fi_container)
      {
        bc->setupFaceData(fi, fi->faceType(std::make_pair(0, _cht_system_numbers[region_index])));
        temperature_container[1 - region_index][fi->id()] = bc->computeBoundaryValue();
      }
    }
  }
}

void
CHTHandler::updateCHTBoundaryCouplingFields(const NS::CHTSide side)
{
  // Well we can just use the face that this enum casts into int very nicely
  // we can use it to get the index of the other side
  const NS::CHTSide other_side = static_cast<NS::CHTSide>(1 - side);

  for (const auto bd_index : index_range(_cht_boundary_ids))
  {
    auto & other_bc = _cht_boundary_conditions[bd_index][other_side];
    auto & other_kernel = _cht_conduction_kernels[other_side];

    // We get the relaxation from the other side, so if we are fluid side we get the solid
    // relaxation
    const auto temperature_relaxation = _cht_flux_relaxation_factor[other_side][bd_index];
    const auto flux_relaxation = _cht_temperature_relaxation_factor[other_side][bd_index];

    // Fetching the right container here, if side is fluid we fetch "heat_flux_to_fluid"
    auto & flux_container = _boundary_heat_flux[bd_index][side];
    // Fetching the other side's contaienr here, if side is fluid we fetch the solid temperature
    auto & temperature_container = _boundary_temperature[bd_index][other_side];
    // We will also update the integrated flux for output info
    auto & integrated_flux = _integrated_boundary_heat_flux[bd_index][side];
    // We are recomputing this so, time to zero this out
    integrated_flux = 0.0;

    const auto & bd_fi_container = _cht_face_info[bd_index];

    // We enter the face loop to update the coupling fields
    for (const auto & fi : bd_fi_container)
    {
      other_kernel->setupFaceData(fi);
      // We will want the flux in W/m2 for the coupling so no face integral for now,
      // this can cause issues if we start using face area in the kernels
      // for more than just face integral multipliers.
      // Also, if we decide to not require overlapping meshes on the boundary
      // this will probably have to change.
      other_kernel->setCurrentFaceArea(1.0);
      other_bc->setupFaceData(fi, fi->faceType(std::make_pair(0, _cht_system_numbers[other_side])));

      // T_new = relaxation * T_boundary + (1-relaxation) * T_old
      temperature_container[fi->id()] =
          temperature_relaxation * other_bc->computeBoundaryValue() +
          (1 - temperature_relaxation) * temperature_container[fi->id()];

      // Flux_new = relaxation * Flux_boundary + (1-relaxation) * Flux_old,
      // minus sign is due to the normal differences

      // Conductive flux
      auto flux = other_kernel->computeBoundaryFlux(*other_bc);

      // If participating media radiation system exists we add the heat flux from the fluid
      // to the solid region.
      if (!_pm_radiation_systems.empty() && side == NS::CHTSide::SOLID)
        for (const auto sys_i : index_range(_pm_radiation_systems))
        {
          _cht_pm_radiation_kernels[sys_i]->setupFaceData(fi);
          _cht_pm_radiation_kernels[sys_i]->setCurrentFaceArea(1.0);
          _cht_pm_radiation_boundary_conditions[bd_index][sys_i]->setupFaceData(
              fi, fi->faceType(std::make_pair(0, _cht_pm_radiation_system_numbers[sys_i])));
          flux += _cht_pm_radiation_kernels[sys_i]->computeBoundaryFlux(
              *_cht_pm_radiation_boundary_conditions[bd_index][sys_i]);
        }

      flux_container[fi->id()] =
          flux_relaxation * flux + (1 - flux_relaxation) * flux_container[fi->id()];

      // We do the integral here
      integrated_flux += flux * fi->faceArea() * fi->faceCoord();
    }
  }
}

void
CHTHandler::sumIntegratedFluxes()
{
  for (const auto i : index_range(_integrated_boundary_heat_flux))
  {
    auto & integrated_fluxes = _integrated_boundary_heat_flux[i];
    _problem.comm().sum(integrated_fluxes[NS::CHTSide::SOLID]);
    _problem.comm().sum(integrated_fluxes[NS::CHTSide::FLUID]);
  }
}

void
CHTHandler::printIntegratedFluxes() const
{
  for (const auto i : index_range(_integrated_boundary_heat_flux))
  {
    auto & integrated_fluxes = _integrated_boundary_heat_flux[i];
    _console << " Iteration " << _fpi_it << " Boundary " << _cht_boundary_names[i]
             << " flux on solid side " << integrated_fluxes[NS::CHTSide::SOLID]
             << " flux on fluid side: " << integrated_fluxes[NS::CHTSide::FLUID] << std::endl;
  }
}

void
CHTHandler::resetIntegratedFluxes()
{
  for (const auto i : index_range(_integrated_boundary_heat_flux))
    _integrated_boundary_heat_flux[i] = std::vector<Real>({0.0, 0.0});
}

bool
CHTHandler::converged() const
{
  if (_fpi_it >= _max_cht_fpi)
    return true;

  for (const auto & boundary_flux : _integrated_boundary_heat_flux)
  {
    const Real f1 = boundary_flux[0];
    const Real f2 = boundary_flux[1];

    // Special case: both are zero at startup not converged yet
    if (_fpi_it != 0 && (f1 == 0.0 && f2 == 0.0))
      return true;

    // These fluxes should be of opposite sign
    const Real diff = std::abs(f1 + f2);
    const Real denom = std::max({std::fabs(f1), std::fabs(f2), Real(1e-14)});
    const Real rel_diff = diff / denom;

    if (rel_diff >= _cht_heat_flux_tolerance)
      return false;
  }

  return _fpi_it;
}

} // End FV namespace
} // End Moose namespace
