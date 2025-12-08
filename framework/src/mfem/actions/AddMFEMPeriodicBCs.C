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

registerMooseAction("MooseApp", AddMFEMPeriodicBCs, "add_mfem_periodic_bcs");
registerMooseObject("MooseApp", MFEMPeriodicByVector);

InputParameters
MFEMPeriodicByVector::validParams()
{
  InputParameters params = MooseObject::validParams();
  params.addParam<std::string>("type", "MFEMPeriodicByVector", "dummy string");
  return params;
}

MFEMPeriodicByVector::MFEMPeriodicByVector(const InputParameters& parameters)
  : MooseObject(parameters) {}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

InputParameters
AddMFEMPeriodicBCs::validParams()
{
  InputParameters params = Action::validParams();
  params.addParam<std::string>("type", "MFEMPeriodicByVector", "dummy string");
  return params;
}

AddMFEMPeriodicBCs::AddMFEMPeriodicBCs(const InputParameters & parameters)
  : MooseObjectAction(parameters)
{
}

void
AddMFEMPeriodicBCs::act()
{
  // can we convert the moose mesh into mfemmesh?
  bool success = ( dynamic_cast<MFEMMesh*>(_mesh.get()) != nullptr );

  if (success) std::cout << "Got a successful cast!\n";
}

#endif
