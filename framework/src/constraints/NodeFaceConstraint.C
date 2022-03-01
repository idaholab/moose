//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodeFaceConstraint.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseEnum.h"
#include "MooseMesh.h"
#include "MooseVariableFE.h"
#include "PenetrationLocator.h"
#include "SystemBase.h"

#include "libmesh/string_to_enum.h"

InputParameters
NodeFaceConstraint::validParams()
{
  MooseEnum orders("FIRST SECOND THIRD FOURTH", "FIRST");
  InputParameters params = Constraint::validParams();
  params.addParam<BoundaryName>("secondary", "The boundary ID associated with the secondary side");
  params.addParam<BoundaryName>("primary", "The boundary ID associated with the primary side");
  params.addParam<Real>("tangential_tolerance",
                        "Tangential distance to extend edges of contact surfaces");
  params.addParam<Real>(
      "normal_smoothing_distance",
      "Distance from edge in parametric coordinates over which to smooth contact normal");
  params.addParam<std::string>("normal_smoothing_method",
                               "Method to use to smooth normals (edge_based|nodal_normal_based)");
  params.addParam<MooseEnum>("order", orders, "The finite element order used for projections");

  params.addCoupledVar("primary_variable", "The variable on the primary side of the domain");

  return params;
}

NodeFaceConstraint::NodeFaceConstraint(const InputParameters & parameters)
  : Constraint(parameters),
    // The secondary side is at nodes (hence passing 'true').  The neighbor side is the primary side
    // and it is not at nodes (so passing false)
    NeighborCoupleableMooseVariableDependencyIntermediateInterface(this, true, false),
    NeighborMooseVariableInterface<Real>(
        this, true, Moose::VarKindType::VAR_NONLINEAR, Moose::VarFieldType::VAR_FIELD_STANDARD),
    _secondary(_mesh.getBoundaryID(getParam<BoundaryName>("secondary"))),
    _primary(_mesh.getBoundaryID(getParam<BoundaryName>("primary"))),
    _var(_sys.getFieldVariable<Real>(_tid, parameters.get<NonlinearVariableName>("variable"))),

    _primary_q_point(_assembly.qPointsFace()),
    _primary_qrule(_assembly.qRuleFace()),

    _penetration_locator(
        getPenetrationLocator(getParam<BoundaryName>("primary"),
                              getParam<BoundaryName>("secondary"),
                              Utility::string_to_enum<Order>(getParam<MooseEnum>("order")))),

    _current_node(_var.node()),
    _current_primary(_var.neighbor()),
    _u_secondary(_var.dofValues()),
    _phi_secondary(1),  // One entry
    _test_secondary(1), // One entry

    _primary_var(*getVar("primary_variable", 0)),
    _primary_var_num(_primary_var.number()),

    _phi_primary(_assembly.phiFaceNeighbor(_primary_var)),
    _grad_phi_primary(_assembly.gradPhiFaceNeighbor(_primary_var)),

    _test_primary(_var.phiFaceNeighbor()),
    _grad_test_primary(_var.gradPhiFaceNeighbor()),

    _u_primary(_primary_var.slnNeighbor()),
    _grad_u_primary(_primary_var.gradSlnNeighbor()),

    _dof_map(_sys.dofMap()),
    _node_to_elem_map(_mesh.nodeToElemMap()),

    _overwrite_secondary_residual(true),
    _primary_JxW(_assembly.JxWNeighbor())
{
  addMooseVariableDependency(&_var);

  if (parameters.isParamValid("tangential_tolerance"))
  {
    _penetration_locator.setTangentialTolerance(getParam<Real>("tangential_tolerance"));
  }
  if (parameters.isParamValid("normal_smoothing_distance"))
  {
    _penetration_locator.setNormalSmoothingDistance(getParam<Real>("normal_smoothing_distance"));
  }
  if (parameters.isParamValid("normal_smoothing_method"))
  {
    _penetration_locator.setNormalSmoothingMethod(
        parameters.get<std::string>("normal_smoothing_method"));
  }
  // Put a "1" into test_secondary
  // will always only have one entry that is 1
  _test_secondary[0].push_back(1);
}

