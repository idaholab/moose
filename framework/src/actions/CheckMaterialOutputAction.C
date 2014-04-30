/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

// MOOSE includes
#include "CheckMaterialOutputAction.h"
#include "Material.h"
#include "MooseApp.h"

template<>
InputParameters validParams<CheckMaterialOutputAction>()
{
  InputParameters params = validParams<Action>();
  return params;
}

CheckMaterialOutputAction::CheckMaterialOutputAction(const std::string & name, InputParameters params) :
  Action(name, params)
{
}

CheckMaterialOutputAction::~CheckMaterialOutputAction()
{
}

void
CheckMaterialOutputAction::act()
{
  // Do nothing if _problem is NULL (this is the case for coupled problems)
  /* Do not produce warning, you will get a warning from MaterialOutputAction */
  if (_problem == NULL)
    return;

  // A complete list of all Material objects
  std::vector<Material *> materials = _problem->getMaterialWarehouse(0).getMaterials();

  // Loop through each material object
  for (std::vector<Material *>::iterator material_iter = materials.begin(); material_iter != materials.end(); ++material_iter)
  {
    // Extract the names of the output objects to which the material properties will be exported
    std::set<OutputName> outputs = (*material_iter)->getOutputs();

    // Check that the outputs exist
    _app.getOutputWarehouse().checkOutputs(outputs);
  }
}
