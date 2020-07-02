//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalNumIntactBondsPD.h"
#include "AuxiliarySystem.h"

registerMooseObject("PeridynamicsApp", NodalNumIntactBondsPD);

InputParameters
NodalNumIntactBondsPD::validParams()
{
  InputParameters params = NodalAuxVariableUserObjectBasePD::validParams();
  params.addClassDescription("Class for computing number of intact bonds for each material point "
                             "in peridynamic fracture modeling and simulation");

  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};

  return params;
}

NodalNumIntactBondsPD::NodalNumIntactBondsPD(const InputParameters & parameters)
  : NodalAuxVariableUserObjectBasePD(parameters)
{
}

void
NodalNumIntactBondsPD::computeValue(unsigned int /*id*/, dof_id_type dof)
{
  if (_bond_status_var->getElementalValue(_current_elem) > 0.5)
    _aux.solution().add(dof, 1.0);
}
