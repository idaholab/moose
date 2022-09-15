//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MortarConstraintBase.h"
#include "FEProblemBase.h"
#include "Assembly.h"
#include "MooseVariableFE.h"

#include "libmesh/string_to_enum.h"

InputParameters
MortarConstraintBase::validParams()
{
  InputParameters params = Constraint::validParams();
  params += MortarConsumerInterface::validParams();
  params += TwoMaterialPropertyInterface::validParams();

  // Whether on a displaced or undisplaced mesh, coupling ghosting will only happen for
  // cross-interface elements
  params.addRelationshipManager("AugmentSparsityOnInterface",
                                Moose::RelationshipManagerType::COUPLING,
                                [](const InputParameters & obj_params, InputParameters & rm_params)
                                {
                                  rm_params.set<bool>("use_displaced_mesh") =
                                      obj_params.get<bool>("use_displaced_mesh");
                                  rm_params.set<BoundaryName>("secondary_boundary") =
                                      obj_params.get<BoundaryName>("secondary_boundary");
                                  rm_params.set<BoundaryName>("primary_boundary") =
                                      obj_params.get<BoundaryName>("primary_boundary");
                                  rm_params.set<SubdomainName>("secondary_subdomain") =
                                      obj_params.get<SubdomainName>("secondary_subdomain");
                                  rm_params.set<SubdomainName>("primary_subdomain") =
                                      obj_params.get<SubdomainName>("primary_subdomain");
                                  rm_params.set<bool>("ghost_higher_d_neighbors") =
                                      obj_params.get<bool>("ghost_higher_d_neighbors");
                                });

  // If the LM is ever a Lagrange variable, we will attempt to obtain its dof indices from the
  // process that owns the node. However, in order for the dof indices to get set for the Lagrange
  // variable, the process that owns the node needs to have local copies of any lower-d elements
  // that have the connected node. Note that the geometric ghosting done here is different than that
  // done by the AugmentSparsityOnInterface RM, even when ghost_point_neighbors is true. The latter
  // ghosts equal-manifold lower-dimensional secondary element point neighbors and their interface
  // couplings. This ghosts lower-dimensional point neighbors of higher-dimensional elements.
  // Neither is guaranteed to be a superset of the other. For instance ghosting of lower-d point
  // neighbors (AugmentSparsityOnInterface with ghost_point_neighbors = true) is only guaranteed to
  // ghost those lower-d point neighbors on *processes that own lower-d elements*. And you may have
  // a process that only owns higher-dimensionsional elements
  //
  // Note that in my experience it is only important for the higher-d lower-d point neighbors to be
  // ghosted when forming sparsity patterns and so I'm putting this here instead of at the
  // MortarConsumerInterface level
  params.addRelationshipManager("GhostHigherDLowerDPointNeighbors",
                                Moose::RelationshipManagerType::GEOMETRIC);

  params.addParam<VariableName>("secondary_variable", "Primal variable on secondary surface.");
  params.addParam<VariableName>(
      "primary_variable",
      "Primal variable on primary surface. If this parameter is not provided then the primary "
      "variable will be initialized to the secondary variable");
  params.makeParamNotRequired<NonlinearVariableName>("variable");
  params.setDocString(
      "variable",
      "The name of the lagrange multiplier variable that this constraint is applied to. This "
      "parameter may not be supplied in the case of using penalty methods for example");
  params.addParam<bool>(
      "compute_primal_residuals", true, "Whether to compute residuals for the primal variable.");
  params.addParam<bool>(
      "compute_lm_residuals", true, "Whether to compute Lagrange Multiplier residuals");
  params.addParam<bool>(
      "compute_scalar_residuals", true, "Whether to compute scalar residuals");
  params.addParam<MooseEnum>(
      "quadrature",
      MooseEnum("DEFAULT FIRST SECOND THIRD FOURTH", "DEFAULT"),
      "Quadrature rule to use on mortar segments. For 2D mortar DEFAULT is recommended. "
      "For 3D mortar, QUAD meshes are integrated using triangle mortar segments. "
      "While DEFAULT quadrature order is typically sufficiently accurate, exact integration of "
      "QUAD mortar faces requires SECOND order quadrature for FIRST variables and FOURTH order "
      "quadrature for SECOND order variables.");
  return params;
}