NodeFaceConstraint::~NodeFaceConstraint()
{
  _phi_secondary.release();
  _test_secondary.release();
}

void
NodeFaceConstraint::computeSecondaryValue(NumericVector<Number> & current_solution)
{
  const dof_id_type & dof_idx = _var.nodalDofIndex();
  _qp = 0;
  current_solution.set(dof_idx, computeQpSecondaryValue());
}

void
NodeFaceConstraint::residualSetup()
{
  _secondary_residual_computed = false;
}

Real
NodeFaceConstraint::secondaryResidual() const
{
  mooseAssert(_secondary_residual_computed,
              "The secondary residual has not yet been computed, so the value will be garbage!");
  return _secondary_residual;
}

void
NodeFaceConstraint::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.number());
  DenseVector<Number> & neighbor_re = _assembly.residualBlockNeighbor(_primary_var.number());

  _qp = 0;

  for (_i = 0; _i < _test_primary.size(); _i++)
    neighbor_re(_i) += computeQpResidual(Moose::Primary);

  _i = 0;
  _secondary_residual = re(0) = computeQpResidual(Moose::Secondary);
  _secondary_residual_computed = true;
}

void
NodeFaceConstraint::computeJacobian()
{
  getConnectedDofIndices(_var.number());

  // Just do a direct assignment here because the Jacobian coming from assembly has already been
  // properly sized according to the neighbor _var dof indices. It has also been zeroed
  _Ken = _assembly.jacobianBlockNeighbor(Moose::ElementNeighbor, _var.number(), _var.number());

  //  DenseMatrix<Number> & Kne = _assembly.jacobianBlockNeighbor(Moose::NeighborElement,
  //  _var.number(), _var.number());
  DenseMatrix<Number> & Knn = _assembly.jacobianBlockNeighbor(
      Moose::NeighborNeighbor, _primary_var.number(), _var.number());

  _Kee.resize(_test_secondary.size(), _connected_dof_indices.size());
  _Kne.resize(_test_primary.size(), _connected_dof_indices.size());

  _phi_secondary.resize(_connected_dof_indices.size());

  _qp = 0;

  // Fill up _phi_secondary so that it is 1 when j corresponds to this dof and 0 for every other dof
  // This corresponds to evaluating all of the connected shape functions at _this_ node
  for (unsigned int j = 0; j < _connected_dof_indices.size(); j++)
  {
    _phi_secondary[j].resize(1);

    if (_connected_dof_indices[j] == _var.nodalDofIndex())
      _phi_secondary[j][_qp] = 1.0;
    else
      _phi_secondary[j][_qp] = 0.0;
  }

  for (_i = 0; _i < _test_secondary.size(); _i++)
    // Loop over the connected dof indices so we can get all the jacobian contributions
    for (_j = 0; _j < _connected_dof_indices.size(); _j++)
      _Kee(_i, _j) += computeQpJacobian(Moose::SecondarySecondary);

  if (_Ken.m() && _Ken.n())
    for (_i = 0; _i < _test_secondary.size(); _i++)
      for (_j = 0; _j < _phi_primary.size(); _j++)
        _Ken(_i, _j) += computeQpJacobian(Moose::SecondaryPrimary);

  for (_i = 0; _i < _test_primary.size(); _i++)
    // Loop over the connected dof indices so we can get all the jacobian contributions
    for (_j = 0; _j < _connected_dof_indices.size(); _j++)
      _Kne(_i, _j) += computeQpJacobian(Moose::PrimarySecondary);

  if (Knn.m() && Knn.n())
    for (_i = 0; _i < _test_primary.size(); _i++)
      for (_j = 0; _j < _phi_primary.size(); _j++)
        Knn(_i, _j) += computeQpJacobian(Moose::PrimaryPrimary);
}

