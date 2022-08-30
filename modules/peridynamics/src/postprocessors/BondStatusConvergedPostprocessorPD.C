//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BondStatusConvergedPostprocessorPD.h"

registerMooseObject("PeridynamicsApp", BondStatusConvergedPostprocessorPD);

InputParameters
BondStatusConvergedPostprocessorPD::validParams()
{
  InputParameters params = ElementPostprocessor::validParams();
  params.addClassDescription(
      "Postprocessor to check whether the bond status is converged within a time step");

  return params;
}

BondStatusConvergedPostprocessorPD::BondStatusConvergedPostprocessorPD(
    const InputParameters & parameters)
  : ElementPostprocessor(parameters),
    _bond_status_var(&_subproblem.getStandardVariable(_tid, "bond_status")),
    _pdmesh(dynamic_cast<PeridynamicsMesh &>(_mesh)),
    _dim(_pdmesh.dimension())
{
}

void
BondStatusConvergedPostprocessorPD::initialize()
{
  _num_bond_status_updated = 0;
}

void
BondStatusConvergedPostprocessorPD::execute()
{
  if ((_bond_status_var->getElementalValueOld(_current_elem) -
       _bond_status_var->getElementalValue(_current_elem)) > 0.5)
    _num_bond_status_updated += 1;
}

Real
BondStatusConvergedPostprocessorPD::getValue()
{
  return _num_bond_status_updated;
}

void
BondStatusConvergedPostprocessorPD::finalize()
{
  gatherSum(_num_bond_status_updated);
}

void
BondStatusConvergedPostprocessorPD::threadJoin(const UserObject & uo)
{
  const BondStatusConvergedPostprocessorPD & pps =
      static_cast<const BondStatusConvergedPostprocessorPD &>(uo);
  _num_bond_status_updated += pps._num_bond_status_updated;
}
