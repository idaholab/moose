/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "AuxKernel.h"

// local includes
#include "FEProblem.h"
#include "SubProblem.h"
#include "AuxiliarySystem.h"
#include "MooseTypes.h"
#include "Assembly.h"

// libmesh includes
#include "libmesh/numeric_vector.h"
#include "libmesh/dof_map.h"
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<AuxKernel>()
{
  InputParameters params = validParams<MooseObject>();
  params += validParams<BlockRestrictable>();
  params += validParams<BoundaryRestrictable>();
  params += validParams<RandomInterface>();
  params += validParams<MeshChangedInterface>();
  params += validParams<MaterialPropertyInterface>();

  // Add the SetupInterface parameter, 'execute_on', the default is 'linear'
  params += validParams<SetupInterface>();

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

  // This flag is set to true if the AuxKernel is being used on a boundary
  params.addPrivateParam<bool>("_on_boundary", false);

  params.declareControllable("enable"); // allows Control to enable/disable this type of object
  params.registerBase("AuxKernel");

  return params;
}

AuxKernel::AuxKernel(const InputParameters & parameters)
  : MooseObject(parameters),
    BlockRestrictable(parameters),
    BoundaryRestrictable(parameters,
                         parameters.get<AuxiliarySystem *>("_aux_sys")
                             ->getVariable(parameters.get<THREAD_ID>("_tid"),
                                           parameters.get<AuxVariableName>("variable"))
                             .isNodal()),
    SetupInterface(this),
    CoupleableMooseVariableDependencyIntermediateInterface(
        this,
        parameters.get<AuxiliarySystem *>("_aux_sys")
            ->getVariable(parameters.get<THREAD_ID>("_tid"),
                          parameters.get<AuxVariableName>("variable"))
            .isNodal()),
    FunctionInterface(this),
    UserObjectInterface(this),
    TransientInterface(this),
    MaterialPropertyInterface(this, blockIDs(), boundaryIDs()),
    PostprocessorInterface(this),
    DependencyResolverInterface(),
    RandomInterface(parameters,
                    *parameters.get<FEProblemBase *>("_fe_problem_base"),
                    parameters.get<THREAD_ID>("_tid"),
                    parameters.get<AuxiliarySystem *>("_aux_sys")
                        ->getVariable(parameters.get<THREAD_ID>("_tid"),
                                      parameters.get<AuxVariableName>("variable"))
                        .isNodal()),
    GeometricSearchInterface(this),
    Restartable(parameters, "AuxKernels"),
    ZeroInterface(parameters),
    MeshChangedInterface(parameters),
    VectorPostprocessorInterface(this),
    _subproblem(*parameters.get<SubProblem *>("_subproblem")),
    _sys(*parameters.get<SystemBase *>("_sys")),
    _nl_sys(*parameters.get<SystemBase *>("_nl_sys")),
    _aux_sys(*parameters.get<AuxiliarySystem *>("_aux_sys")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid)),

    _var(_aux_sys.getVariable(_tid, parameters.get<AuxVariableName>("variable"))),
    _nodal(_var.isNodal()),
    _bnd(boundaryRestricted()),

    _mesh(_subproblem.mesh()),

    _q_point(_bnd ? _assembly.qPointsFace() : _assembly.qPoints()),
    _qrule(_bnd ? _assembly.qRuleFace() : _assembly.qRule()),
    _JxW(_bnd ? _assembly.JxWFace() : _assembly.JxW()),
    _coord(_assembly.coordTransformation()),

    _u(_nodal ? _var.nodalSln() : _var.sln()),
    _u_old(_nodal ? _var.nodalSlnOld() : _var.slnOld()),
    _u_older(_nodal ? _var.nodalSlnOlder() : _var.slnOlder()),
    _test(_var.phi()),

    _current_elem(_var.currentElem()),
    _current_side(_var.currentSide()),
    _current_elem_volume(_assembly.elemVolume()),
    _current_side_volume(_assembly.sideElemVolume()),

    _current_node(_assembly.node()),

    _solution(_aux_sys.solution())
{
  _supplied_vars.insert(parameters.get<AuxVariableName>("variable"));

  std::map<std::string, std::vector<MooseVariable *>> coupled_vars = getCoupledVars();
  for (const auto & it : coupled_vars)
    for (const auto & var : it.second)
      _depend_vars.insert(var->name());
}

