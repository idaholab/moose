//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MortarExecutorInterface.h"
#include "FEProblemBase.h"
#include "MortarInterfaceWarehouse.h"

MortarExecutorInterface::MortarExecutorInterface(FEProblemBase & fe_problem)
  : _mortar_warehouse(fe_problem.mortarData())
{
  _mortar_warehouse.notifyWhenMortarSetup(this);
}

MortarExecutorInterface::MortarExecutorInterface(MortarExecutorInterface && other)
  : _mortar_warehouse(other._mortar_warehouse),
    _secondary_ip_sub_to_mats(std::move(other._secondary_ip_sub_to_mats)),
    _primary_ip_sub_to_mats(std::move(other._primary_ip_sub_to_mats)),
    _secondary_boundary_mats(std::move(other._secondary_boundary_mats))
{
  // Swap which object the warehouse calls back into; the moved-from object is about to be
  // destroyed and must not receive further mortarSetup() notifications.
  _mortar_warehouse.dontNotifyWhenMortarSetup(&other);
  _mortar_warehouse.notifyWhenMortarSetup(this);
}
