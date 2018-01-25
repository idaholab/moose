//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NavierStokesApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

#include "NSMassInviscidFlux.h"
#include "NSMomentumInviscidFlux.h"
#include "NSEnergyInviscidFlux.h"
#include "NSGravityPower.h"
#include "NSGravityForce.h"
#include "NSThermalBC.h"
#include "NSVelocityAux.h"
#include "NSMachAux.h"
#include "NSInternalEnergyAux.h"
#include "NSSpecificVolumeAux.h"
#include "NSImposedVelocityBC.h"
#include "NSTemperatureAux.h"
#include "NSTemperatureL2.h"
#include "NSPressureAux.h"
#include "NSEnthalpyAux.h"
#include "NSEnergyThermalFlux.h"
#include "NSMomentumViscousFlux.h"
#include "NSEnergyViscousFlux.h"
#include "NSMomentumInviscidFluxWithGradP.h"
#include "NSSUPGMomentum.h"
#include "NSSUPGMass.h"
#include "NSSUPGEnergy.h"
#include "NSMassSpecifiedNormalFlowBC.h"
#include "NSMassUnspecifiedNormalFlowBC.h"
#include "NSInflowThermalBC.h"
#include "NSMomentumInviscidSpecifiedPressureBC.h"
#include "NSMomentumInviscidSpecifiedNormalFlowBC.h"
#include "NSMomentumViscousBC.h"
#include "NSEnergyInviscidSpecifiedPressureBC.h"
#include "NSEnergyInviscidSpecifiedNormalFlowBC.h"
#include "NSEnergyInviscidUnspecifiedBC.h"
#include "NSEnergyInviscidSpecifiedBC.h"
#include "NSEnergyInviscidSpecifiedDensityAndVelocityBC.h"
#include "NSEnergyViscousBC.h"
#include "NSStagnationPressureBC.h"
#include "NSStagnationTemperatureBC.h"
#include "NSImposedVelocityDirectionBC.h"
#include "NSMassWeakStagnationBC.h"
#include "NSMomentumConvectiveWeakStagnationBC.h"
#include "NSMomentumPressureWeakStagnationBC.h"
#include "NSEnergyWeakStagnationBC.h"
#include "NSPenalizedNormalFlowBC.h"
#include "NSMomentumInviscidNoPressureImplicitFlowBC.h"
#include "NSPressureNeumannBC.h"
#include "NSEntropyError.h"
#include "AddNavierStokesVariablesAction.h"
#include "AddNavierStokesICsAction.h"
#include "AddNavierStokesKernelsAction.h"
#include "AddNavierStokesBCsAction.h"
#include "NSInitialCondition.h"
#include "NSWeakStagnationInletBC.h"
#include "NSNoPenetrationBC.h"
#include "NSStaticPressureOutletBC.h"

// So we can register objects from the fluid_properties module.
#include "FluidPropertiesApp.h"

//
// Incompressible
//

// Kernels
#include "INSMass.h"
#include "INSMassRZ.h"
#include "INSMomentumTimeDerivative.h"
#include "INSMomentumTractionForm.h"
#include "INSMomentumTractionFormRZ.h"
#include "INSMomentumLaplaceForm.h"
#include "INSMomentumLaplaceFormRZ.h"
#include "INSTemperatureTimeDerivative.h"
#include "INSTemperature.h"
#include "INSSplitMomentum.h"
#include "INSProjection.h"
#include "INSPressurePoisson.h"
#include "INSChorinPredictor.h"
#include "INSChorinCorrector.h"
#include "INSChorinPressurePoisson.h"
#include "INSCompressibilityPenalty.h"

// BCs
#include "INSMomentumNoBCBCTractionForm.h"
#include "INSMomentumNoBCBCLaplaceForm.h"
#include "INSTemperatureNoBCBC.h"
#include "ImplicitNeumannBC.h"

// AuxKernels
#include "INSCourant.h"
#include "INSDivergenceAux.h"

// Materials - this will eventually be replaced by FluidProperties stuff...
#include "Air.h"

// Postprocessors
#include "INSExplicitTimestepSelector.h"
#include "VolumetricFlowRate.h"

// Functions
#include "WedgeFunction.h"

