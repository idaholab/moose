#include "PorousFlowApp.h"
#include "Moose.h"
#include "TensorMechanicsApp.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

// UserObjects
#include "PorousFlowDictator.h"

// DiracKernels
#include "PorousFlowSquarePulsePointSource.h"

// Postprocessors
#include "PorousFlowFluidMass.h"

// Materials
#include "PorousFlow1PhaseMD_Gaussian.h"
#include "PorousFlow2PhasePP.h"
#include "PorousFlow2PhasePS_VG.h"
#include "PorousFlow1PhaseP.h"
#include "PorousFlow2PhasePP_VG.h"
#include "PorousFlowMassFraction.h"
#include "PorousFlow1PhaseP_VG.h"
#include "PorousFlow2PhasePS.h"
#include "PorousFlowVariableBase.h"
#include "PorousFlowBrine.h"
#include "PorousFlowCapillaryPressureBase.h"
#include "PorousFlowCapillaryPressureConstant.h"
#include "PorousFlowCapillaryPressureLinear.h"
#include "PorousFlowCapillaryPressureVGS.h"
#include "PorousFlowCapillaryPressureVGP.h"
#include "PorousFlowDensityConstBulk.h"
#include "PorousFlowEffectiveFluidPressure.h"
#include "PorousFlowFluidPropertiesBase.h"
#include "PorousFlowIdealGas.h"
#include "PorousFlowMethane.h"
#include "PorousFlowPermeabilityConst.h"
#include "PorousFlowPermeabilityKozenyCarman.h"
#include "PorousFlowPermeabilityExponential.h"
#include "PorousFlowPorosityConst.h"
#include "PorousFlowPorosityHM.h"
#include "PorousFlowPorosityUnity.h"
#include "PorousFlowRelativePermeabilityCorey.h"
#include "PorousFlowRelativePermeabilityUnity.h"
#include "PorousFlowSimpleCO2.h"
#include "PorousFlowViscosityConst.h"
#include "PorousFlowVolumetricStrain.h"
#include "PorousFlowWater.h"
#include "PorousFlowJoiner.h"
#include "PorousFlowNodeNumber.h"
#include "PorousFlowTemperature.h"
#include "PorousFlowThermalConductivityIdeal.h"
#include "PorousFlowMatrixInternalEnergy.h"
#include "PorousFlowInternalEnergyIdeal.h"
#include "PorousFlowEnthalpy.h"

// Kernels
#include "PorousFlowAdvectiveFlux.h"
#include "PorousFlowMassTimeDerivative.h"
#include "PorousFlowEffectiveStressCoupling.h"
#include "PorousFlowMassVolumetricExpansion.h"
#include "PorousFlowEnergyTimeDerivative.h"
#include "PorousFlowHeatConduction.h"
#include "PorousFlowConvectiveFlux.h"

// BoundaryConditions
#include "PorousFlowSink.h"
#include "PorousFlowPiecewiseLinearSink.h"
#include "PorousFlowHalfGaussianSink.h"
#include "PorousFlowHalfCubicSink.h"

// AuxKernels
#include "PorousFlowDarcyVelocityComponent.h"
#include "PorousFlowPropertyAux.h"

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
  PorousFlowApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  TensorMechanicsApp::associateSyntax(_syntax, _action_factory);
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

  // DiracKernels
  registerDiracKernel(PorousFlowSquarePulsePointSource);

  // Postprocessors
  registerPostprocessor(PorousFlowFluidMass);

  // Materials
  registerMaterial(PorousFlow1PhaseMD_Gaussian);
  registerMaterial(PorousFlow2PhasePP);
  registerMaterial(PorousFlow2PhasePS_VG);
  registerMaterial(PorousFlow1PhaseP);
  registerMaterial(PorousFlow2PhasePP_VG);
  registerMaterial(PorousFlowMassFraction);
  registerMaterial(PorousFlow1PhaseP_VG);
  registerMaterial(PorousFlow2PhasePS);
  registerMaterial(PorousFlowVariableBase);
  registerMaterial(PorousFlowBrine);
  registerMaterial(PorousFlowCapillaryPressureBase);
  registerMaterial(PorousFlowCapillaryPressureConstant);
  registerMaterial(PorousFlowCapillaryPressureLinear);
  registerMaterial(PorousFlowCapillaryPressureVGS);
  registerMaterial(PorousFlowCapillaryPressureVGP);
  registerMaterial(PorousFlowDensityConstBulk);
  registerMaterial(PorousFlowEffectiveFluidPressure);
  registerMaterial(PorousFlowFluidPropertiesBase);
  registerMaterial(PorousFlowIdealGas);
  registerMaterial(PorousFlowMethane);
  registerMaterial(PorousFlowPermeabilityConst);
  registerMaterial(PorousFlowPermeabilityKozenyCarman);
  registerMaterial(PorousFlowPermeabilityExponential);
  registerMaterial(PorousFlowPorosityConst);
  registerMaterial(PorousFlowPorosityHM);
  registerMaterial(PorousFlowPorosityUnity);
  registerMaterial(PorousFlowRelativePermeabilityCorey);
  registerMaterial(PorousFlowRelativePermeabilityUnity);
  registerMaterial(PorousFlowSimpleCO2);
  registerMaterial(PorousFlowViscosityConst);
  registerMaterial(PorousFlowVolumetricStrain);
  registerMaterial(PorousFlowWater);
  registerMaterial(PorousFlowJoiner);
  registerMaterial(PorousFlowNodeNumber);
  registerMaterial(PorousFlowTemperature);
  registerMaterial(PorousFlowThermalConductivityIdeal);
  registerMaterial(PorousFlowMatrixInternalEnergy);
  registerMaterial(PorousFlowInternalEnergyIdeal);
  registerMaterial(PorousFlowEnthalpy);


  // Kernels
  registerKernel(PorousFlowAdvectiveFlux);
  registerKernel(PorousFlowMassTimeDerivative);
  registerKernel(PorousFlowEffectiveStressCoupling);
  registerKernel(PorousFlowMassVolumetricExpansion);
  registerKernel(PorousFlowEnergyTimeDerivative);
  registerKernel(PorousFlowHeatConduction);
  registerKernel(PorousFlowConvectiveFlux);

  // BoundaryConditions
  registerBoundaryCondition(PorousFlowSink);
  registerBoundaryCondition(PorousFlowPiecewiseLinearSink);
  registerBoundaryCondition(PorousFlowHalfGaussianSink);
  registerBoundaryCondition(PorousFlowHalfCubicSink);

  // AuxKernels
  registerAuxKernel(PorousFlowDarcyVelocityComponent);
  registerAuxKernel(PorousFlowPropertyAux);
}

// External entry point for dynamic syntax association
extern "C" void PorousFlowApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory) { PorousFlowApp::associateSyntax(syntax, action_factory); }
void
PorousFlowApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}
