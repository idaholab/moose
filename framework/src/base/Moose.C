#include "Moose.h"
#include "Factory.h"
#include "ImplicitSystem.h"
#include "PetscSupport.h"

// objects that can be created by MOOSE
#include "TimeDerivative.h"
#include "Diffusion.h"
#include "CoupledForce.h"
#include "UserForcingFunction.h"
#include "BodyForce.h"
#include "ImplicitEuler.h"
// bcs
#include "DirichletBC.h"
#include "NeumannBC.h"
#include "FunctionDirichletBC.h"
#include "FunctionNeumannBC.h"
#include "MatchedValueBC.h"
// auxkernels
#include "CoupledAux.h"
#include "ConstantAux.h"
#include "FunctionAux.h"
#include "NearestNodeDistanceAux.h"
#include "NearestNodeValueAux.h"
#include "PenetrationAux.h"
#include "ProcessorIDAux.h"
// ics
#include "ConstantIC.h"
#include "BoundingBoxIC.h"
#include "FunctionIC.h"
#include "RandomIC.h"
// mesh modifiers
#include "ElementDeleter.h"
// executioners
#include "Steady.h"
#include "Transient.h"
#include "LooseCoupling.h"
// functions
#include "ParsedFunction.h"
#include "ParsedGradFunction.h"
#include "PiecewiseLinear.h"
#include "SolutionFunction.h"
#include "SphereFunction.h"
// materials
#include "GenericConstantMaterial.h"
// PPS
#include "AverageElementSize.h"
#include "AverageNodalVariableValue.h"
#include "ElementAverageValue.h"
#include "ElementH1Error.h"
#include "ElementH1SemiError.h"
#include "ElementIntegral.h"
#include "ElementL2Error.h"
#include "NodalVariableValue.h"
#include "PrintDOFs.h"
#include "PrintDT.h"
#include "PrintNumElems.h"
#include "PrintNumNodes.h"
#include "Reporter.h"
#include "SideAverageValue.h"
#include "SideFluxIntegral.h"
#include "SideIntegral.h"
// stabilizers
#include "ConvectionDiffusionSUPG.h"

namespace Moose {

static bool registered = false;

void
registerObjects()
{
  if (registered)
    return;

  registerObject(TimeDerivative);

  // kernels
  registerObject(Diffusion);
  registerObject(CoupledForce);
  registerObject(UserForcingFunction);
  registerObject(BodyForce);
  registerObject(ImplicitEuler);
  // bcs
  registerObject(DirichletBC);
  registerObject(NeumannBC);
  registerObject(FunctionDirichletBC);
  registerObject(FunctionNeumannBC);
  registerObject(MatchedValueBC);
  // aux kernels
  registerObject(CoupledAux);
  registerObject(ConstantAux);
  registerObject(FunctionAux);
  registerObject(NearestNodeDistanceAux);
  registerObject(NearestNodeValueAux);
  registerObject(PenetrationAux);
  registerObject(ProcessorIDAux);
  // Initial Conditions
  registerObject(ConstantIC);
  registerObject(BoundingBoxIC);
  registerObject(FunctionIC);
  registerObject(RandomIC);
  // Mesh Modifiers
  registerObject(ElementDeleter);
  // executioners
  registerObject(Steady);
  registerObject(Transient);
  registerObject(LooseCoupling);
  // functions
  registerObject(ParsedFunction);
  registerObject(ParsedGradFunction);
  registerObject(PiecewiseLinear);
  registerObject(SolutionFunction);
  registerObject(SphereFunction);
  // materials
  registerObject(GenericConstantMaterial);
  // PPS
  registerObject(AverageElementSize);
  registerObject(AverageNodalVariableValue);
  registerObject(ElementAverageValue);
  registerObject(ElementH1Error);
  registerObject(ElementH1SemiError);
  registerObject(ElementIntegral);
  registerObject(ElementL2Error);
  registerObject(NodalVariableValue);
  registerObject(PrintDOFs);
  registerObject(PrintDT);
  registerObject(PrintNumElems);
  registerObject(PrintNumNodes);
  registerObject(Reporter);
  registerObject(SideAverageValue);
  registerObject(SideFluxIntegral);
  registerObject(SideIntegral);
  // stabilizers
  registerObject(ConvectionDiffusionSUPG);

  registered = true;
}

void
setSolverDefaults(ImplicitSystem & system)
{
#ifdef LIBMESH_HAVE_PETSC
  Moose::PetscSupport::petscSetDefaults(system);
#endif //LIBMESH_HAVE_PETSC
}

} // namespace
