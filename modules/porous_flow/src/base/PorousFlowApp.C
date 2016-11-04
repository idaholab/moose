#include "PorousFlowApp.h"
#include "Moose.h"
#include "TensorMechanicsApp.h"
#include "AppFactory.h"
#include "MooseSyntax.h"
#include "FluidPropertiesApp.h"

// UserObjects
#include "PorousFlowDictator.h"
#include "PorousFlowSumQuantity.h"

// DiracKernels
#include "PorousFlowSquarePulsePointSource.h"
#include "PorousFlowPeacemanBorehole.h"
#include "PorousFlowPolyLineSink.h"

// Postprocessors
#include "PorousFlowFluidMass.h"
#include "PorousFlowHeatEnergy.h"
#include "PorousFlowPlotQuantity.h"

// Materials
#include "PorousFlow1PhaseMD_Gaussian.h"
#include "PorousFlow2PhasePP.h"
#include "PorousFlow2PhasePS_VG.h"
#include "PorousFlow1PhaseP.h"
#include "PorousFlow2PhasePP_VG.h"
#include "PorousFlow2PhasePP_RSC.h"
#include "PorousFlowMassFraction.h"
#include "PorousFlow1PhaseP_VG.h"
#include "PorousFlow1PhaseP_BW.h"
#include "PorousFlow2PhasePS.h"
#include "PorousFlowVariableBase.h"
#include "PorousFlowBrine.h"
#include "PorousFlowDensityConstBulk.h"
#include "PorousFlowEffectiveFluidPressure.h"
#include "PorousFlowFluidPropertiesBase.h"
#include "PorousFlowIdealGas.h"
#include "PorousFlowPermeabilityConst.h"
#include "PorousFlowPermeabilityConstFromVar.h"
#include "PorousFlowPermeabilityKozenyCarman.h"
#include "PorousFlowPermeabilityExponential.h"
#include "PorousFlowPorosityConst.h"
#include "PorousFlowPorosityHM.h"
#include "PorousFlowPorosityTM.h"
#include "PorousFlowPorosityTHM.h"
#include "PorousFlowRelativePermeabilityCorey.h"
#include "PorousFlowRelativePermeabilityConst.h"
#include "PorousFlowRelativePermeabilityVG.h"
#include "PorousFlowRelativePermeabilityBW.h"
#include "PorousFlowRelativePermeabilityFLAC.h"
#include "PorousFlowViscosityConst.h"
#include "PorousFlowVolumetricStrain.h"
#include "PorousFlowJoiner.h"
#include "PorousFlowTemperature.h"
#include "PorousFlowThermalConductivityIdeal.h"
#include "PorousFlowMatrixInternalEnergy.h"
#include "PorousFlowInternalEnergyIdeal.h"
#include "PorousFlowEnthalpy.h"
#include "PorousFlowDiffusivityConst.h"
#include "PorousFlowDiffusivityMillingtonQuirk.h"
#include "PorousFlowSingleComponentFluid.h"
#include "PorousFlowNearestQp.h"

// Kernels
#include "PorousFlowAdvectiveFlux.h"
#include "PorousFlowMassTimeDerivative.h"
#include "PorousFlowEffectiveStressCoupling.h"
#include "PorousFlowMassVolumetricExpansion.h"
#include "PorousFlowEnergyTimeDerivative.h"
#include "PorousFlowHeatConduction.h"
#include "PorousFlowHeatAdvection.h"
#include "PorousFlowDispersiveFlux.h"
#include "PorousFlowHeatVolumetricExpansion.h"
#include "PorousFlowPlasticHeatEnergy.h"

// BoundaryConditions
#include "PorousFlowSink.h"
#include "PorousFlowPiecewiseLinearSink.h"
#include "PorousFlowHalfGaussianSink.h"
#include "PorousFlowHalfCubicSink.h"

// AuxKernels
#include "PorousFlowDarcyVelocityComponent.h"
#include "PorousFlowPropertyAux.h"

// Functions
#include "MovingPlanarFront.h"

template<>
InputParameters validParams<PorousFlowApp>()
{
  InputParameters params = validParams<MooseApp>();

  params.set<bool>("use_legacy_uo_initialization") = false;
  params.set<bool>("use_legacy_uo_aux_computation") = false;
  params.set<bool>("use_legacy_output_syntax") = false;

  return params;
}

PorousFlowApp::PorousFlowApp(const InputParameters & parameters) :
    MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  TensorMechanicsApp::registerObjects(_factory);
  FluidPropertiesApp::registerObjects(_factory);
  PorousFlowApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  TensorMechanicsApp::associateSyntax(_syntax, _action_factory);
  FluidPropertiesApp::associateSyntax(_syntax, _action_factory);
  PorousFlowApp::associateSyntax(_syntax, _action_factory);
}

PorousFlowApp::~PorousFlowApp()
{
}