template <>
InputParameters
validParams<NavierStokesApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

NavierStokesApp::NavierStokesApp(InputParameters parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  NavierStokesApp::registerObjectDepends(_factory);
  NavierStokesApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  NavierStokesApp::associateSyntaxDepends(_syntax, _action_factory);
  NavierStokesApp::associateSyntax(_syntax, _action_factory);

  Moose::registerExecFlags(_factory);
  NavierStokesApp::registerExecFlags(_factory);
}

NavierStokesApp::~NavierStokesApp() {}

// External entry point for dynamic application loading
extern "C" void
NavierStokesApp__registerApps()
{
  NavierStokesApp::registerApps();
}
void
NavierStokesApp::registerApps()
{
  registerApp(NavierStokesApp);
}

void
NavierStokesApp::registerObjectDepends(Factory & factory)
{
  FluidPropertiesApp::registerObjects(factory);
}

// External entry point for dynamic object registration
extern "C" void
NavierStokesApp__registerObjects(Factory & factory)
{
  NavierStokesApp::registerObjects(factory);
}
void
NavierStokesApp::registerObjects(Factory & factory)
{
  registerKernel(NSMassInviscidFlux);
  registerKernel(NSMomentumInviscidFlux);
  registerKernel(NSEnergyInviscidFlux);
  registerKernel(NSGravityPower);
  registerKernel(NSGravityForce);
  registerKernel(NSTemperatureL2);
  registerBoundaryCondition(NSThermalBC);
  registerAux(NSVelocityAux);
  registerAux(NSMachAux);
  registerAux(NSInternalEnergyAux);
  registerAux(NSSpecificVolumeAux);
  registerBoundaryCondition(NSImposedVelocityBC);
  registerAux(NSTemperatureAux);
  registerAux(NSPressureAux);
  registerAux(NSEnthalpyAux);
  registerKernel(NSEnergyThermalFlux);
  registerKernel(NSMomentumViscousFlux);
  registerKernel(NSEnergyViscousFlux);
  registerKernel(NSMomentumInviscidFluxWithGradP);
  registerKernel(NSSUPGMomentum);
  registerKernel(NSSUPGMass);
  registerKernel(NSSUPGEnergy);
  registerBoundaryCondition(NSMassSpecifiedNormalFlowBC);
  registerBoundaryCondition(NSMassUnspecifiedNormalFlowBC);
  registerBoundaryCondition(NSInflowThermalBC);
  registerBoundaryCondition(NSMomentumInviscidSpecifiedPressureBC);
  registerBoundaryCondition(NSMomentumInviscidSpecifiedNormalFlowBC);
  registerBoundaryCondition(NSMomentumViscousBC);
  registerBoundaryCondition(NSEnergyInviscidSpecifiedPressureBC);
  registerBoundaryCondition(NSEnergyInviscidSpecifiedNormalFlowBC);
  registerBoundaryCondition(NSEnergyInviscidUnspecifiedBC);
  registerBoundaryCondition(NSEnergyInviscidSpecifiedBC);
  registerBoundaryCondition(NSEnergyInviscidSpecifiedDensityAndVelocityBC);
  registerBoundaryCondition(NSEnergyViscousBC);
  registerBoundaryCondition(NSStagnationPressureBC);
  registerBoundaryCondition(NSStagnationTemperatureBC);
  registerBoundaryCondition(NSImposedVelocityDirectionBC);
  registerBoundaryCondition(NSMassWeakStagnationBC);
  registerBoundaryCondition(NSMomentumConvectiveWeakStagnationBC);
  registerBoundaryCondition(NSMomentumPressureWeakStagnationBC);
  registerBoundaryCondition(NSEnergyWeakStagnationBC);
  registerBoundaryCondition(NSPenalizedNormalFlowBC);
  registerBoundaryCondition(NSMomentumInviscidNoPressureImplicitFlowBC);
  registerBoundaryCondition(NSPressureNeumannBC);
  registerPostprocessor(NSEntropyError);
  registerInitialCondition(NSInitialCondition);
  // Boundary condition meta-objects
  registerObject(NSWeakStagnationInletBC);
  registerObject(NSNoPenetrationBC);
  registerObject(NSStaticPressureOutletBC);

  //
  // Incompressible
  //

  // Kernels
  registerKernel(INSMass);
  registerKernel(INSMassRZ);
  registerKernel(INSMomentumTimeDerivative);
  registerKernel(INSMomentumTractionForm);
  registerKernel(INSMomentumTractionFormRZ);
  registerKernel(INSMomentumLaplaceForm);
  registerKernel(INSMomentumLaplaceFormRZ);
  registerKernel(INSTemperatureTimeDerivative);
  registerKernel(INSTemperature);
  registerKernel(INSSplitMomentum);
  registerKernel(INSProjection);
  registerKernel(INSPressurePoisson);
  registerKernel(INSChorinPredictor);
  registerKernel(INSChorinCorrector);
  registerKernel(INSChorinPressurePoisson);
  registerKernel(INSCompressibilityPenalty);

  // BCs
  registerBoundaryCondition(INSMomentumNoBCBCTractionForm);
  registerBoundaryCondition(INSMomentumNoBCBCLaplaceForm);
  registerBoundaryCondition(INSTemperatureNoBCBC);
  registerBoundaryCondition(ImplicitNeumannBC);

  // AuxKernels
  registerAux(INSCourant);
  registerAux(INSDivergenceAux);

  // Postprocessors
  registerPostprocessor(INSExplicitTimestepSelector);
  registerPostprocessor(VolumetricFlowRate);

  // Materials
  registerMaterial(Air);

  // Functions
  registerFunction(WedgeFunction);
}

