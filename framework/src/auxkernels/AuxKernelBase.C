//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AuxKernelBase.h"

// local includes
#include "FEProblem.h"
#include "SubProblem.h"
#include "AuxiliarySystem.h"
#include "MooseTypes.h"
#include "Assembly.h"

InputParameters
AuxKernelBase::validParams()
{
  InputParameters params = MooseObject::validParams();
  params += BlockRestrictable::validParams();
  params += BoundaryRestrictable::validParams();
  params += RandomInterface::validParams();
  params += MeshChangedInterface::validParams();
  params += MaterialPropertyInterface::validParams();
  params += FunctorInterface::validParams();
  params += GeometricSearchInterface::validParams();

  // Add the SetupInterface parameter 'execute_on' with 'linear' and 'timestep_end'
  params += SetupInterface::validParams();
  ExecFlagEnum & exec_enum = params.set<ExecFlagEnum>("execute_on", true);
  exec_enum.addAvailableFlags(EXEC_PRE_DISPLACE);
  exec_enum = {EXEC_LINEAR, EXEC_TIMESTEP_END};
  params.setDocString("execute_on", exec_enum.getDocString());
  params.addParam<int>(
      "execution_order_group",
      0,
      "Execution order groups are executed in increasing order (e.g., the lowest "
      "number is executed first). Note that negative group numbers may be used to execute groups "
      "before the default (0) group. Please refer to the auxkernel documentation "
      "for ordering of auxkernel execution within a group for example elemental vs nodal or "
      "between scalar, regular, vector and array auxkernels.");
  params.addParamNamesToGroup("execute_on execution_order_group", "Execution scheduling");

  params.addRequiredParam<AuxVariableName>("variable",
                                           "The name of the variable that this object applies to");

  params.addParam<bool>("use_displaced_mesh",
                        false,
                        "Whether or not this object should use the "
                        "displaced mesh for computation.  Note that "
                        "in the case this is true but no "
                        "displacements are provided in the Mesh block "
                        "the undisplaced mesh will still be used.");
  params.addParamNamesToGroup("use_displaced_mesh", "Advanced");
  params.addParam<bool>("check_boundary_restricted",
                        true,
                        "Whether to check for multiple element sides on the boundary "
                        "in the case of a boundary restricted, element aux variable. "
                        "Setting this to false will allow contribution to a single element's "
                        "elemental value(s) from multiple boundary sides on the same element "
                        "(example: when the restricted boundary exists on two or more sides "
                        "of an element, such as at a corner of a mesh");

  params.addRelationshipManager("GhostLowerDElems",
                                Moose::RelationshipManagerType::GEOMETRIC |
                                    Moose::RelationshipManagerType::ALGEBRAIC);

  params.declareControllable("enable"); // allows Control to enable/disable this type of object

  params.registerBase("AuxKernel");

  return params;
}

AuxKernelBase::AuxKernelBase(const InputParameters & parameters)
  : MooseObject(parameters),
    BlockRestrictable(this),
    BoundaryRestrictable(this, getVariableHelper(parameters).isNodal()),
    SetupInterface(this),
    CoupleableMooseVariableDependencyIntermediateInterface(this,
                                                           getVariableHelper(parameters).isNodal()),
    FunctionInterface(this),
    UserObjectInterface(this),
    TransientInterface(this),
    MaterialPropertyInterface(this, blockIDs(), boundaryIDs()),
    PostprocessorInterface(this),
    DependencyResolverInterface(),
    RandomInterface(parameters,
                    *parameters.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base"),
                    parameters.get<THREAD_ID>("_tid"),
                    getVariableHelper(parameters).isNodal()),
    GeometricSearchInterface(this),
    Restartable(this, "AuxKernels"),
    MeshChangedInterface(parameters),
    VectorPostprocessorInterface(this),
    ElementIDInterface(this),
    NonADFunctorInterface(this),

    _var(getVariableHelper(parameters)),
    _bnd(boundaryRestricted()),
    _check_boundary_restricted(getParam<bool>("check_boundary_restricted")),
    _subproblem(*getCheckedPointerParam<SubProblem *>("_subproblem")),
    _sys(*getCheckedPointerParam<SystemBase *>("_sys")),
    _nl_sys(*getCheckedPointerParam<SystemBase *>("_nl_sys")),
    _aux_sys(static_cast<AuxiliarySystem &>(_sys)),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid, 0)),
    _mesh(_subproblem.mesh())
{
  addMooseVariableDependency(&_var);
  _supplied_vars.insert(parameters.get<AuxVariableName>("variable"));

  // Check for supported variable types
  // Any 'nodal' family that actually has DoFs outside of nodes, or gradient dofs at nodes is
  // not properly set by AuxKernelTempl::compute
  // NOTE: We could add a few exceptions, lower order from certain unsupported families and on
  //       certain element types only have value-DoFs on nodes
  const auto type = _var.feType();
  if (_var.isNodal() && !((type.family == LAGRANGE) || (type.order <= FIRST)))
    paramError("variable",
               "Variable family " + Moose::stringify(type.family) + " is not supported at order " +
                   Moose::stringify(type.order) + " by the AuxKernel system.");
}

