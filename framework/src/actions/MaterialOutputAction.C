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
#include "MaterialOutputAction.h"
#include "FEProblem.h"
#include "MooseApp.h"

// Declare the output helper specializations
template<>
void MaterialOutputAction::materialOutputHelper<Real>(const std::string & material_name, Material * material);

template<>
void MaterialOutputAction::materialOutputHelper<RealVectorValue>(const std::string & material_name, Material * material);

template<>
void MaterialOutputAction::materialOutputHelper<RealTensorValue>(const std::string & material_name, Material * material);


template<>
InputParameters validParams<MaterialOutputAction>()
{
  InputParameters params = validParams<Action>();
  return params;
}

MaterialOutputAction::MaterialOutputAction(const std::string & name, InputParameters params) :
    Action(name, params),
    _output_warehouse(_app.getOutputWarehouse())
{
}

MaterialOutputAction::~MaterialOutputAction()
{
}

void
MaterialOutputAction::act()
{
  // Do nothing if _problem is NULL (this is the case for coupled problems)
  if (_problem == NULL)
  {
    mooseWarning("FEProblem pointer is NULL, if you are executing a coupled problem this is expected. Auto material output is not supported for this case.");
    return;
  }

  // Set the pointers to the MaterialData objects (Note, these pointers are not available at construction)
  _block_material_data = _problem->getMaterialData(0);
  _boundary_material_data = _problem->getBoundaryMaterialData(0);

  // A complete list of all Material objects
  std::vector<Material *> materials = _problem->getMaterialWarehouse(0).getMaterials();

  // Loop through each material object
  for (std::vector<Material *>::iterator material_iter = materials.begin(); material_iter != materials.end(); ++material_iter)
  {
    // Extract the names of the output objects to which the material properties will be exported
    std::set<OutputName> outputs = (*material_iter)->getOutputs();

    // Extract the property names that will actually be output
    std::vector<std::string> output_properties = (*material_iter)->getParam<std::vector<std::string> >("output_properties");

    /* Clear the list of variable names for the current material object, this list will be populated with all the
    variables names for the current material object and is needed for purposes of controlling the which output objects
    show the material property data */
    _material_variable_names.clear();

    // Only continue if the the 'outputs' input parameter is not equal to 'none'
    if (outputs.find("none") == outputs.end())
    {
      // Loop over the material property names
      const std::set<std::string> names = (*material_iter)->getSuppliedItems();
      for (std::set<std::string>::const_iterator name_iter = names.begin(); name_iter != names.end(); ++name_iter)
      {
        // Add the material property for output if the name is contained in the 'output_properties' list or if the list is empty
        if (output_properties.empty() || std::find(output_properties.begin(), output_properties.end(), *name_iter) != output_properties.end())
        {
          if (hasProperty<Real>(*name_iter))
            materialOutputHelper<Real>(*name_iter, *material_iter);

          else if (hasProperty<RealVectorValue>(*name_iter))
            materialOutputHelper<RealVectorValue>(*name_iter, *material_iter);

          else if (hasProperty<RealTensorValue>(*name_iter))
            materialOutputHelper<RealTensorValue>(*name_iter, *material_iter);

          else
            mooseWarning("The type for material property '" << *name_iter << "' is not supported for automatic output.");
        }

        // Update the OutputWarehouse
        /* If 'outputs' is supplied with a list of output objects to limit the output to this information must be communicated
         * to output objects, which is done via the OutputWarehouse */
        if (!outputs.empty())
          _output_warehouse.updateMaterialOutput(outputs, _material_variable_names);
      }
    }
  }

  // Create the AuxVariables
  FEType fe_type(CONSTANT, MONOMIAL); // currently only elemental variables are support for material property output
  for (std::set<AuxVariableName>::iterator it = _variable_names.begin(); it != _variable_names.end(); ++it)
    _problem->addAuxVariable(*it, fe_type);

  // Update the complete list of material related AuxVariables to the OutputWarehouse
  _output_warehouse.setMaterialOutputVariables(_variable_names);
}

MooseObjectAction *
MaterialOutputAction::createAction(const std::string & type, const std::string & property_name,
                                   const std::string & variable_name, Material * material)
{
  // Declare the pointer to be returned by this method
  MooseObjectAction * action;

  // Append the list of variables to create
  _variable_names.insert(variable_name);

  // Append the list of output variables for the current material
  _material_variable_names.insert(variable_name);

  // Generate the name
  std::ostringstream long_name;
  long_name << "AuxKernels/" << material->name() << "_" << variable_name;

  // Set the action parameters
  InputParameters action_params = _action_factory.getValidParams("AddKernelAction");
  action_params.set<std::string>("type") = type;
  action_params.set<ActionWarehouse *>("awh") = &_awh;
  action_params.set<std::string>("registered_identifier") = "(AutoBuilt)";
  action_params.set<std::string>("task") = "add_aux_kernel";

  // Create the action
  action = static_cast<MooseObjectAction *>(_action_factory.create("AddKernelAction", long_name.str(), action_params));

  // Set the object parameters
  InputParameters & object_params = action->getObjectParams();
  object_params.set<std::string>("property") = property_name;
  object_params.set<AuxVariableName>("variable") = variable_name;
  object_params.set<MooseEnum>("execute_on") = "timestep";

  if (material->boundaryRestricted())
    object_params.set<std::vector<BoundaryName> >("boundary") = material->boundaryNames();
  else
    object_params.set<std::vector<SubdomainName> >("block") = material->blocks();

  // Return the created action
  return action;
}

template<>
void
MaterialOutputAction::materialOutputHelper<Real>(const std::string & property_name, Material * material)
{
  MooseObjectAction * action = createAction("MaterialRealAux", property_name, property_name, material);
  _awh.addActionBlock(action);
}

template<>
void
MaterialOutputAction::materialOutputHelper<RealVectorValue>(const std::string & property_name, Material * material)
{
  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
  {
    std::ostringstream oss;
    oss << property_name << "_" << i;

    MooseObjectAction * action = createAction("MaterialRealVectorValueAux", property_name, oss.str(), material);
    action->getObjectParams().set<unsigned int>("component") = i;
    _awh.addActionBlock(action);
  }
}


template<>
void
MaterialOutputAction::materialOutputHelper<RealTensorValue>(const std::string & property_name, Material * material)
{
  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
    {
      std::ostringstream oss;
      oss << property_name << "_" << i << j;

      MooseObjectAction * action = createAction("MaterialRealTensorValueAux", property_name, oss.str(), material);
      action->getObjectParams().set<unsigned int>("row") = i;
      action->getObjectParams().set<unsigned int>("column") = j;
      _awh.addActionBlock(action);
    }
}
