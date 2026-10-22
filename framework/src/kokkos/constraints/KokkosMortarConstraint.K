//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosMortarConstraint.h"

#include "AutomaticMortarGeneration.h"
#include "FEProblemBase.h"
#include "MortarUtils.h"

#include "libmesh/fe_map.h"
#include "libmesh/petsc_vector.h"
#include "libmesh/quadrature.h"
#include "libmesh/reference_elem.h"

namespace Moose::Kokkos
{

InputParameters
MortarConstraintBase::validParams()
{
  InputParameters params = ResidualObject::validParams();
  params += MortarConsumerInterface::validParams();
  // The mortar loops integrate every segment of a partially covered secondary element and keep its
  // Lagrange multiplier degrees of freedom active
  params.set<bool>("correct_edge_dropping") = true;
  params.suppressParameter<bool>("correct_edge_dropping");

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
                                  rm_params.set<bool>("ghost_point_neighbors") =
                                      obj_params.get<bool>("ghost_point_neighbors");
                                  rm_params.set<bool>("ghost_higher_d_neighbors") =
                                      obj_params.get<bool>("ghost_higher_d_neighbors");
                                });
  params.addRelationshipManager("GhostHigherDLowerDPointNeighbors",
                                Moose::RelationshipManagerType::GEOMETRIC);

  params.addParam<bool>("compute_lm_residuals",
                        true,
                        "Whether to compute the residuals of the Lagrange multiplier. A constraint "
                        "that only applies the force of the Lagrange multiplier to the primal "
                        "variables leaves its rows to another constraint.");

  params.registerBase("Constraint");

  return params;
}

MortarConstraintBase::MortarConstraintBase(const InputParameters & parameters)
  : ResidualObject(parameters, Moose::VarFieldType::VAR_FIELD_STANDARD),
    CoupleableMooseVariableDependencyIntermediateInterface(this, false),
    _interface(std::make_shared<MortarConsumer>(this)),
    _interface_data(_fe_problem.getKokkosMortarInterfaceData(_interface->amg())),
    _lm_family(_var.feType().family),
    _lm_order(_var.feType().order),
    _use_dual(_var.useDual()),
    _compute_lm_residuals(getParam<bool>("compute_lm_residuals"))
{
  _configuration = Configuration(*this);

  _lm_var.init(_var);
  addMooseVariableDependency(&_var);
  _pinned_lm_vars = {&_var};

  // The Jacobian columns of the displacements that are solution variables of this system
  _n_vars = _sys.nVariables();
  _disp_coupled.create(3 * _n_vars);
  _disp_coupled = false;
  if (_configuration.displaced())
  {
    auto disp = _configuration.displacements();
    for (const auto c : make_range(disp.components()))
    {
      const auto & disp_var = *disp.mooseVar(c);
      if (disp.sys(c) != _sys.number() || disp_var.kind() != Moose::VarKindType::VAR_SOLVER)
        continue;

      if (_n_disp &&
          (disp_var.feType().order != _disp_order || disp_var.feType().family != _disp_family))
        paramError("use_displaced_mesh", "The displacements must share one finite element type.");

      _disp_var[c].init(disp_var);
      _disp_family = disp_var.feType().family;
      _disp_order = disp_var.feType().order;
      for (const auto row : make_range(_n_vars))
        _disp_coupled[_n_vars * c + row] =
            _fe_problem.areCoupled(row, disp_var.number(), _sys.number());
      _n_disp = c + 1;
    }
  }
  _disp_coupled.copyToDevice();
}

MortarConstraintBase::MortarConstraintBase(const MortarConstraintBase & object)
  : ResidualObject(object),
    CoupleableMooseVariableDependencyIntermediateInterface(object, {}),
    _interface(object._interface),
    _interface_data(object._interface_data),
    _geometry(object._geometry),
    _lm_var(object._lm_var),
    _lm_fe(object._lm_fe),
    _lm_family(object._lm_family),
    _lm_order(object._lm_order),
    _use_dual(object._use_dual),
    _n_disp(object._n_disp),
    _disp_fe(object._disp_fe),
    _disp_family(object._disp_family),
    _disp_order(object._disp_order),
    _disp_coupled(object._disp_coupled),
    _n_vars(object._n_vars),
    _computing_residual(object._computing_residual),
    _computing_jacobian(object._computing_jacobian),
    _compute_lm_residuals(object._compute_lm_residuals),
    _inactive_dofs(object._inactive_dofs)
{
  for (const auto c : make_range(3u))
    _disp_var[c] = object._disp_var[c];
}

