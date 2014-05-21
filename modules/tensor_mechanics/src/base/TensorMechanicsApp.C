#include "TensorMechanicsApp.h"
#include "Moose.h"
#include "AppFactory.h"

#include "TensorMechanicsAction.h"
#include "StressDivergenceTensors.h"
#include "MomentBalancing.h"
#include "LinearElasticMaterial.h"
#include "FiniteStrainElasticMaterial.h"
#include "FiniteStrainPlasticMaterial.h"
#include "FiniteStrainRatePlasticMaterial.h"
#include "FiniteStrainCrystalPlasticity.h"
#include "RankTwoAux.h"
#include "RealTensorValueAux.h"
#include "RankFourAux.h"
#include "TensorElasticEnergyAux.h"
#include "FiniteStrainPlasticAux.h"
#include "CrystalPlasticitySlipSysAux.h"
#include "CrystalPlasticityRotationOutAux.h"
#include "CosseratLinearElasticMaterial.h"

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
  registerKernel(MomentBalancing);

  registerMaterial(LinearElasticMaterial);
  registerMaterial(FiniteStrainElasticMaterial);
  registerMaterial(FiniteStrainPlasticMaterial);
  registerMaterial(FiniteStrainRatePlasticMaterial);
  registerMaterial(FiniteStrainCrystalPlasticity);
  registerMaterial(CosseratLinearElasticMaterial);

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
  syntax.registerActionSyntax("TensorMechanicsAction", "TensorMechanics/*");

  registerAction(TensorMechanicsAction, "add_kernel");
}