void
NavierStokesApp::associateSyntaxDepends(Syntax & syntax, ActionFactory & action_factory)
{
  FluidPropertiesApp::associateSyntax(syntax, action_factory);
}

// External entry point for dynamic syntax association
extern "C" void
NavierStokesApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  NavierStokesApp::associateSyntax(syntax, action_factory);
}
void
NavierStokesApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
#undef registerAction
#define registerAction(type, action)                                                               \
  action_factory.reg<type>(stringifyName(type), action, __FILE__, __LINE__)

  // Create the syntax
  registerSyntax("AddNavierStokesVariablesAction", "Modules/NavierStokes/Variables");
  registerSyntax("AddNavierStokesICsAction", "Modules/NavierStokes/ICs");
  registerSyntax("AddNavierStokesKernelsAction", "Modules/NavierStokes/Kernels");
  registerSyntax("AddNavierStokesBCsAction", "Modules/NavierStokes/BCs/*");

  // add variables action
  registerTask("add_navier_stokes_variables", /*is_required=*/false);
  addTaskDependency("add_navier_stokes_variables", "add_variable");
  registerAction(AddNavierStokesVariablesAction, "add_navier_stokes_variables");

  // add ICs action
  registerTask("add_navier_stokes_ics", /*is_required=*/false);
  addTaskDependency("add_navier_stokes_ics", "add_ic");
  registerAction(AddNavierStokesICsAction, "add_navier_stokes_ics");

  // add Kernels action
  registerTask("add_navier_stokes_kernels", /*is_required=*/false);
  addTaskDependency("add_navier_stokes_kernels", "add_kernel");
  registerAction(AddNavierStokesKernelsAction, "add_navier_stokes_kernels");

  // add BCs actions
  registerMooseObjectTask("add_navier_stokes_bcs", NSWeakStagnationInletBC, /*is_required=*/false);
  appendMooseObjectTask("add_navier_stokes_bcs", NSNoPenetrationBC);
  appendMooseObjectTask("add_navier_stokes_bcs", NSStaticPressureOutletBC);
  addTaskDependency("add_navier_stokes_bcs", "add_bc");
  registerAction(AddNavierStokesBCsAction, "add_navier_stokes_bcs");

#undef registerAction
#define registerAction(type, action) action_factory.regLegacy<type>(stringifyName(type), action)
}

// External entry point for dynamic execute flag registration
extern "C" void
NavierStokesApp__registerExecFlags(Factory & factory)
{
  NavierStokesApp::registerExecFlags(factory);
}
void
NavierStokesApp::registerExecFlags(Factory & /*factory*/)
{
}
