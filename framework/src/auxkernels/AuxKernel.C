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
#include "libmesh/boundary_info.h"

defineLegacyParams(AuxKernel);
defineLegacyParams(VectorAuxKernel);
defineLegacyParams(ArrayAuxKernel);

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
  params += FunctorInterface::validParams();

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
  params.addParam<bool>("check_boundary_restricted",
                        true,
                        "Whether to check for multiple element sides on the boundary "
                        "in the case of a boundary restricted, element aux variable. "
                        "Setting this to false will allow contribution to a single element's "
                        "elemental value(s) from multiple boundary sides on the same element "
                        "(example: when the restricted boundary exists on two or more sides "
                        "of an element, such as at a corner of a mesh");

  // This flag is set to true if the AuxKernelTempl is being used on a boundary
  params.addPrivateParam<bool>("_on_boundary", false);

  params.declareControllable("enable"); // allows Control to enable/disable this type of object
  params.registerBase("AuxKernel");

  if (typeid(AuxKernelTempl<ComputeValueType>).name() == typeid(VectorAuxKernel).name())
    params.registerBase("VectorAuxKernel");
  if (typeid(AuxKernelTempl<ComputeValueType>).name() == typeid(ArrayAuxKernel).name())
    params.registerBase("ArrayAuxKernel");
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
        std::is_same<Real, ComputeValueType>::value
            ? Moose::VarFieldType::VAR_FIELD_STANDARD
            : (std::is_same<RealVectorValue, ComputeValueType>::value
                   ? Moose::VarFieldType::VAR_FIELD_VECTOR
                   : Moose::VarFieldType::VAR_FIELD_ARRAY)),
    BlockRestrictable(this),
    BoundaryRestrictable(this, mooseVariableBase()->isNodal()),
    SetupInterface(this),
    CoupleableMooseVariableDependencyIntermediateInterface(this, mooseVariableBase()->isNodal()),
    FunctionInterface(this),
    UserObjectInterface(this),
    TransientInterface(this),
    MaterialPropertyInterface(this, blockIDs(), boundaryIDs()),
    PostprocessorInterface(this),
    DependencyResolverInterface(),
    RandomInterface(parameters,
                    *parameters.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base"),
                    parameters.get<THREAD_ID>("_tid"),
                    mooseVariableBase()->isNodal()),
    GeometricSearchInterface(this),
    Restartable(this, "AuxKernels"),
    MeshChangedInterface(parameters),
    VectorPostprocessorInterface(this),
    ElementIDInterface(this),
    FunctorInterface(this),
    _check_boundary_restricted(getParam<bool>("check_boundary_restricted")),
    _subproblem(*getCheckedPointerParam<SubProblem *>("_subproblem")),
    _sys(*getCheckedPointerParam<SystemBase *>("_sys")),
    _nl_sys(*getCheckedPointerParam<SystemBase *>("_nl_sys")),
    _aux_sys(*getCheckedPointerParam<AuxiliarySystem *>("_aux_sys")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _var(_aux_sys.getActualFieldVariable<ComputeValueType>(
        _tid, parameters.get<AuxVariableName>("variable"))),
    _nodal(_var.isNodal()),
    _u(_nodal ? _var.nodalValueArray() : _var.sln()),

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
  addMooseVariableDependency(&_var);
  _supplied_vars.insert(parameters.get<AuxVariableName>("variable"));

  const auto & coupled_vars = getCoupledVars();
  for (const auto & it : coupled_vars)
    for (const auto & var : it.second)
      _depend_vars.insert(var->name());

  if (_bnd && !isNodal() && _check_boundary_restricted)
  {
    // when the variable is elemental and this aux kernel operates on boundaries,
    // we need to check that no elements are visited more than once through visiting
    // all the sides on the boundaries
    auto boundaries = _mesh.getMesh().get_boundary_info().build_side_list();
    std::set<dof_id_type> elements;
    for (const auto & t : boundaries)
    {
      if (hasBoundary(std::get<2>(t)))
      {
        const auto eid = std::get<0>(t);
        const auto stat = elements.insert(eid);
        if (!stat.second) // already existed in the set
          mooseError(
              "Boundary restricted auxiliary kernel '",
              name(),
              "' has element (id=",
              eid,
              ") connected with more than one boundary sides.\nTo skip this error check, "
              "set 'check_boundary_restricted = false'.\nRefer to the AuxKernel "
              "documentation on boundary restricted aux kernels for understanding this error.");
      }
    }
  }
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
void
AuxKernelTempl<ComputeValueType>::addUserObjectDependencyHelper(const UserObject & uo) const
{
  _depend_uo.insert(uo.name());
  for (const auto & indirect_dependent : uo.getDependObjects())
    _depend_uo.insert(indirect_dependent);
}

template <typename ComputeValueType>
void
AuxKernelTempl<ComputeValueType>::addPostprocessorDependencyHelper(
    const PostprocessorName & name) const
{
  getUserObjectBaseByName(name); // getting the UO will call addUserObjectDependencyHelper()
}

template <typename ComputeValueType>
void
AuxKernelTempl<ComputeValueType>::addVectorPostprocessorDependencyHelper(
    const VectorPostprocessorName & name) const
{
  getUserObjectBaseByName(name); // getting the UO will call addUserObjectDependencyHelper()
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

template <>
void
AuxKernelTempl<Real>::setDofValueHelper(const Real & value)
{
  mooseAssert(_n_local_dofs == 1,
              "Should only be calling setDofValue if there is one dof for the aux var");
  _var.setDofValue(value, 0);
}

template <>
void
AuxKernelTempl<RealVectorValue>::setDofValueHelper(const RealVectorValue &)
{
  mooseError("Not implemented");
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
      if (_var.isFV())
        setDofValueHelper(value);
      else
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

      // mass matrix is always SPD but in case of boundary restricted, it will be rank deficient
      _local_sol.resize(_n_local_dofs);
      if (_bnd)
        _local_ke.svd_solve(_local_re, _local_sol);
      else
        _local_ke.cholesky_solve(_local_re, _local_sol);

      _var.setDofValues(_local_sol);
    }
  }
}

