//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "AddMFEMPeriodicBCs.h"
#include "MFEMMesh.h"
#include "MFEMPeriodicBCs.h"

registerMooseAction("MooseApp", AddMFEMPeriodicBCs, "add_mfem_periodic_bcs");

InputParameters
AddMFEMPeriodicBCs::validParams()
{
  InputParameters params = Action::validParams();
  params.registerBase("AddMFEMPeriodicBCs");
  params.addRequiredParam<std::string>("type", "Method to impose periodic BCs on the MFEM mesh");
  return params;
}

AddMFEMPeriodicBCs::AddMFEMPeriodicBCs(const InputParameters & parameters)
  : MooseObjectAction(parameters)
{
}

void
AddMFEMPeriodicBCs::act()
{
  MFEMMesh * mesh_ptr = dynamic_cast<MFEMMesh *>(_mesh.get());
  mooseAssert(mesh_ptr != nullptr, "Could not cast the mesh pointer into MFEMMesh");

  auto bc = _factory.create<MFEMPeriodicByVector>(_type, "mfemperiodicbc", _moose_object_pars);

  mesh_ptr->registerPeriodicBCs(*bc);
}

#endif
