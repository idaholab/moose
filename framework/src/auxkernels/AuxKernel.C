//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AuxKernel.h"

// local includes
#include "FEProblem.h"
#include "SubProblem.h"
#include "AuxiliarySystem.h"
#include "MooseTypes.h"
#include "Assembly.h"

#include "libmesh/numeric_vector.h"
#include "libmesh/dof_map.h"
#include "libmesh/quadrature.h"

defineLegacyParams(AuxKernel);
defineLegacyParams(VectorAuxKernel);

template <typename ComputeValueType>
InputParameters
AuxKernelTempl<ComputeValueType>::validParams()
{
  InputParameters params = MooseObject::validParams();
  params += BlockRestrictable::validParams();
  params += BoundaryRestrictable::validParams();
  params += RandomInterface::validParams();
  params += MeshChangedInterface::validParams();
  params += MaterialPropertyInterface::validParams();

  // Add the SetupInterface parameter 'execute_on' with 'linear' and 'timestep_end'
  params += SetupInterface::validParams();
  ExecFlagEnum & exec_enum = params.set<ExecFlagEnum>("execute_on", true);
  exec_enum.addAvailableFlags(EXEC_PRE_DISPLACE);
  exec_enum = {EXEC_LINEAR, EXEC_TIMESTEP_END};
  params.setDocString("execute_on", exec_enum.getDocString());

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

  // This flag is set to true if the AuxKernelTempl is being used on a boundary
  params.addPrivateParam<bool>("_on_boundary", false);

  params.declareControllable("enable"); // allows Control to enable/disable this type of object
  params.registerBase("AuxKernel");

  if (typeid(AuxKernelTempl<ComputeValueType>).name() == typeid(VectorAuxKernel).name())
    params.registerBase("VectorAuxKernel");
  return params;
}