void
MortarConstraintBase::packInactiveDofs()
{
  const auto & amg = _interface->amg();
  const auto & dof_map = _sys.dofMap();
  const auto & solution =
      static_cast<const libMesh::PetscVector<Number> &>(*_sys.system().current_local_solution);
  const auto sys_num = _sys.number();

  // The non-Kokkos mortar constraints pin the same degrees of freedom in zeroInactiveLMDofs()
  std::vector<dof_id_type> dofs;
  for (const auto var : _pinned_lm_vars)
  {
    const auto var_num = var->number();
    if (var->isNodal())
    {
      for (const auto node : amg.getInactiveLMNodes())
        if (node->processor_id() == processor_id() && node->n_comp(sys_num, var_num))
          dofs.push_back(node->dof_number(sys_num, var_num, 0));
    }
    else
      for (const auto elem : amg.getInactiveLMElems())
        for (const auto comp : make_range(elem->n_comp(sys_num, var_num)))
          dofs.push_back(elem->dof_number(sys_num, var_num, comp));
  }

  _inactive_dofs.create(dofs.size());
  for (const auto i : index_range(dofs))
  {
    mooseAssert(dof_map.local_index(dofs[i]), "Inactive Lagrange multiplier DOF is not local");
    _inactive_dofs[i] = solution.map_global_to_local_index(dofs[i]);
  }
  _inactive_dofs.copyToDevice();
}

void
MortarConstraintBase::dispatch(const bool residual, const bool jacobian)
{
  _geometry = _interface_data.update(jacobian);

  // The FE type IDs exist once the Kokkos assembly is initialized
  _lm_fe = kokkosAssembly().getFETypeID(_var.feType());
  for (const auto c : make_range(_n_disp))
    if (_disp_var[c].initialized())
      _disp_fe = kokkosAssembly().getFETypeID(
          const_cast<Variable &>(_disp_var[c]).mooseFieldVar()->feType());

  if (_compute_lm_residuals && _interface_data.revision() != _inactive_revision)
  {
    packInactiveDofs();
    _inactive_revision = _interface_data.revision();
  }

  _computing_residual = residual;
  _computing_jacobian = jacobian;

  dispatchMortar(_geometry.secondary_qp_begin.size() ? _geometry.secondary_qp_begin.size() - 1 : 0);

  // A process may own inactive degrees of freedom without holding any mortar segment
  if (_compute_lm_residuals && _inactive_dofs.size())
  {
    if (!_inactive_dispatcher)
      _inactive_dispatcher = DispatcherRegistry::build<InactiveLoop>(this, type());

    _inactive_dispatcher->parallelFor(Policy(0, _inactive_dofs.size()));
  }
}

void
MortarConstraintBase::computeResidual()
{
  dispatch(true, false);
}

void
MortarConstraintBase::computeJacobian()
{
  dispatch(false, true);
}

void
MortarConstraintBase::computeResidualAndJacobian()
{
  dispatch(true, true);
}

InputParameters
MortarConstraint::validParams()
{
  InputParameters params = MortarConstraintBase::validParams();

  // The Lagrange multiplier is the variable of the constraint, on the lower-dimensional secondary
  // elements
  params.addRequiredParam<VariableName>("secondary_variable",
                                        "Primal variable on the secondary surface.");
  params.addParam<VariableName>(
      "primary_variable",
      "Primal variable on the primary surface. If this parameter is not provided then the primary "
      "variable is the secondary variable.");

  return params;
}

MortarConstraint::MortarConstraint(const InputParameters & parameters)
  : MortarConstraintBase(parameters),
    _secondary_var(_sys.getVariable(_tid, getParam<VariableName>("secondary_variable"))),
    _primary_var(isParamValid("primary_variable")
                     ? _sys.getVariable(_tid, getParam<VariableName>("primary_variable"))
                     : _secondary_var)
{
  // Sides are indexed by MortarSide: the secondary and primary primal variables and the Lagrange
  // multiplier, all on lower-dimensional elements
  const MooseVariableFieldBase * const vars[3] = {&_secondary_var, &_primary_var, &_var};
  for (const auto side : make_range(3u))
  {
    _side_var[side].init(*vars[side]);
    _side_family[side] = vars[side]->feType().family;
    _side_order[side] = vars[side]->feType().order;
    addMooseVariableDependency(const_cast<MooseVariableFieldBase *>(vars[side]));
  }

  for (const auto row : make_range(3u))
    for (const auto col : make_range(3u))
      _coupled[row][col] =
          _fe_problem.areCoupled(vars[row]->number(), vars[col]->number(), _sys.number());
}

MortarConstraint::MortarConstraint(const MortarConstraint & object)
  : MortarConstraintBase(object),
    _secondary_var(object._secondary_var),
    _primary_var(object._primary_var)
{
  for (const auto side : make_range(3u))
  {
    _side_var[side] = object._side_var[side];
    _side_fe[side] = object._side_fe[side];
    _side_family[side] = object._side_family[side];
    _side_order[side] = object._side_order[side];
    for (const auto col : make_range(3u))
      _coupled[side][col] = object._coupled[side][col];
  }
}

void
MortarConstraint::dispatchMortar(const ThreadID n_secondary)
{
  const MooseVariableFieldBase * const vars[3] = {&_secondary_var, &_primary_var, &_var};
  for (const auto side : make_range(3u))
    _side_fe[side] = kokkosAssembly().getFETypeID(vars[side]->feType());

  if (!n_secondary)
    return;

  if (!_residual_dispatcher)
    _residual_dispatcher = DispatcherRegistry::build<MortarLoop>(this, type());

  TeamLaunch launch;
  launch.league_size = n_secondary;
  _residual_dispatcher->parallelForTeams(launch);
}

} // namespace Moose::Kokkos
