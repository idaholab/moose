//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosMaterial.h"

namespace Moose::Kokkos
{

InputParameters
Material::validParams()
{
  InputParameters params = MaterialBase::validParams();
  params += MaterialPropertyInterface::validParams();
  params.addParamNamesToGroup("use_displaced_mesh", "Advanced");
  return params;
}

Material::Material(const InputParameters & parameters)
  : MaterialBase(parameters),
    Coupleable(this, false),
    MaterialPropertyInterface(this, blockIDs(), boundaryIDs()),
    _bnd(_material_data_type != Moose::BLOCK_MATERIAL_DATA),
    _neighbor(_material_data_type == Moose::NEIGHBOR_MATERIAL_DATA),
    _qrule(_bnd ? (_neighbor ? _subproblem.assembly(_tid, 0).qRuleNeighbor()
                             : _subproblem.assembly(_tid, 0).qRuleFace())
                : _subproblem.assembly(_tid, 0).qRule())
{
  for (auto coupled_var : getCoupledMooseVars())
    addMooseVariableDependency(coupled_var);
}

Material::Material(const Material & object)
  : MaterialBase(object),
    Coupleable(object, {}),
    MaterialPropertyInterface(object, {}),
    _bnd(object._bnd),
    _neighbor(object._neighbor),
    _qrule(object._qrule)
{
}

void
Material::initStatefulProperties(unsigned int)
{
  if ((!_bnd && !_neighbor) || _constant_option == PropertyConstantOption::SUBDOMAIN)
  {
    if (!DispatcherRegistry::hasUserMethod<ElementInit>(type()))
      return;

    Policy policy(0, numKokkosElements());

    if (!_init_dispatcher)
      _init_dispatcher = DispatcherRegistry::build<ElementInit>(this, type());

    _init_dispatcher->parallelFor(policy);
  }
  else if (_bnd && !_neighbor)
  {
    if (!DispatcherRegistry::hasUserMethod<SideInit>(type()))
      return;

    Policy policy(0, numKokkosElementSides());

    if (!_init_dispatcher)
      _init_dispatcher = DispatcherRegistry::build<SideInit>(this, type());

    _init_dispatcher->parallelFor(policy);
  }
  else
  {
    if (!DispatcherRegistry::hasUserMethod<NeighborInit>(type()))
      return;

    Policy policy(0, numKokkosElementSides());

    if (!_init_dispatcher)
      _init_dispatcher = DispatcherRegistry::build<NeighborInit>(this, type());

    _init_dispatcher->parallelFor(policy);
  }
}

void
Material::computeProperties()
{
  if ((!_bnd && !_neighbor) || _constant_option == PropertyConstantOption::SUBDOMAIN)
  {
    Policy policy(0, numKokkosElements());

    if (!_compute_dispatcher)
      _compute_dispatcher = DispatcherRegistry::build<ElementCompute>(this, type());

    _compute_dispatcher->parallelFor(policy);
  }
  else if (_bnd && !_neighbor)
  {
    Policy policy(0, numKokkosElementSides());

    if (!_compute_dispatcher)
      _compute_dispatcher = DispatcherRegistry::build<SideCompute>(this, type());

    _compute_dispatcher->parallelFor(policy);
  }
  else
  {
    Policy policy(0, numKokkosElementSides());

    if (!_compute_dispatcher)
      _compute_dispatcher = DispatcherRegistry::build<NeighborCompute>(this, type());

    _compute_dispatcher->parallelFor(policy);
  }
}

void
Material::getKokkosMaterialPropertyHook(const std::string & prop_name_in, const unsigned int state)
{
  MaterialBase::checkExecutionStage();

  const auto prop_name =
      _get_suffix.empty()
          ? prop_name_in
          : MooseUtils::join(std::vector<std::string>({prop_name_in, _get_suffix}), "_");

  if (state == 0)
    _requested_props.insert(prop_name);

  registerPropName(prop_name, true, state);
}

} // namespace Moose::Kokkos
