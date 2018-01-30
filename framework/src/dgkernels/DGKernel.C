//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DGKernel.h"
#include "Assembly.h"
#include "MooseVariable.h"
#include "Problem.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "MaterialData.h"
#include "ParallelUniqueId.h"

#include "libmesh/dof_map.h"
#include "libmesh/dense_vector.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/dense_subvector.h"
#include "libmesh/libmesh_common.h"
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<DGKernel>()
{
  InputParameters params = validParams<MooseObject>();
  params += validParams<TwoMaterialPropertyInterface>();
  params += validParams<TransientInterface>();
  params += validParams<BlockRestrictable>();
  params += validParams<BoundaryRestrictable>();
  params += validParams<MeshChangedInterface>();
  params.addRequiredParam<NonlinearVariableName>(
      "variable", "The name of the variable that this boundary condition applies to");
  params.addParam<bool>("use_displaced_mesh",
                        false,
                        "Whether or not this object should use the "
                        "displaced mesh for computation. Note that in "
                        "the case this is true but no displacements "
                        "are provided in the Mesh block the "
                        "undisplaced mesh will still be used.");
  params.addParamNamesToGroup("use_displaced_mesh", "Advanced");

  params.declareControllable("enable");
  params.registerBase("DGKernel");
  params.addParam<std::vector<AuxVariableName>>(
      "save_in",
      "The name of auxiliary variables to save this Kernel's residual contributions to. "
      " Everything about that variable must match everything about this variable (the "
      "type, what blocks it's on, etc.)");
  params.addParam<std::vector<AuxVariableName>>(
      "diag_save_in",
      "The name of auxiliary variables to save this Kernel's diagonal Jacobian "
      "contributions to. Everything about that variable must match everything "
      "about this variable (the type, what blocks it's on, etc.)");

  return params;
}

// Static mutex definitions
Threads::spin_mutex DGKernel::_resid_vars_mutex;
Threads::spin_mutex DGKernel::_jacoby_vars_mutex;