// External entry point for dynamic application loading
extern "C" void PorousFlowApp__registerApps() { PorousFlowApp::registerApps(); }
void
PorousFlowApp::registerApps()
{
  registerApp(PorousFlowApp);
}

// External entry point for dynamic object registration
extern "C" void PorousFlowApp__registerObjects(Factory & factory) { PorousFlowApp::registerObjects(factory); }
void
PorousFlowApp::registerObjects(Factory & factory)
{
  // UserObjects
  registerUserObject(PorousFlowDictator);
  registerUserObject(PorousFlowSumQuantity);

  // DiracKernels
  registerDiracKernel(PorousFlowSquarePulsePointSource);
  registerDiracKernel(PorousFlowPeacemanBorehole);
  registerDiracKernel(PorousFlowPolyLineSink);

  // Postprocessors
  registerPostprocessor(PorousFlowFluidMass);
  registerPostprocessor(PorousFlowHeatEnergy);
  registerPostprocessor(PorousFlowPlotQuantity);

  // Materials
  registerMaterial(PorousFlow1PhaseMD_Gaussian);
  registerMaterial(PorousFlow2PhasePP);
  registerMaterial(PorousFlow2PhasePS_VG);
  registerMaterial(PorousFlow1PhaseP);
  registerMaterial(PorousFlow2PhasePP_VG);
  registerMaterial(PorousFlow2PhasePP_RSC);
  registerMaterial(PorousFlowMassFraction);
  registerMaterial(PorousFlow1PhaseP_VG);
  registerMaterial(PorousFlow1PhaseP_BW);
  registerMaterial(PorousFlow2PhasePS);
  registerMaterial(PorousFlowVariableBase);
  registerMaterial(PorousFlowBrine);
  registerMaterial(PorousFlowDensityConstBulk);
  registerMaterial(PorousFlowEffectiveFluidPressure);
  registerMaterial(PorousFlowFluidPropertiesBase);
  registerMaterial(PorousFlowIdealGas);
  registerMaterial(PorousFlowPermeabilityConst);
  registerMaterial(PorousFlowPermeabilityConstFromVar);
  registerMaterial(PorousFlowPermeabilityKozenyCarman);
  registerMaterial(PorousFlowPermeabilityExponential);
  registerMaterial(PorousFlowPorosityConst);
  registerMaterial(PorousFlowPorosityHM);
  registerMaterial(PorousFlowPorosityTM);
  registerMaterial(PorousFlowPorosityTHM);
  registerMaterial(PorousFlowRelativePermeabilityCorey);
  registerMaterial(PorousFlowRelativePermeabilityConst);
  registerMaterial(PorousFlowRelativePermeabilityVG);
  registerMaterial(PorousFlowRelativePermeabilityBW);
  registerMaterial(PorousFlowRelativePermeabilityFLAC);
  registerMaterial(PorousFlowViscosityConst);
  registerMaterial(PorousFlowVolumetricStrain);
  registerMaterial(PorousFlowJoiner);
  registerMaterial(PorousFlowTemperature);
  registerMaterial(PorousFlowThermalConductivityIdeal);
  registerMaterial(PorousFlowMatrixInternalEnergy);
  registerMaterial(PorousFlowInternalEnergyIdeal);
  registerMaterial(PorousFlowEnthalpy);
  registerMaterial(PorousFlowDiffusivityConst);
  registerMaterial(PorousFlowDiffusivityMillingtonQuirk);
  registerMaterial(PorousFlowSingleComponentFluid);
  registerMaterial(PorousFlowNearestQp);

  // Kernels
  registerKernel(PorousFlowAdvectiveFlux);
  registerKernel(PorousFlowMassTimeDerivative);
  registerKernel(PorousFlowEffectiveStressCoupling);
  registerKernel(PorousFlowMassVolumetricExpansion);
  registerKernel(PorousFlowEnergyTimeDerivative);
  registerKernel(PorousFlowHeatConduction);
  registerKernel(PorousFlowHeatAdvection);
  registerKernel(PorousFlowDispersiveFlux);
  registerKernel(PorousFlowHeatVolumetricExpansion);
  registerKernel(PorousFlowPlasticHeatEnergy);

  // BoundaryConditions
  registerBoundaryCondition(PorousFlowSink);
  registerBoundaryCondition(PorousFlowPiecewiseLinearSink);
  registerBoundaryCondition(PorousFlowHalfGaussianSink);
  registerBoundaryCondition(PorousFlowHalfCubicSink);

  // AuxKernels
  registerAuxKernel(PorousFlowDarcyVelocityComponent);
  registerAuxKernel(PorousFlowPropertyAux);

  // Functions
  registerFunction(MovingPlanarFront);
}

// External entry point for dynamic syntax association
extern "C" void PorousFlowApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory) { PorousFlowApp::associateSyntax(syntax, action_factory); }
void
PorousFlowApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}
