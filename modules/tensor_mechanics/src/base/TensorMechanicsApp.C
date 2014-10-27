#include "TensorMechanicsApp.h"
#include "Moose.h"
#include "AppFactory.h"

#include "TensorMechanicsAction.h"
#include "StressDivergenceTensors.h"
#include "CosseratStressDivergenceTensors.h"
#include "MomentBalancing.h"
#include "LinearElasticMaterial.h"
#include "FiniteStrainElasticMaterial.h"
#include "FiniteStrainPlasticMaterial.h"
#include "FiniteStrainRatePlasticMaterial.h"
#include "FiniteStrainMohrCoulomb.h"
#include "FiniteStrainCrystalPlasticity.h"
#include "FiniteStrainMultiPlasticity.h"
#include "RankTwoAux.h"
#include "RealTensorValueAux.h"
#include "RankFourAux.h"
#include "TensorElasticEnergyAux.h"
#include "FiniteStrainPlasticAux.h"
#include "CrystalPlasticitySlipSysAux.h"
#include "CrystalPlasticityRotationOutAux.h"
#include "CosseratLinearElasticMaterial.h"

#include "TensorMechanicsPlasticSimpleTester.h"
#include "TensorMechanicsPlasticTensile.h"
#include "TensorMechanicsPlasticTensileExponential.h"
#include "TensorMechanicsPlasticTensileCubic.h"
#include "TensorMechanicsPlasticMohrCoulomb.h"
#include "TensorMechanicsPlasticMohrCoulombExponential.h"
#include "TensorMechanicsPlasticMohrCoulombCubic.h"
#include "TensorMechanicsPlasticWeakPlaneTensile.h"
#include "TensorMechanicsPlasticWeakPlaneTensileExponential.h"
#include "TensorMechanicsPlasticWeakPlaneTensileCubic.h"
#include "TensorMechanicsPlasticWeakPlaneTensileN.h"
#include "TensorMechanicsPlasticWeakPlaneShear.h"
#include "TensorMechanicsPlasticWeakPlaneShearExponential.h"
#include "TensorMechanicsPlasticWeakPlaneShearGaussian.h"
#include "TensorMechanicsPlasticWeakPlaneShearCubic.h"
#include "TensorMechanicsPlasticJ2.h"
#include "TensorMechanicsPlasticJ2Gaussian.h"

template<>
InputParameters validParams<TensorMechanicsApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

TensorMechanicsApp::TensorMechanicsApp(const std::string & name, InputParameters parameters) :
    MooseApp(name, parameters)
{
  srand(processor_id());

  Moose::registerObjects(_factory);
  TensorMechanicsApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  TensorMechanicsApp::associateSyntax(_syntax, _action_factory);
}

TensorMechanicsApp::~TensorMechanicsApp()
{
}

void
TensorMechanicsApp::registerApps()
{
  registerApp(TensorMechanicsApp);
}

void
TensorMechanicsApp::registerObjects(Factory & factory)
{
  registerKernel(StressDivergenceTensors);
  registerKernel(CosseratStressDivergenceTensors);
  registerKernel(MomentBalancing);

  registerMaterial(LinearElasticMaterial);
  registerMaterial(FiniteStrainElasticMaterial);
  registerMaterial(FiniteStrainPlasticMaterial);
  registerMaterial(FiniteStrainMohrCoulomb);
  registerMaterial(FiniteStrainRatePlasticMaterial);
  registerMaterial(FiniteStrainCrystalPlasticity);
  registerMaterial(FiniteStrainMultiPlasticity);
  registerMaterial(CosseratLinearElasticMaterial);

  registerUserObject(TensorMechanicsPlasticSimpleTester);
  registerUserObject(TensorMechanicsPlasticTensile);
  registerUserObject(TensorMechanicsPlasticTensileExponential);
  registerUserObject(TensorMechanicsPlasticTensileCubic);
  registerUserObject(TensorMechanicsPlasticMohrCoulomb);
  registerUserObject(TensorMechanicsPlasticMohrCoulombExponential);
  registerUserObject(TensorMechanicsPlasticMohrCoulombCubic);
  registerUserObject(TensorMechanicsPlasticWeakPlaneTensile);
  registerUserObject(TensorMechanicsPlasticWeakPlaneTensileExponential);
  registerUserObject(TensorMechanicsPlasticWeakPlaneTensileCubic);
  registerUserObject(TensorMechanicsPlasticWeakPlaneTensileN);
  registerUserObject(TensorMechanicsPlasticWeakPlaneShear);
  registerUserObject(TensorMechanicsPlasticWeakPlaneShearExponential);
  registerUserObject(TensorMechanicsPlasticWeakPlaneShearGaussian);
  registerUserObject(TensorMechanicsPlasticWeakPlaneShearCubic);
  registerUserObject(TensorMechanicsPlasticJ2);
  registerUserObject(TensorMechanicsPlasticJ2Gaussian);

  registerAux(RankTwoAux);
  registerAux(RealTensorValueAux);
  registerAux(RankFourAux);
  registerAux(TensorElasticEnergyAux);
  registerAux(FiniteStrainPlasticAux);
  registerAux(CrystalPlasticitySlipSysAux);
  registerAux(CrystalPlasticityRotationOutAux);
}

void
TensorMechanicsApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  syntax.registerActionSyntax("TensorMechanicsAction", "Kernels/TensorMechanics");

  registerAction(TensorMechanicsAction, "add_kernel");
}
