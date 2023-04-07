//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVAction.h"

registerMooseAction("NavierStokesApp", NSFVAction, "add_navier_stokes_variables");
registerMooseAction("NavierStokesApp", NSFVAction, "add_navier_stokes_user_objects");
registerMooseAction("NavierStokesApp", NSFVAction, "add_navier_stokes_ics");
registerMooseAction("NavierStokesApp", NSFVAction, "add_navier_stokes_kernels");
registerMooseAction("NavierStokesApp", NSFVAction, "add_navier_stokes_bcs");
registerMooseAction("NavierStokesApp", NSFVAction, "add_material");
registerMooseAction("NavierStokesApp", NSFVAction, "add_navier_stokes_pps");
registerMooseAction("NavierStokesApp", NSFVAction, "add_navier_stokes_materials");
registerMooseAction("NavierStokesApp", NSFVAction, "navier_stokes_check_copy_nodal_vars");
registerMooseAction("NavierStokesApp", NSFVAction, "navier_stokes_copy_nodal_vars");

InputParameters
NSFVAction::validParams()
{
  InputParameters params = NSFVBase<Action>::validParams();

  params.addParam<std::vector<SubdomainName>>(
      "block", "The list of blocks on which NS equations are defined on");

  params.addClassDescription("This class allows us to set up Navier-Stokes equations for porous "
                             "medium or clean fluid flows using incompressible or weakly "
                             "compressible approximations with a finite volume discretization.");

  return params;
}

NSFVAction::NSFVAction(const InputParameters & parameters) : NSFVBase<Action>(parameters) {}

void
NSFVAction::act()
{
  if (_current_task == "add_navier_stokes_variables")
  {
    // Process parameters necessary to handle block-restriction
    processMesh();

    addNSVariables();
  }

  if (_current_task == "add_navier_stokes_user_objects")
    addNSUserObjects();

  if (_current_task == "add_navier_stokes_ics")
    addNSInitialConditions();

  if (_current_task == "add_navier_stokes_kernels")
    addNSKernels();

  if (_current_task == "add_navier_stokes_bcs")
    addNSBoundaryConditions();

  if (_current_task == "add_navier_stokes_materials")
    addNSMaterials();

  if (_current_task == "add_navier_stokes_pps")
    addNSPostprocessors();

  if (_current_task == "navier_stokes_check_copy_nodal_vars")
    checkCopyNSNodalVariables();

  if (_current_task == "navier_stokes_copy_nodal_vars")
    copyNSNodalVariables();
}

void
NSFVAction::addRelationshipManagers(Moose::RelationshipManagerType input_rm_type)
{
  InputParameters params = getGhostParametersForRM();
  addRelationshipManagers(input_rm_type, params);
}
