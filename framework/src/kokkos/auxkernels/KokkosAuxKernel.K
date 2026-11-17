//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosAuxKernel.h"

namespace Moose::Kokkos
{

InputParameters
AuxKernel::validParams()
{
  InputParameters params = ::AuxKernelBase::validParams();
  params.addPrivateParam<bool>(MooseBase::kokkos_object_param, true);
  params.makeParamNotRequired<AuxVariableName>("variable");
  params.addParam<std::vector<AuxVariableName>>(
      "variables",
      "The names of the variables that this object applies to, as an alternative to 'variable'. "
      "All variables must have the same FE family and order.");
  // Kokkos aux kernels are not yet migrated to TheWarehouse-based execution_order_group
  // filtering, so this parameter would have no effect; suppress it rather than silently
  // ignoring a user-supplied value.
  params.set<int>("execution_order_group") = 0;
  params.suppressParameter<int>("execution_order_group");
  return params;
}

AuxKernel::AuxKernel(const InputParameters & parameters)
  : ::AuxKernelBase(parameters),
    MeshHolder(*_mesh.getKokkosMesh()),
    AssemblyHolder(_sys.feProblem().kokkosAssembly()),
    FESystemHolder(_sys.feProblem().getKokkosFESystems()),
    _nodal(_var.isNodal()),
    _vars(getMooseVariables()),
    _test(),
    _u(_vars, Moose::SOLUTION_TAG, _nodal),
    _t(TransientInterface::_t),
    _t_old(TransientInterface::_t_old),
    _t_step(TransientInterface::_t_step),
    _dt(TransientInterface::_dt),
    _dt_old(TransientInterface::_dt_old)
{
  if (!_nodal && _bnd)
    mooseError("Boundary restriction of a Kokkos AuxKernel is only supported for nodal variables.");

  auto tag = _sys.feProblem().addVectorTag("parallel_solution", Moose::VECTOR_TAG_SOLUTION);

  _kokkos_var.init(_vars, tag);

  // Kokkos aux kernels currently only consume variable information through finite element bases and
  // quadrature points
  for (auto * const var : getMooseVariableDependencies())
    var->requireQpComputations();
}

AuxKernel::AuxKernel(const AuxKernel & object)
  : ::AuxKernelBase(object, {}),
    MeshHolder(object),
    AssemblyHolder(object),
    FESystemHolder(object),
    _nodal(object._nodal),
    _vars(object._vars),
    _kokkos_var(object._kokkos_var),
    _thread(object._thread),
    _test(object._test),
    _u(object._u),
    _t(object._t),
    _t_old(object._t_old),
    _t_step(object._t_step),
    _dt(object._dt),
    _dt_old(object._dt_old),
    _fe_type(object._fe_type),
    _mass_factors(object._mass_factors),
    _rhs(object._rhs),
    _solution(object._solution)
{
}

std::vector<MooseVariableFieldBase *>
AuxKernel::getMooseVariables()
{
  std::vector<MooseVariableFieldBase *> vars;

  for (const auto & name : getVariableNamesHelper(parameters()))
    vars.push_back(&_sys.getVariable(_tid, name));

  for (const auto i : make_range(std::size_t(1), vars.size()))
    if (vars[0]->feType() != vars[i]->feType())
      paramError(parameters().varParamName(),
                 "For multi-variable Kokkos auxiliary kernels, all variables should have the same "
                 "FE family and order.");

  return vars;
}

void
AuxKernel::initialSetup()
{
  ::AuxKernelBase::initialSetup();

  setupMassFactors(true);
}

void
AuxKernel::meshChanged()
{
  ::AuxKernelBase::meshChanged();

  setupMassFactors(true);
}

void
AuxKernel::meshDisplaced()
{
  ::AuxKernelBase::meshDisplaced();

  setupMassFactors(false);
}

void
AuxKernel::setupMassFactors(const bool rebuild)
{
  if (_nodal)
    return;

  if (rebuild)
  {
    _fe_type = kokkosAssembly().getFETypeID(_var.feType());

    const auto num_elems = kokkosMesh().getNumLocalElements();
    const auto num_comps = _vars.size();

    _mass_factors.create(num_elems);
    _rhs.create(num_elems);
    _solution.create(num_elems);

    for (const auto tid : make_range(numKokkosBlockElements()))
    {
      const auto & info = kokkosMesh().getElementInfo(kokkosBlockElementID(tid));
      const auto n = kokkosAssembly().getNumDofs(info.type, _fe_type);

      _mass_factors.reserve({info.id}, {n * (n + 1) / 2});
      _rhs.reserve({info.id}, {num_comps, n});
      _solution.reserve({info.id}, {num_comps, n});
    }

    _mass_factors.finalize();
    _rhs.finalize();
    _solution.finalize();
  }

  Policy policy(0, numKokkosBlockElements());

  DispatcherRegistry::build<MassMatrixLoop>(this, type())->parallelFor(policy);
}

void
AuxKernel::compute()
{
  if (_nodal)
  {
    _thread.resize(_vars.size(), _bnd ? numKokkosBoundaryNodes() : numKokkosBlockNodes());

    Policy policy(0, _thread.size());

    if (!_node_dispatcher)
      _node_dispatcher = DispatcherRegistry::build<NodeLoop>(this, type());

    _node_dispatcher->parallelFor(policy);
  }
  else
  {
    _thread.resize(_vars.size(), numKokkosBlockElements());

    Policy policy(0, _thread.size());

    if (!_element_dispatcher)
      _element_dispatcher = DispatcherRegistry::build<ElementLoop>(this, type());

    _element_dispatcher->parallelFor(policy);
  }
}

VariableValue
AuxKernel::uOld() const
{
  _var.sys().needSolutionState(1);

  return VariableValue(_vars, Moose::OLD_SOLUTION_TAG, isNodal());
}

VariableValue
AuxKernel::uOlder() const
{
  _var.sys().needSolutionState(2);

  return VariableValue(_vars, Moose::OLDER_SOLUTION_TAG, isNodal());
}

void
AuxKernel::getKokkosMaterialPropertyHook(const std::string & prop_name_in,
                                         const unsigned int /* state */)
{
  if (isNodal())
    mooseError("Nodal KokkosAuxKernel '",
               name(),
               "' attempted to reference material property '",
               prop_name_in,
               "'\nConsider using an elemental auxiliary variable for '",
               _var.name(),
               "'.");
}

} // namespace Moose::Kokkos