DGKernel::DGKernel(const InputParameters & parameters)
  : MooseObject(parameters),
    BlockRestrictable(this),
    BoundaryRestrictable(this, false), // false for _not_ nodal
    SetupInterface(this),
    TransientInterface(this),
    FunctionInterface(this),
    UserObjectInterface(this),
    NeighborCoupleableMooseVariableDependencyIntermediateInterface(this, false, false),
    TwoMaterialPropertyInterface(this, blockIDs(), boundaryIDs()),
    Restartable(parameters, "DGKernels"),
    MeshChangedInterface(parameters),
    _subproblem(*getCheckedPointerParam<SubProblem *>("_subproblem")),
    _sys(*getCheckedPointerParam<SystemBase *>("_sys")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid)),
    _var(_sys.getVariable(_tid, parameters.get<NonlinearVariableName>("variable"))),
    _mesh(_subproblem.mesh()),

    _current_elem(_assembly.elem()),
    _current_elem_volume(_assembly.elemVolume()),

    _neighbor_elem(_assembly.neighbor()),

    _current_side(_assembly.side()),
    _current_side_elem(_assembly.sideElem()),
    _current_side_volume(_assembly.sideElemVolume()),

    _coord_sys(_assembly.coordSystem()),
    _q_point(_assembly.qPointsFace()),
    _qrule(_assembly.qRuleFace()),
    _JxW(_assembly.JxWFace()),
    _coord(_assembly.coordTransformation()),

    _u(_is_implicit ? _var.sln() : _var.slnOld()),
    _grad_u(_is_implicit ? _var.gradSln() : _var.gradSlnOld()),

    _phi(_assembly.phiFace()),
    _grad_phi(_assembly.gradPhiFace()),

    _test(_var.phiFace()),
    _grad_test(_var.gradPhiFace()),

    _normals(_var.normals()),

    _phi_neighbor(_assembly.phiFaceNeighbor()),
    _grad_phi_neighbor(_assembly.gradPhiFaceNeighbor()),

    _test_neighbor(_var.phiFaceNeighbor()),
    _grad_test_neighbor(_var.gradPhiFaceNeighbor()),

    _u_neighbor(_is_implicit ? _var.slnNeighbor() : _var.slnOldNeighbor()),
    _grad_u_neighbor(_is_implicit ? _var.gradSlnNeighbor() : _var.gradSlnOldNeighbor()),

    _save_in_strings(parameters.get<std::vector<AuxVariableName>>("save_in")),
    _diag_save_in_strings(parameters.get<std::vector<AuxVariableName>>("diag_save_in"))
{
  _save_in.resize(_save_in_strings.size());
  _diag_save_in.resize(_diag_save_in_strings.size());

  for (unsigned int i = 0; i < _save_in_strings.size(); i++)
  {
    MooseVariable * var = &_subproblem.getVariable(_tid, _save_in_strings[i]);

    if (_sys.hasVariable(_save_in_strings[i]))
      mooseError("Trying to use solution variable " + _save_in_strings[i] +
                 " as a save_in variable in " + name());

    if (var->feType() != _var.feType())
      paramError(
          "save_in",
          "saved-in auxiliary variable is incompatible with the object's nonlinear variable: ",
          moose::internal::incompatVarMsg(*var, _var));

    _save_in[i] = var;
    var->sys().addVariableToZeroOnResidual(_save_in_strings[i]);
    addMooseVariableDependency(var);
  }

  _has_save_in = _save_in.size() > 0;

  for (unsigned int i = 0; i < _diag_save_in_strings.size(); i++)
  {
    MooseVariable * var = &_subproblem.getVariable(_tid, _diag_save_in_strings[i]);

    if (_sys.hasVariable(_diag_save_in_strings[i]))
      mooseError("Trying to use solution variable " + _diag_save_in_strings[i] +
                 " as a diag_save_in variable in " + name());

    if (var->feType() != _var.feType())
      paramError(
          "diag_save_in",
          "saved-in auxiliary variable is incompatible with the object's nonlinear variable: ",
          moose::internal::incompatVarMsg(*var, _var));

    _diag_save_in[i] = var;
    var->sys().addVariableToZeroOnJacobian(_diag_save_in_strings[i]);
    addMooseVariableDependency(var);
  }

  _has_diag_save_in = _diag_save_in.size() > 0;
}

DGKernel::~DGKernel() {}

void
DGKernel::computeElemNeighResidual(Moose::DGResidualType type)
{
  bool is_elem;
  if (type == Moose::Element)
    is_elem = true;
  else
    is_elem = false;

  const VariableTestValue & test_space = is_elem ? _test : _test_neighbor;
  DenseVector<Number> & re = is_elem ? _assembly.residualBlock(_var.number())
                                     : _assembly.residualBlockNeighbor(_var.number());
  _local_re.resize(re.size());
  _local_re.zero();

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < test_space.size(); _i++)
      _local_re(_i) += _JxW[_qp] * _coord[_qp] * computeQpResidual(type);

  re += _local_re;

  if (_has_save_in)
  {
    Threads::spin_mutex::scoped_lock lock(_resid_vars_mutex);
    for (const auto & var : _save_in)
    {
      std::vector<dof_id_type> & dof_indices =
          is_elem ? var->dofIndices() : var->dofIndicesNeighbor();
      var->sys().solution().add_vector(_local_re, dof_indices);
    }
  }
}

void
DGKernel::computeResidual()
{
  // Compute the residual for this element
  computeElemNeighResidual(Moose::Element);

  // Compute the residual for the neighbor
  computeElemNeighResidual(Moose::Neighbor);
}