template <typename ComputeValueType>
AuxKernelTempl<ComputeValueType>::AuxKernelTempl(const InputParameters & parameters)
  : MooseObject(parameters),
    MooseVariableInterface<ComputeValueType>(
        this,
        parameters.getCheckedPointerParam<AuxiliarySystem *>("_aux_sys")
            ->getVariable(parameters.get<THREAD_ID>("_tid"),
                          parameters.get<AuxVariableName>("variable"))
            .isNodal(),
        "variable",
        Moose::VarKindType::VAR_AUXILIARY,
        std::is_same<Real, ComputeValueType>::value ? Moose::VarFieldType::VAR_FIELD_STANDARD
                                                    : Moose::VarFieldType::VAR_FIELD_VECTOR),
    BlockRestrictable(this),
    BoundaryRestrictable(this, mooseVariable()->isNodal()),
    SetupInterface(this),
    CoupleableMooseVariableDependencyIntermediateInterface(this, mooseVariable()->isNodal()),
    FunctionInterface(this),
    UserObjectInterface(this),
    TransientInterface(this),
    MaterialPropertyInterface(this, blockIDs(), boundaryIDs()),
    PostprocessorInterface(this),
    DependencyResolverInterface(),
    RandomInterface(parameters,
                    *parameters.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base"),
                    parameters.get<THREAD_ID>("_tid"),
                    mooseVariable()->isNodal()),
    GeometricSearchInterface(this),
    Restartable(this, "AuxKernels"),
    MeshChangedInterface(parameters),
    VectorPostprocessorInterface(this),
    ElementIDInterface(this),
    _subproblem(*getCheckedPointerParam<SubProblem *>("_subproblem")),
    _sys(*getCheckedPointerParam<SystemBase *>("_sys")),
    _nl_sys(*getCheckedPointerParam<SystemBase *>("_nl_sys")),
    _aux_sys(*getCheckedPointerParam<AuxiliarySystem *>("_aux_sys")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _var(_aux_sys.getFieldVariable<ComputeValueType>(_tid,
                                                     parameters.get<AuxVariableName>("variable"))),
    _nodal(_var.isNodal()),
    _u(_nodal ? _var.nodalValueArray() : _var.sln()),
    _u_old(_nodal ? _var.nodalValueOldArray() : _var.slnOld()),
    _u_older(_nodal ? _var.nodalValueOlderArray() : _var.slnOlder()),

    _test(_var.phi()),
    _assembly(_subproblem.assembly(_tid)),
    _bnd(boundaryRestricted()),
    _mesh(_subproblem.mesh()),

    _q_point(_bnd ? _assembly.qPointsFace() : _assembly.qPoints()),
    _qrule(_bnd ? _assembly.qRuleFace() : _assembly.qRule()),
    _JxW(_bnd ? _assembly.JxWFace() : _assembly.JxW()),
    _coord(_assembly.coordTransformation()),

    _current_elem(_assembly.elem()),
    _current_side(_assembly.side()),
    _current_elem_volume(_assembly.elemVolume()),
    _current_side_volume(_assembly.sideElemVolume()),

    _current_node(_assembly.node()),
    _current_boundary_id(_assembly.currentBoundaryID()),
    _solution(_aux_sys.solution())
{
  addMooseVariableDependency(mooseVariable());
  _supplied_vars.insert(parameters.get<AuxVariableName>("variable"));

  const auto & coupled_vars = getCoupledVars();
  for (const auto & it : coupled_vars)
    for (const auto & var : it.second)
      _depend_vars.insert(var->name());
}

template <typename ComputeValueType>
const std::set<std::string> &
AuxKernelTempl<ComputeValueType>::getRequestedItems()
{
  return _depend_vars;
}

template <typename ComputeValueType>
const std::set<std::string> &
AuxKernelTempl<ComputeValueType>::getSuppliedItems()
{
  return _supplied_vars;
}

template <typename ComputeValueType>
const UserObject &
AuxKernelTempl<ComputeValueType>::getUserObjectBase(const UserObjectName & name)
{
  return getUserObjectBaseByName(_pars.get<UserObjectName>(name));
}

template <typename ComputeValueType>
const UserObject &
AuxKernelTempl<ComputeValueType>::getUserObjectBaseByName(const UserObjectName & name)
{
  _depend_uo.insert(name);
  auto & uo = UserObjectInterface::getUserObjectBaseByName(name);
  auto indirect_dependents = uo.getDependObjects();
  for (auto & indirect_dependent : indirect_dependents)
    _depend_uo.insert(indirect_dependent);
  return uo;
}

template <typename ComputeValueType>
const PostprocessorValue &
AuxKernelTempl<ComputeValueType>::getPostprocessorValue(const std::string & name,
                                                        unsigned int index)
{
  if (hasPostprocessor(name, index))
    getUserObjectBaseByName(_pars.get<PostprocessorName>(name));
  return PostprocessorInterface::getPostprocessorValue(name);
}

template <typename ComputeValueType>
const PostprocessorValue &
AuxKernelTempl<ComputeValueType>::getPostprocessorValueByName(const PostprocessorName & name)
{
  getUserObjectBaseByName(name);
  return PostprocessorInterface::getPostprocessorValueByName(name);
}

template <typename ComputeValueType>
const VectorPostprocessorValue &
AuxKernelTempl<ComputeValueType>::getVectorPostprocessorValue(const std::string & name,
                                                              const std::string & vector_name)
{
  getUserObjectBaseByName(_pars.get<VectorPostprocessorName>(name));
  return VectorPostprocessorInterface::getVectorPostprocessorValue(name, vector_name);
}

template <typename ComputeValueType>
const VectorPostprocessorValue &
AuxKernelTempl<ComputeValueType>::getVectorPostprocessorValueByName(
    const VectorPostprocessorName & name, const std::string & vector_name)
{
  getUserObjectBaseByName(name);
  return VectorPostprocessorInterface::getVectorPostprocessorValueByName(name, vector_name);
}

template <typename ComputeValueType>
const VectorPostprocessorValue &
AuxKernelTempl<ComputeValueType>::getVectorPostprocessorValue(const std::string & name,
                                                              const std::string & vector_name,
                                                              bool needs_broadcast)
{
  getUserObjectBaseByName(_pars.get<VectorPostprocessorName>(name));
  return VectorPostprocessorInterface::getVectorPostprocessorValue(
      name, vector_name, needs_broadcast);
}

template <typename ComputeValueType>
const VectorPostprocessorValue &
AuxKernelTempl<ComputeValueType>::getVectorPostprocessorValueByName(
    const VectorPostprocessorName & name, const std::string & vector_name, bool needs_broadcast)
{
  getUserObjectBaseByName(name);
  return VectorPostprocessorInterface::getVectorPostprocessorValueByName(
      name, vector_name, needs_broadcast);
}

template <typename ComputeValueType>
const ScatterVectorPostprocessorValue &
AuxKernelTempl<ComputeValueType>::getScatterVectorPostprocessorValue(
    const std::string & name, const std::string & vector_name)
{
  getUserObjectBaseByName(_pars.get<VectorPostprocessorName>(name));
  return VectorPostprocessorInterface::getScatterVectorPostprocessorValue(name, vector_name);
}

template <typename ComputeValueType>
const ScatterVectorPostprocessorValue &
AuxKernelTempl<ComputeValueType>::getScatterVectorPostprocessorValueByName(
    const std::string & name, const std::string & vector_name)
{
  getUserObjectBaseByName(_pars.get<PostprocessorName>(name));
  return VectorPostprocessorInterface::getScatterVectorPostprocessorValueByName(name, vector_name);
}

template <typename ComputeValueType>
void
AuxKernelTempl<ComputeValueType>::coupledCallback(const std::string & var_name, bool is_old) const
{
  if (is_old)
  {
    std::vector<VariableName> var_names = getParam<std::vector<VariableName>>(var_name);
    for (const auto & name : var_names)
      _depend_vars.erase(name);
  }
}

template <typename ComputeValueType>
const VariableValue &
AuxKernelTempl<ComputeValueType>::coupledDot(const std::string & var_name, unsigned int comp) const
{
  auto var = getVar(var_name, comp);
  if (var->kind() == Moose::VAR_AUXILIARY)
    mooseError(
        name(),
        ": Unable to couple time derivative of an auxiliary variable into the auxiliary system.");

  return Coupleable::coupledDot(var_name, comp);
}

template <typename ComputeValueType>
const VariableValue &
AuxKernelTempl<ComputeValueType>::coupledDotDu(const std::string & var_name,
                                               unsigned int comp) const
{
  auto var = getVar(var_name, comp);
  if (var->kind() == Moose::VAR_AUXILIARY)
    mooseError(
        name(),
        ": Unable to couple time derivative of an auxiliary variable into the auxiliary system.");

  return Coupleable::coupledDotDu(var_name, comp);
}

template <typename ComputeValueType>
void
AuxKernelTempl<ComputeValueType>::compute()
{
  precalculateValue();

  if (isNodal()) /* nodal variables */
  {
    if (_var.isNodalDefined())
    {
      _qp = 0;
      ComputeValueType value = computeValue();
      // update variable data, which is referenced by other kernels, so the value is up-to-date
      _var.setNodalValue(value);
    }
  }
  else /* elemental variables */
  {
    _n_local_dofs = _var.numberOfDofs();
    if (_n_local_dofs == 1) /* p0 */
    {
      ComputeValueType value = 0;
      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
        value += _JxW[_qp] * _coord[_qp] * computeValue();
      value /= (_bnd ? _current_side_volume : _current_elem_volume);
      // update the variable data referenced by other kernels.
      // Note that this will update the values at the quadrature points too
      // (because this is an Elemental variable)
      _var.setNodalValue(value);
    }
    else /* high-order */
    {
      _local_re.resize(_n_local_dofs);
      _local_re.zero();
      _local_ke.resize(_n_local_dofs, _n_local_dofs);
      _local_ke.zero();

      // assemble the local mass matrix and the load
      for (unsigned int i = 0; i < _test.size(); i++)
        for (_qp = 0; _qp < _qrule->n_points(); _qp++)
        {
          ComputeValueType t = _JxW[_qp] * _coord[_qp] * _test[i][_qp];
          _local_re(i) += t * computeValue();
          for (unsigned int j = 0; j < _test.size(); j++)
            _local_ke(i, j) += t * _test[j][_qp];
        }

      // mass matrix is always SPD
      _local_sol.resize(_n_local_dofs);
      _local_ke.cholesky_solve(_local_re, _local_sol);

      _var.setDofValues(_local_sol);
    }
  }
}

// Explicitly instantiates the two versions of the AuxKernelTempl class
template class AuxKernelTempl<Real>;
template class AuxKernelTempl<RealVectorValue>;