MortarConstraintBase::MortarConstraintBase(const InputParameters & parameters)
  : Constraint(parameters),
    NeighborCoupleableMooseVariableDependencyIntermediateInterface(this, false, false),
    MortarConsumerInterface(this),
    TwoMaterialPropertyInterface(this, Moose::EMPTY_BLOCK_IDS, getBoundaryIDs()),
    MooseVariableInterface<Real>(this,
                                 true,
                                 isParamValid("variable") ? "variable" : "secondary_variable",
                                 Moose::VarKindType::VAR_NONLINEAR,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _var(isParamValid("variable")
             ? &_subproblem.getStandardVariable(_tid, parameters.getMooseType("variable"))
             : nullptr),
    _secondary_var(
        isParamValid("secondary_variable")
            ? _sys.getActualFieldVariable<Real>(_tid, parameters.getMooseType("secondary_variable"))
            : _sys.getActualFieldVariable<Real>(_tid, parameters.getMooseType("primary_variable"))),
    _primary_var(
        isParamValid("primary_variable")
            ? _sys.getActualFieldVariable<Real>(_tid, parameters.getMooseType("primary_variable"))
            : _secondary_var),

    _compute_primal_residuals(getParam<bool>("compute_primal_residuals")),
    _compute_lm_residuals(!_var ? false : getParam<bool>("compute_lm_residuals")),
    _compute_scalar_residuals(getParam<bool>("compute_scalar_residuals")),
    _test_dummy(),
    _use_dual(_var ? _var->useDual() : false),
    _normals_primary(_assembly.neighborNormals()),
    _tangents(_assembly.tangents()),
    _coord(_assembly.mortarCoordTransformation()),
    _q_point(_assembly.qPointsMortar()),
    _test(_var ? _var->phiLower() : _test_dummy),
    _test_secondary(_secondary_var.phiFace()),
    _test_primary(_primary_var.phiFaceNeighbor()),
    _grad_test_secondary(_secondary_var.gradPhiFace()),
    _grad_test_primary(_primary_var.gradPhiFaceNeighbor()),
    _interior_secondary_elem(_assembly.elem()),
    _interior_primary_elem(_assembly.neighbor()),
    _lower_secondary_elem(_assembly.lowerDElem()),
    _lower_primary_elem(_assembly.neighborLowerDElem()),
    _displaced(getParam<bool>("use_displaced_mesh"))
{
  if (_use_dual)
    _assembly.activateDual();

  // Note parameter is discretization order, we then convert to quadrature order
  const MooseEnum p_order = getParam<MooseEnum>("quadrature");
  // If quadrature not DEFAULT, set mortar qrule
  if (p_order != "DEFAULT")
  {
    Order q_order = static_cast<Order>(2 * Utility::string_to_enum<Order>(p_order) + 1);
    _assembly.setMortarQRule(q_order);
  }

  if (_var)
    addMooseVariableDependency(_var);
  addMooseVariableDependency(&_secondary_var);
  addMooseVariableDependency(&_primary_var);
}

void
MortarConstraintBase::computeResidual()
{
  setNormals();

  if (_compute_primal_residuals)
  {
    // Compute the residual for the secondary interior primal dofs
    computeResidual(Moose::MortarType::Secondary);

    // Compute the residual for the primary interior primal dofs.
    computeResidual(Moose::MortarType::Primary);
  }

  if (_compute_lm_residuals)
    // Compute the residual for the lower dimensional LM dofs (if we even have an LM variable)
    computeResidual(Moose::MortarType::Lower);

  if (_compute_scalar_residuals)
    // Compute the residual for the scalar dofs
    computeResidualScalar();
}

void
MortarConstraintBase::computeJacobian()
{
  setNormals();

  if (_compute_primal_residuals)
  {
    // Compute the jacobian for the secondary interior primal dofs
    computeJacobian(Moose::MortarType::Secondary);

    // Compute the jacobian for the primary interior primal dofs.
    computeJacobian(Moose::MortarType::Primary);
  }

  if (_compute_lm_residuals)
    // Compute the jacobian for the lower dimensional LM dofs (if we even have an LM variable)
    computeJacobian(Moose::MortarType::Lower);

  if (_compute_scalar_residuals)
    // Compute the jacobian for the scalar dofs
    computeJacobianScalar();
}

void
MortarConstraintBase::zeroInactiveLMDofs(const std::unordered_set<const Node *> & inactive_lm_nodes,
                                         const std::unordered_set<const Elem *> & inactive_lm_elems)
{
  // If no LM variable has been defined, skip
  if (!_var)
    return;

  const auto sn = _sys.number();
  const auto vn = _var->number();

  // If variable is nodal, zero DoFs based on inactive LM nodes
  if (_var->isNodal())
  {
    for (const auto node : inactive_lm_nodes)
    {
      // Allow mixed Lagrange orders between primal and LM
      if (!node->n_comp(sn, vn))
        continue;

      const auto dof_index = node->dof_number(sn, vn, 0);
      if (_subproblem.currentlyComputingJacobian() ||
          _subproblem.currentlyComputingResidualAndJacobian())
        _assembly.cacheJacobian(dof_index, dof_index, 1., _matrix_tags);
      if (!_subproblem.currentlyComputingJacobian())
      {
        Real lm_value = _var->getNodalValue(*node);
        _assembly.cacheResidual(dof_index, lm_value, _vector_tags);
      }
    }
  }
  // If variable is elemental, zero based on inactive LM elems
  else
  {
    for (const auto el : inactive_lm_elems)
    {
      const auto n_comp = el->n_comp(sn, vn);

      for (const auto comp : make_range(n_comp))
      {
        const auto dof_index = el->dof_number(sn, vn, comp);
        if (_assembly.computingJacobian())
          _assembly.cacheJacobian(dof_index, dof_index, 1., _matrix_tags);
        if (_assembly.computingResidual())
        {
          const Real lm_value = _var->getElementalValue(el, comp);
          _assembly.cacheResidual(dof_index, lm_value, _vector_tags);
        }
      }
    }
  }
}
