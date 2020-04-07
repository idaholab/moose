
#include "FVKernel.h"
#include "Assembly.h"
#include "MooseVariableFV.h"

InputParameters
FVKernel::validParams()
{
  InputParameters params = MooseObject::validParams();
  params += TransientInterface::validParams();
  params += BlockRestrictable::validParams();
  params += TaggingInterface::validParams();
  params.addRequiredParam<NonlinearVariableName>(
      "variable", "The name of the variable that this kernel condition applies to");
  params.addParam<bool>("use_displaced_mesh",
                        false,
                        "Whether or not this object should use the "
                        "displaced mesh for computation. Note that in "
                        "the case this is true but no displacements "
                        "are provided in the Mesh block the "
                        "undisplaced mesh will still be used.");
  params.addParamNamesToGroup("use_displaced_mesh", "Advanced");

  params.declareControllable("enable");

  // FV Kernels always need one layer of ghosting.
  params.addRelationshipManager("ElementSideNeighborLayers",
                                Moose::RelationshipManagerType::GEOMETRIC |
                                    Moose::RelationshipManagerType::ALGEBRAIC |
                                    Moose::RelationshipManagerType::COUPLING);

  params.registerBase("FVKernel");

  return params;
}

FVKernel::FVKernel(const InputParameters & params)
  : MooseObject(params), TaggingInterface(this), TransientInterface(this), BlockRestrictable(this)
{
  if (getParam<bool>("use_displaced_mesh"))
    paramError("use_displaced_mesh", "FV kernels do not yet support displaced mesh");
}

InputParameters
FVFluxKernel::validParams()
{
  InputParameters params = FVKernel::validParams();
  params += TwoMaterialPropertyInterface::validParams();
  return params;
}

FVFluxKernel::FVFluxKernel(const InputParameters & params)
  : FVKernel(params),
    TwoMaterialPropertyInterface(this, blockIDs(), {}),
    NeighborMooseVariableInterface(
        this, false, Moose::VarKindType::VAR_NONLINEAR, Moose::VarFieldType::VAR_FIELD_STANDARD),
    NeighborCoupleableMooseVariableDependencyIntermediateInterface(this, false, false),
    _var(*mooseVariableFV()),
    _tid(params.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid)),
    _u_left(_var.sln()),
    _u_right(_var.slnNeighbor()),
    _grad_u_left(_var.gradSln()),
    _grad_u_right(_var.gradSlnNeighbor())
{
}

void
FVFluxKernel::computeResidual(const FaceInfo & fi)
{
  _face_info = &fi;
  _normal = fi.normal();
  auto r = fi.faceArea() * computeQpResidual();

  if (ownLeftElem())
  {
    prepareVectorTag(_assembly, _var.number());
    _local_re(0) = r;
    accumulateTaggedLocalResidual();
  }
  if (ownRightElem())
  {
    prepareVectorTagNeighbor(_assembly, _var.number());
    _local_re(0) = -r;
    accumulateTaggedLocalResidual();
  }
}