template <>
void
AuxKernelTempl<RealEigenVector>::compute()
{
  precalculateValue();

  if (isNodal()) /* nodal variables */
  {
    if (_var.isNodalDefined())
    {
      _qp = 0;
      RealEigenVector value = computeValue();
      // update variable data, which is referenced by other kernels, so the value is up-to-date
      _var.setNodalValue(value);
    }
  }
  else /* elemental variables */
  {
    _n_local_dofs = _var.numberOfDofs();
    if (_n_local_dofs == 1) /* p0 */
    {
      RealEigenVector value = RealEigenVector::Zero(_var.count());
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
      for (unsigned int i = 0; i < _local_re.size(); ++i)
        _local_re(i) = RealEigenVector::Zero(_var.count());
      _local_ke.resize(_n_local_dofs, _n_local_dofs);
      _local_ke.zero();

      // assemble the local mass matrix and the load
      for (unsigned int i = 0; i < _test.size(); i++)
        for (_qp = 0; _qp < _qrule->n_points(); _qp++)
        {
          Real t = _JxW[_qp] * _coord[_qp] * _test[i][_qp];
          _local_re(i) += t * computeValue();
          for (unsigned int j = 0; j < _test.size(); j++)
            _local_ke(i, j) += t * _test[j][_qp];
        }

      // mass matrix is always SPD
      _local_sol.resize(_n_local_dofs);
      for (unsigned int i = 0; i < _local_re.size(); ++i)
        _local_sol(i) = RealEigenVector::Zero(_var.count());
      DenseVector<Number> re(_n_local_dofs);
      DenseVector<Number> sol(_n_local_dofs);
      for (unsigned int i = 0; i < _var.count(); ++i)
      {
        for (unsigned int j = 0; j < _n_local_dofs; ++j)
          re(j) = _local_re(j)(i);

        if (_bnd)
          _local_ke.svd_solve(re, sol);
        else
          _local_ke.cholesky_solve(re, sol);

        for (unsigned int j = 0; j < _n_local_dofs; ++j)
          _local_sol(j)(i) = sol(j);
      }

      _var.setDofValues(_local_sol);
    }
  }
}

template <typename ComputeValueType>
const typename OutputTools<ComputeValueType>::VariableValue &
AuxKernelTempl<ComputeValueType>::uOld() const
{
  if (_sys.solutionStatesInitialized())
    mooseError("The solution states have already been initialized when calling ",
               type(),
               "::uOld().\n\n",
               "Make sure to call uOld() within the object constructor.");

  return _nodal ? _var.nodalValueOldArray() : _var.slnOld();
}

template <typename ComputeValueType>
const typename OutputTools<ComputeValueType>::VariableValue &
AuxKernelTempl<ComputeValueType>::uOlder() const
{
  if (_sys.solutionStatesInitialized())
    mooseError("The solution states have already been initialized when calling ",
               type(),
               "::uOlder().\n\n",
               "Make sure to call uOlder() within the object constructor.");

  return _nodal ? _var.nodalValueOlderArray() : _var.slnOlder();
}

// Explicitly instantiates the two versions of the AuxKernelTempl class
template class AuxKernelTempl<Real>;
template class AuxKernelTempl<RealVectorValue>;
template class AuxKernelTempl<RealEigenVector>;
