#include "SolidMechanicsModule.h"
#include "Factory.h"
#include "ActionFactory.h"
#include "Parser.h"

// solid_mechanics
#include "CLSHPlasticMaterial.h"
#include "Elastic.h"
#include "ElasticEnergyAux.h"
#include "Gravity.h"
#include "LinearAnisotropicMaterial.h"
#include "LinearIsotropicMaterial.h"
#include "SiLinearIsotropicMaterial.h"
#include "LinearStrainHardening.h"
#include "MaterialTensorAux.h"
#include "MaterialVectorAux.h"
#include "PLC_LSH.h"
#include "PowerLawCreep.h"
#include "PlenumPressure.h"
#include "PlenumPressureAction.h"
#include "Pressure.h"
#include "PressureAction.h"
#include "PLSHPlasticMaterial.h"
#include "SolidMechanicsAction.h"
#include "SolidMechImplicitEuler.h"
#include "StressDivergence.h"
#include "StressDivergenceRZ.h"
#include "LinearGeneralAnisotropicMaterial.h"
#include "DashpotBC.h"

void
Elk::SolidMechanics::registerObjects()
{
  // solid_mechanics
  registerMaterial(CLSHPlasticMaterial);
  registerMaterial(Elastic);
  registerKernel(Gravity);
  registerMaterial(LinearAnisotropicMaterial);
  registerMaterial(LinearIsotropicMaterial);
  registerMaterial(SiLinearIsotropicMaterial);
  registerMaterial(LinearStrainHardening);
  registerAux(MaterialTensorAux);
  registerAux(MaterialVectorAux);
  registerMaterial(PLC_LSH);
  registerMaterial(PLSHPlasticMaterial);
  registerMaterial(PowerLawCreep);
  registerMaterial(LinearGeneralAnisotropicMaterial);

  registerBoundaryCondition(DashpotBC);
  registerBoundaryCondition(PlenumPressure);
  registerAction(PlenumPressureAction, "add_bc");

  registerBoundaryCondition(Pressure);
  registerAction(PressureAction, "add_bc");

  registerKernel(SolidMechImplicitEuler);

  registerAux(ElasticEnergyAux);

  registerKernel(StressDivergence);
  registerKernel(StressDivergenceRZ);
  registerAction(SolidMechanicsAction, "add_kernel");
}

void
Elk::SolidMechanics::associateSyntax()
{
  Moose::syntax.registerActionSyntax("EmptyAction", "BCs/PlenumPressure");
  Moose::syntax.registerActionSyntax("PlenumPressureAction", "BCs/PlenumPressure/*");

  Moose::syntax.registerActionSyntax("EmptyAction", "BCs/Pressure");
  Moose::syntax.registerActionSyntax("PressureAction", "BCs/Pressure/*");
  
  Moose::syntax.registerActionSyntax("SolidMechanicsAction", "SolidMechanics/*");
}
