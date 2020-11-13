//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "HitOutput.h"
#include "InputParameterWarehouse.h"
#include "FEProblemBase.h"
#include "Conversion.h"
#include "parse.h"

#include "SetupMeshAction.h"
#include "SetAdaptivityOptionsAction.h"
#include "AddPeriodicBCAction.h"
//#include "CreateProblemAction.h"
//#include "AddVariableAction.h"
//#include "AddAuxVariableAction.h"

registerMooseObjectAliased("MooseApp", Hitoutput, "HIT");

InputParameters
Hitoutput::validParams()
{
  InputParameters params = FileOutput::validParams();
  params.set<ExecFlagEnum>("execute_on") = EXEC_INITIAL;
  params.suppressParameter<ExecFlagEnum>("execute_on");
  return params;
}

Hitoutput::Hitoutput(const InputParameters & parameters) : FileOutput(parameters) {}

std::string
Hitoutput::filename()
{
  return _file_base + ".i";
}

void
Hitoutput::output(const ExecFlagType & /*type*/)
{
  // Create a unique set of InputParameter objects
  InputParameterWarehouse & warehouse = _app.getInputParameterWarehouse();
  std::set<std::shared_ptr<InputParameters>> parameters;
  for (auto & map_pair : warehouse.getInputParameters())
    parameters.insert(map_pair.second);

  // Loop over all the InputParameter objects and add the parameters to the hit tree
  std::unique_ptr<hit::Section> root = libmesh_make_unique<hit::Section>("");
  for (const std::shared_ptr<InputParameters> params : parameters)
    addInputParameters(root.get(), params);

  // It is possible to loop over actions and get the parameters for each action and task, but
  // the purpose of this object is to explode the objects and not rely on the actions. The following
  // calls are as minimal as possible to re-run the input file.
  addActionSyntax<SetupMeshAction>("Mesh", root.get(), {"uniform_refine"});
  addActionSyntax<SetAdaptivityOptionsAction>("Adaptivity", root.get());
  addActionSyntax<AddPeriodicBCAction>("BCs/Periodic", root.get(), {}, /*create_sections=*/true);

  // addActionSyntax<CreateProblemAction>("Problem", root.get());
  // addActionSyntax<AddVariableAction>("Variables", root.get(), /*create_sections = */ true);
  // addActionSyntax<AddAuxVariableAction>("AuxVariables", root.get(), /*create_sections = */ true);

  // Explode and write the tree to a file
  hit::Node * node = hit::explode(root.get());
  std::ofstream outfile;
  outfile.open(filename());
  outfile << node->render() << std::endl;
  outfile.close();
}

void
Hitoutput::addInputParameters(hit::Node * root, std::shared_ptr<InputParameters> params)
{
  // Skip if _disable_moose_hit_syntax is set
  if (params->isParamValid("_disable_moose_hit_syntax") &&
      params->get<bool>("_disable_moose_hit_syntax"))
    return;

  if (!params->isParamValid("_moose_hit_syntax"))
  {
    mooseWarning("Only objects that call InputParameters::registerBase with both the system name "
                 "and default input syntax can be output from the input exploder.\n",
                 *params);
    return;
  }

  if (!params->isParamValid("_object_name"))
  {
    // This should not be reachable unless someone calls this method manually
    mooseWarning("Only objects that are added to the InputParameterWarehouse via the "
                 "addInputParameters method can be output from the input exploder.\n",
                 *params);
    return;
  }

  if (!params->isParamValid("_type"))
  {
    mooseWarning("Only objects that are created using Factory::create can be output from the input "
                 "exploder.\n",
                 *params);
    return;
  }

  // Get necessary information for the HIT node from parameters
  const std::string & name = params->get<std::string>("_object_name");
  const std::string & syntax = params->get<std::string>("_moose_hit_syntax");
  const std::string & type = params->get<std::string>("_type");

  // Build the HIT node name
  const std::size_t star_pos = syntax.rfind("*");
  const bool sub_block = (star_pos != std::string::npos);
  std::string hit_name = sub_block ? syntax.substr(0, star_pos) + name : syntax;

  // Create HIT node
  hit::Section * node = new hit::Section(hit_name);
  node->addChild(new hit::Field("type", hit::Field::Kind::String, type));
  for (const auto & param_map_pair : *params)
    addParameter(param_map_pair.first, param_map_pair.second, node, *params);
  root->addChild(node);
}

void
Hitoutput::addParameter(const std::string & name,
                        libMesh::Parameters::Value * value,
                        hit::Node * parent,
                        const InputParameters & parameters)
{
  if (!parameters.isPrivate(name) && parameters.isParamSetByUser(name))
  {
    if (name[0] == '_')
      mooseWarning("The parameter '", name, "' begins with and underscore but it is not private.");

    std::string param_string; // the parameter string to output

    // Special case for bool parameters
    auto param_bool = dynamic_cast<InputParameters::Parameter<bool> *>(value);
    auto param_realvec = dynamic_cast<InputParameters::Parameter<RealVectorValue> *>(value);
    auto param_point = dynamic_cast<InputParameters::Parameter<Point> *>(value);
    if (param_bool)
      param_string = param_bool->get() ? "true" : "false";

    else if (param_realvec)
    {
      std::stringstream ss;
      param_realvec->get().write_unformatted(ss);
      param_string = MooseUtils::trim(ss.str());
    }

    else if (param_point)
    {
      std::stringstream ss;
      ss << param_point->get()(0) << " " << param_point->get()(1) << " " << param_point->get()(2);
      param_string = ss.str();
    }

    else
    {
      std::stringstream ss;
      value->print(ss);
      param_string = MooseUtils::trim(ss.str());
    }

    // Create the parameter node
    if (!param_string.empty())
    {
      // If spaces exists add quotes
      if (param_string.find_first_of(" ") != std::string::npos)
        param_string = "'" + param_string + "'";

      hit::Field * field = new hit::Field(name, hit::Field::Kind::String, param_string);
      parent->addChild(field);
    }
  }
}
