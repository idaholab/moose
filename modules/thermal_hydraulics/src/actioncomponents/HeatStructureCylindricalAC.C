//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatStructureCylindricalAC.h"
#include "MooseMesh.h"
#include "MultiMooseEnum.h"

// Registered under the classic Component's name ('HeatStructureCylindrical') rather than this
// class's own name; see FlowChannel1PhaseAC.C for why.
registerMooseActionAliased("ThermalHydraulicsApp",
                           HeatStructureCylindricalAC,
                           "HeatStructureCylindrical",
                           "add_mesh_generator");
registerMooseActionAliased("ThermalHydraulicsApp",
                           HeatStructureCylindricalAC,
                           "HeatStructureCylindrical",
                           "init_component_physics");
registerMooseActionAliased("ThermalHydraulicsApp",
                           HeatStructureCylindricalAC,
                           "HeatStructureCylindrical",
                           "add_material");
registerMooseActionAliased("ThermalHydraulicsApp",
                           HeatStructureCylindricalAC,
                           "HeatStructureCylindrical",
                           "setup_component");
registerActionComponentAliased("ThermalHydraulicsApp",
                               HeatStructureCylindricalAC,
                               "HeatStructureCylindrical");

InputParameters
HeatStructureCylindricalAC::validParams()
{
  InputParameters params = HeatStructureBaseAC::validParams();
  params.addParam<Real>("inner_radius", 0., "Inner radius of the heat structure [m]");
  params.addClassDescription("Cylindrical heat structure.");
  return params;
}

HeatStructureCylindricalAC::HeatStructureCylindricalAC(const InputParameters & params)
  : ActionComponent(params),
    HeatStructureBaseAC(params),
    _inner_radius(getParam<Real>("inner_radius"))
{
  addRequiredTask("setup_component");
}

void
HeatStructureCylindricalAC::setupComponent()
{
  // MooseMesh::setCoordSystem()/setGeneralAxisymmetricCoordAxes() must each be called exactly once
  // per simulation, covering every RZ block at once (see class documentation) - so only the first
  // HeatStructureCylindricalAC instance (in action-warehouse order) does anything here, making one
  // combined call on behalf of every instance; the rest are no-ops.
  const auto instances = _awh.getActions<HeatStructureCylindricalAC>();
  if (instances.empty() || instances[0] != this)
    return;

  std::vector<SubdomainName> all_blocks;
  std::vector<std::pair<Point, RealVectorValue>> all_axes;
  for (const auto * instance : instances)
  {
    for (const auto & block : instance->_blocks)
      all_blocks.push_back(block);
    all_axes.insert(all_axes.end(),
                    instance->_blocks.size(),
                    std::make_pair(instance->translation(), instance->direction()));
  }

  // Note: MultiMooseEnum's single-string constructor sets the *valid options* list, not the
  // selected value - the two-argument form is required to actually select a value, and the valid
  // Moose::CoordinateSystemType strings are "XYZ"/"RZ"/"RSPHERICAL" (see Conversion.C), not the
  // C++ enumerator names themselves. One "RZ" per block selects the precise, per-block code path
  // in setCoordSystem() rather than the single-value one, which would (harmlessly, here, since
  // covering every block at once, but worth avoiding regardless) mark every mesh subdomain RZ.
  std::string rz_per_block;
  for (unsigned int i = 0; i < all_blocks.size(); i++)
    rz_per_block += (i == 0 ? "" : " ") + std::string("RZ");
  _awh.getMesh()->setCoordSystem(all_blocks, MultiMooseEnum("RZ", rz_per_block));
  _awh.getMesh()->setGeneralAxisymmetricCoordAxes(all_blocks, all_axes);
}