void
NodeFaceConstraint::computeOffDiagJacobian(const unsigned int jvar_num)
{
  getConnectedDofIndices(jvar_num);

  _Kee.resize(_test_secondary.size(), _connected_dof_indices.size());
  _Kne.resize(_test_primary.size(), _connected_dof_indices.size());

  // Just do a direct assignment here because the Jacobian coming from assembly has already been
  // properly sized according to the jvar neighbor dof indices. It has also been zeroed
  _Ken = _assembly.jacobianBlockNeighbor(Moose::ElementNeighbor, _var.number(), jvar_num);

  DenseMatrix<Number> & Knn =
      _assembly.jacobianBlockNeighbor(Moose::NeighborNeighbor, _primary_var.number(), jvar_num);

  _phi_secondary.resize(_connected_dof_indices.size());

  _qp = 0;

  auto primary_jsize = getVariable(jvar_num).dofIndicesNeighbor().size();

  // Fill up _phi_secondary so that it is 1 when j corresponds to this dof and 0 for every other dof
  // This corresponds to evaluating all of the connected shape functions at _this_ node
  for (unsigned int j = 0; j < _connected_dof_indices.size(); j++)
  {
    _phi_secondary[j].resize(1);

    if (_connected_dof_indices[j] == _var.nodalDofIndex())
      _phi_secondary[j][_qp] = 1.0;
    else
      _phi_secondary[j][_qp] = 0.0;
  }

  for (_i = 0; _i < _test_secondary.size(); _i++)
    // Loop over the connected dof indices so we can get all the jacobian contributions
    for (_j = 0; _j < _connected_dof_indices.size(); _j++)
      _Kee(_i, _j) += computeQpOffDiagJacobian(Moose::SecondarySecondary, jvar_num);

  for (_i = 0; _i < _test_secondary.size(); _i++)
    for (_j = 0; _j < primary_jsize; _j++)
      _Ken(_i, _j) += computeQpOffDiagJacobian(Moose::SecondaryPrimary, jvar_num);

  if (_Kne.m() && _Kne.n())
    for (_i = 0; _i < _test_primary.size(); _i++)
      // Loop over the connected dof indices so we can get all the jacobian contributions
      for (_j = 0; _j < _connected_dof_indices.size(); _j++)
        _Kne(_i, _j) += computeQpOffDiagJacobian(Moose::PrimarySecondary, jvar_num);

  for (_i = 0; _i < _test_primary.size(); _i++)
    for (_j = 0; _j < primary_jsize; _j++)
      Knn(_i, _j) += computeQpOffDiagJacobian(Moose::PrimaryPrimary, jvar_num);
}

void
NodeFaceConstraint::getConnectedDofIndices(unsigned int var_num)
{
  MooseVariableFEBase & var = _sys.getVariable(0, var_num);

  _connected_dof_indices.clear();
  std::set<dof_id_type> unique_dof_indices;

  auto node_to_elem_pair = _node_to_elem_map.find(_current_node->id());
  mooseAssert(node_to_elem_pair != _node_to_elem_map.end(), "Missing entry in node to elem map");
  const std::vector<dof_id_type> & elems = node_to_elem_pair->second;

  // Get the dof indices from each elem connected to the node
  for (const auto & cur_elem : elems)
  {
    std::vector<dof_id_type> dof_indices;

    var.getDofIndices(_mesh.elemPtr(cur_elem), dof_indices);

    for (const auto & dof : dof_indices)
      unique_dof_indices.insert(dof);
  }

  for (const auto & dof : unique_dof_indices)
    _connected_dof_indices.push_back(dof);
}

bool
NodeFaceConstraint::overwriteSecondaryResidual()
{
  return _overwrite_secondary_residual;
}