AuxKernel::~AuxKernel() {}

const std::set<std::string> &
AuxKernel::getRequestedItems()
{
  return _depend_vars;
}

const std::set<std::string> &
AuxKernel::getSuppliedItems()
{
  return _supplied_vars;
}

const UserObject &
AuxKernel::getUserObjectBase(const std::string & name)
{
  _depend_uo.insert(_pars.get<UserObjectName>(name));
  return UserObjectInterface::getUserObjectBase(name);
}

const PostprocessorValue &
AuxKernel::getPostprocessorValue(const std::string & name)
{
  _depend_uo.insert(_pars.get<PostprocessorName>(name));
  return PostprocessorInterface::getPostprocessorValue(name);
}

const PostprocessorValue &
AuxKernel::getPostprocessorValueByName(const PostprocessorName & name)
{
  _depend_uo.insert(name);
  return PostprocessorInterface::getPostprocessorValueByName(name);
}

const VectorPostprocessorValue &
AuxKernel::getVectorPostprocessorValue(const std::string & name, const std::string & vector_name)
{
  _depend_uo.insert(_pars.get<VectorPostprocessorName>(name));
  return VectorPostprocessorInterface::getVectorPostprocessorValue(name, vector_name);
}

const VectorPostprocessorValue &
AuxKernel::getVectorPostprocessorValueByName(const VectorPostprocessorName & name,
                                             const std::string & vector_name)
{
  _depend_uo.insert(name);
  return VectorPostprocessorInterface::getVectorPostprocessorValueByName(name, vector_name);
}

void
AuxKernel::coupledCallback(const std::string & var_name, bool is_old)
{
  if (is_old)
  {
    std::vector<VariableName> var_names = getParam<std::vector<VariableName>>(var_name);
    for (const auto & name : var_names)
      _depend_vars.erase(name);
  }
}

void
AuxKernel::compute()
{
  precalculateValue();

  if (isNodal()) /* nodal variables */
  {
    if (_var.isNodalDefined())
    {
      _qp = 0;
      Real value = computeValue();
      // update variable data, which is referenced by other kernels, so the value is up-to-date
      _var.setNodalValue(value);
    }
  }
  else /* elemental variables */
  {
    _n_local_dofs = _var.numberOfDofs();

    if (_n_local_dofs == 1) /* p0 */
    {
      Real value = 0;
      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
        value += _JxW[_qp] * _coord[_qp] * computeValue();
      value /= (_bnd ? _current_side_volume : _current_elem_volume);
      // update the variable data refernced by other kernels.
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
          Real t = _JxW[_qp] * _coord[_qp] * _test[i][_qp];
          _local_re(i) += t * computeValue();
          for (unsigned int j = 0; j < _test.size(); j++)
            _local_ke(i, j) += t * _test[j][_qp];
        }

      // mass matrix is always SPD
      _local_sol.resize(_n_local_dofs);
      _local_ke.cholesky_solve(_local_re, _local_sol);

      _var.setNodalValue(_local_sol);
    }
  }
}

bool
AuxKernel::isNodal()
{
  return _nodal;
}

const VariableValue &
AuxKernel::coupledDot(const std::string & var_name, unsigned int comp)
{
  MooseVariable * var = getVar(var_name, comp);
  if (var->kind() == Moose::VAR_AUXILIARY)
    mooseError(
        name(),
        ": Unable to couple time derivative of an auxiliary variable into the auxiliary system.");

  return Coupleable::coupledDot(var_name, comp);
}

const VariableValue &
AuxKernel::coupledDotDu(const std::string & var_name, unsigned int comp)
{
  MooseVariable * var = getVar(var_name, comp);
  if (var->kind() == Moose::VAR_AUXILIARY)
    mooseError(
        name(),
        ": Unable to couple time derivative of an auxiliary variable into the auxiliary system.");

  return Coupleable::coupledDotDu(var_name, comp);
}
