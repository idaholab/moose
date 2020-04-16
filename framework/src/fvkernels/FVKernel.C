
#include "FVKernel.h"
#include "Assembly.h"
#include "SubProblem.h"

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
  : MooseObject(params),
    TaggingInterface(this),
    TransientInterface(this),
    BlockRestrictable(this),
    _subproblem(*getCheckedPointerParam<SubProblem *>("_subproblem")),
    _tid(params.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid))
{
  _subproblem.haveADObjects(true);
  if (getParam<bool>("use_displaced_mesh"))
    paramError("use_displaced_mesh", "FV kernels do not yet support displaced mesh");
}

