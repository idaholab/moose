
#include "FVKernel.h"
#include "Assembly.h"
#include "MooseVariableFV.h"
#include "SubProblem.h"
#include "SystemBase.h"

#include "ComputeFVFluxThread.h"

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
    _subproblem(*getCheckedPointerParam<SubProblem *>("_subproblem"))
{
  if (getParam<bool>("use_displaced_mesh"))
    paramError("use_displaced_mesh", "FV kernels do not yet support displaced mesh");
}

InputParameters
FVFluxKernelBase::validParams()
{
  InputParameters params = FVKernel::validParams();
  params += TwoMaterialPropertyInterface::validParams();
  return params;
}

FVFluxKernelBase::FVFluxKernelBase(const InputParameters & params)
  : FVKernel(params),
    TwoMaterialPropertyInterface(this, blockIDs(), {}),
    NeighborMooseVariableInterface(
        this, false, Moose::VarKindType::VAR_NONLINEAR, Moose::VarFieldType::VAR_FIELD_STANDARD),
    NeighborCoupleableMooseVariableDependencyIntermediateInterface(this, false, false)
{
}

template <ComputeStage compute_stage>
InputParameters
FVFluxKernel<compute_stage>::validParams()
{
  InputParameters params = FVFluxKernelBase::validParams();
  return params;
}

template <ComputeStage compute_stage>
FVFluxKernel<compute_stage>::FVFluxKernel(const InputParameters & params)
  : FVFluxKernelBase(params),
    _var(*mooseVariableFV()),
    _tid(params.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid)),
    _u_left(_var.adSln<compute_stage>()),
    _u_right(_var.adSlnNeighbor<compute_stage>()),
    _grad_u_left(_var.adGradSln<compute_stage>()),
    _grad_u_right(_var.adGradSlnNeighbor<compute_stage>())
{
  addMooseVariableDependency(&_var);
}

template <ComputeStage compute_stage>
void
FVFluxKernel<compute_stage>::computeResidual(const FaceInfo & fi)
{
  _face_info = &fi;
  _normal = fi.normal();
  auto r = MetaPhysicL::raw_value(fi.faceArea() * computeQpResidual());

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

template <>
void
FVFluxKernel<RESIDUAL>::computeJacobian(const FaceInfo & /*fi*/)
{
}

template <ComputeStage compute_stage>
void
FVFluxKernel<compute_stage>::computeJacobian(const FaceInfo & fi)
{
  _face_info = &fi;
  _normal = fi.normal();
  DualReal r = fi.faceArea() * computeQpResidual();

  auto & sys = _subproblem.systemBaseNonlinear();
  unsigned int dofs_per_elem = sys.getMaxVarNDofsPerElem();
  unsigned int var_num = _var.number();
  unsigned int nvars = sys.system().n_vars();

  if (ownLeftElem())
  {
    prepareMatrixTagNeighbor(_assembly, var_num, var_num, Moose::ElementElement);
    _local_ke(0, 0) += r.derivatives()[var_num * dofs_per_elem];
    accumulateTaggedLocalMatrix();

    prepareMatrixTagNeighbor(_assembly, var_num, var_num, Moose::ElementNeighbor);
    _local_ke(0, 0) += r.derivatives()[var_num * dofs_per_elem + nvars * dofs_per_elem];
    accumulateTaggedLocalMatrix();
  }

  if (ownRightElem())
  {
    prepareMatrixTagNeighbor(_assembly, var_num, var_num, Moose::NeighborElement);
    _local_ke(0, 0) += -1 * r.derivatives()[var_num * dofs_per_elem];
    accumulateTaggedLocalMatrix();

    prepareMatrixTagNeighbor(_assembly, var_num, var_num, Moose::NeighborNeighbor);
    _local_ke(0, 0) += -1 * r.derivatives()[var_num * dofs_per_elem + nvars * dofs_per_elem];
    accumulateTaggedLocalMatrix();
  }
}

adBaseClass(FVFluxKernel);
