//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "AddMFEMCoordinateCoefficientsAction.h"
#include "MFEMCoordinateCoefficients.h"
#include "MFEMProblem.h"

registerMooseAction("MooseApp",
                    AddMFEMCoordinateCoefficientsAction,
                    "add_mfem_coordinate_coefficients");

InputParameters
AddMFEMCoordinateCoefficientsAction::validParams()
{
  return MooseObjectAction::validParams();
}

AddMFEMCoordinateCoefficientsAction::AddMFEMCoordinateCoefficientsAction(
    const InputParameters & params)
  : MooseObjectAction(params)
{
}

void
AddMFEMCoordinateCoefficientsAction::act()
{
  _problem->addUserObject(_type, _name, _moose_object_pars);

  auto & coord = _problem->getUserObject<MFEMCoordinateCoefficients>(_name);
  coord.build();

  if (auto * mfem_problem = dynamic_cast<MFEMProblem *>(_problem.get()))
    mfem_problem->getCoefficients().setBuiltinProvider(&coord);
  else
    mooseError("Coordinates block requires MFEMProblem.");
}

#endif