void
DGKernel::computeElemNeighJacobian(Moose::DGJacobianType type)
{
  const VariableTestValue & test_space =
      (type == Moose::ElementElement || type == Moose::ElementNeighbor) ? _test : _test_neighbor;
  const VariableTestValue & loc_phi =
      (type == Moose::ElementElement || type == Moose::NeighborElement) ? _phi : _phi_neighbor;
  DenseMatrix<Number> & Kxx =
      type == Moose::ElementElement
          ? _assembly.jacobianBlock(_var.number(), _var.number())
          : type == Moose::ElementNeighbor
                ? _assembly.jacobianBlockNeighbor(
                      Moose::ElementNeighbor, _var.number(), _var.number())
                : type == Moose::NeighborElement
                      ? _assembly.jacobianBlockNeighbor(
                            Moose::NeighborElement, _var.number(), _var.number())
                      : _assembly.jacobianBlockNeighbor(
                            Moose::NeighborNeighbor, _var.number(), _var.number());
  _local_kxx.resize(Kxx.m(), Kxx.n());
  _local_kxx.zero();

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < test_space.size(); _i++)
      for (_j = 0; _j < loc_phi.size(); _j++)
        _local_kxx(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpJacobian(type);

  Kxx += _local_kxx;

  if (_has_diag_save_in && (type == Moose::ElementElement || type == Moose::NeighborNeighbor))
  {
    unsigned int rows = _local_kxx.m();
    DenseVector<Number> diag(rows);
    for (unsigned int i = 0; i < rows; i++)
      diag(i) = _local_kxx(i, i);

    Threads::spin_mutex::scoped_lock lock(_jacoby_vars_mutex);
    for (const auto & var : _diag_save_in)
    {
      if (type == Moose::ElementElement)
        var->sys().solution().add_vector(diag, var->dofIndices());
      else
        var->sys().solution().add_vector(diag, var->dofIndicesNeighbor());
    }
  }
}
void
DGKernel::computeJacobian()
{
  // Compute element-element Jacobian
  computeElemNeighJacobian(Moose::ElementElement);

  // Compute element-neighbor Jacobian
  computeElemNeighJacobian(Moose::ElementNeighbor);

  // Compute neighbor-element Jacobian
  computeElemNeighJacobian(Moose::NeighborElement);

  // Compute neighbor-neighbor Jacobian
  computeElemNeighJacobian(Moose::NeighborNeighbor);
}

void
DGKernel::computeOffDiagElemNeighJacobian(Moose::DGJacobianType type, unsigned int jvar)
{
  const VariableTestValue & test_space =
      (type == Moose::ElementElement || type == Moose::ElementNeighbor) ? _test : _test_neighbor;
  const VariableTestValue & loc_phi =
      (type == Moose::ElementElement || type == Moose::NeighborElement) ? _phi : _phi_neighbor;
  DenseMatrix<Number> & Kxx =
      type == Moose::ElementElement
          ? _assembly.jacobianBlock(_var.number(), jvar)
          : type == Moose::ElementNeighbor
                ? _assembly.jacobianBlockNeighbor(Moose::ElementNeighbor, _var.number(), jvar)
                : type == Moose::NeighborElement
                      ? _assembly.jacobianBlockNeighbor(Moose::NeighborElement, _var.number(), jvar)
                      : _assembly.jacobianBlockNeighbor(
                            Moose::NeighborNeighbor, _var.number(), jvar);

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < test_space.size(); _i++)
      for (_j = 0; _j < loc_phi.size(); _j++)
        Kxx(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobian(type, jvar);
}

void
DGKernel::computeOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _var.number())
    computeJacobian();
  else
  {
    // Compute element-element Jacobian
    computeOffDiagElemNeighJacobian(Moose::ElementElement, jvar);

    // Compute element-neighbor Jacobian
    computeOffDiagElemNeighJacobian(Moose::ElementNeighbor, jvar);

    // Compute neighbor-element Jacobian
    computeOffDiagElemNeighJacobian(Moose::NeighborElement, jvar);

    // Compute neighbor-neighbor Jacobian
    computeOffDiagElemNeighJacobian(Moose::NeighborNeighbor, jvar);
  }
}

Real
DGKernel::computeQpOffDiagJacobian(Moose::DGJacobianType /*type*/, unsigned int /*jvar*/)
{
  return 0.;
}

MooseVariable &
DGKernel::variable()
{
  return _var;
}

SubProblem &
DGKernel::subProblem()
{
  return _subproblem;
}

const Real &
DGKernel::getNeighborElemVolume()
{
  return _assembly.neighborVolume();
}