#ifdef MOOSE_KOKKOS_ENABLED
AuxKernelBase::AuxKernelBase(const AuxKernelBase & object, const Moose::Kokkos::FunctorCopy & key)
  : MooseObject(object, key),
    BlockRestrictable(object, key),
    BoundaryRestrictable(object, key),
    SetupInterface(object, key),
    CoupleableMooseVariableDependencyIntermediateInterface(object, key),
    FunctionInterface(object, key),
    UserObjectInterface(object, key),
    TransientInterface(object, key),
    MaterialPropertyInterface(object, key),
    PostprocessorInterface(object, key),
    DependencyResolverInterface(object, key),
    RandomInterface(object, key),
    GeometricSearchInterface(object, key),
    Restartable(object, key),
    MeshChangedInterface(object, key),
    VectorPostprocessorInterface(object, key),
    ElementIDInterface(object, key),
    NonADFunctorInterface(object, key),

    _var(object._var),
    _bnd(object._bnd),
    _check_boundary_restricted(object._check_boundary_restricted),
    _subproblem(object._subproblem),
    _sys(object._sys),
    _nl_sys(object._nl_sys),
    _aux_sys(object._aux_sys),
    _tid(object._tid),
    _assembly(object._assembly),
    _mesh(object._mesh)
{
}
#endif

void
AuxKernelBase::initialSetup()
{
  // This check must occur after the EquationSystems object has been init'd (due to calls to
  // Elem::n_dofs()) so we can't do it in the constructor
  if (_bnd && !_var.isNodal() && _check_boundary_restricted)
  {
    // when the variable is elemental and this aux kernel operates on boundaries,
    // we need to check that no elements are visited more than once through visiting
    // all the sides on the boundaries
    auto boundaries = _mesh.getMesh().get_boundary_info().build_side_list();
    std::set<dof_id_type> element_ids;
    for (const auto & [elem_id, _, boundary_id] : boundaries)
    {
      if (hasBoundary(boundary_id) && _mesh.elemPtr(elem_id)->n_dofs(_sys.number(), _var.number()))
      {
        const auto [_, inserted] = element_ids.insert(elem_id);
        if (!inserted) // already existed in the set
          mooseError(
              "Boundary restricted auxiliary kernel '",
              name(),
              "' has element (id=",
              elem_id,
              ") connected with more than one boundary sides.\nTo skip this error check, "
              "set 'check_boundary_restricted = false'.\nRefer to the AuxKernel "
              "documentation on boundary restricted aux kernels for understanding this error.");
      }
    }
  }
}

const std::set<std::string> &
AuxKernelBase::getRequestedItems()
{
  return _depend_vars;
}

const std::set<std::string> &
AuxKernelBase::getSuppliedItems()
{
  return _supplied_vars;
}

void
AuxKernelBase::coupledCallback(const std::string & var_name, bool is_old) const
{
  if (!is_old)
  {
    const auto & var_names = getParam<std::vector<VariableName>>(var_name);
    _depend_vars.insert(var_names.begin(), var_names.end());
  }
}

void
AuxKernelBase::addUserObjectDependencyHelper(const UserObject & uo) const
{
  _depend_uo.insert(uo.name());
  for (const auto & indirect_dependent : uo.getDependObjects())
    _depend_uo.insert(indirect_dependent);
}

void
AuxKernelBase::addPostprocessorDependencyHelper(const PostprocessorName & name) const
{
  getUserObjectBaseByName(name); // getting the UO will call addUserObjectDependencyHelper()
}

void
AuxKernelBase::addVectorPostprocessorDependencyHelper(const VectorPostprocessorName & name) const
{
  getUserObjectBaseByName(name); // getting the UO will call addUserObjectDependencyHelper()
}

MooseVariableFieldBase &
AuxKernelBase::getVariableHelper(const InputParameters & parameters)
{
  return parameters.getCheckedPointerParam<SystemBase *>("_sys")->getVariable(
      parameters.get<THREAD_ID>("_tid"), parameters.get<AuxVariableName>("variable"));
}
