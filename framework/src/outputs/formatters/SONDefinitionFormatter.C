//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SONDefinitionFormatter.h"
#include "MooseUtils.h"
#include "pcrecpp.h"
#include <algorithm>

SONDefinitionFormatter::SONDefinitionFormatter() : _spaces(2), _level(0) {}

// ******************************** toString ************************************ //
// traverse the associated types array of cpp_types and absolute lookup paths and //
// transform the paths to work with our parsed hierarchy and store pairs in a map //
// of types to paths for use by InputChoices rule and store the global/parameters //
// object then add root blocks recursively and return constructed stream's string //
// ****************************************************************************** //
std::string
SONDefinitionFormatter::toString(const nlohmann::json & root)
{

  const std::map<std::string, std::string> json_path_regex_replacement_map = {
      {"/star/subblock_types/([A-Za-z0-9_]*)/", "/\\1_type/"},
      {"[A-Za-z0-9_]*/types/([A-Za-z0-9_]*)/", "\\1_type/"},
      {"/actions/[A-Za-z0-9_]*/parameters/", "/"},
      {"/parameters/", "/"},
      {"/subblocks/", "/"}};

  for (const auto & el : root["global"]["associated_types"].items())
  {
    const auto & type = el.key();
    for (const auto & el_path : el.value().items())
    {
      std::string path = el_path.value();
      for (const auto & map_iter : json_path_regex_replacement_map)
        pcrecpp::RE(map_iter.first).GlobalReplace(map_iter.second, &path);
      _assoc_types_map[type].push_back(path);
    }
  }

  _global_params = root["global"]["parameters"];
  _stream.clear();
  _stream.str("");
  for (const auto & el : root["blocks"].items())
    addBlock(el.key(), el.value());
  return _stream.str();
}

// ******************************** addLine ************************************* //
// add a single new-line-terminated and indented line to the stream               //
// ****************************************************************************** //
void
SONDefinitionFormatter::addLine(const std::string & line)
{
  _stream << line << "\n";
  return;
}

// ******************************* addBlock ************************************* //
// add parameters and recursively add NormalBlock children and TypeBlock children //
// ****************************************************************************** //
void
SONDefinitionFormatter::addBlock(const std::string & block_name,
                                 const nlohmann::json & block,
                                 bool is_typeblock,
                                 const std::string & parent_name,
                                 const nlohmann::json & parameters_in,
                                 const nlohmann::json & subblocks_in)
{
  // open block with "_type" appended to the name if this is a TypeBlock because the
  // parser appends "_type" to the name of blocks with a "type=" parameter specified
  addLine("'" + block_name + (is_typeblock ? "_type" : "") + "'{");
  _level++;

  // decide the actual block [./declarator] name that will be specified later unless
  // this is a StarBlock and then decide if this is a StarBlock or not for later use
  //  - if TypeBlock   : this will be the parent block name
  //  - if NormalBlock : this will be this block name
  std::string block_decl = (is_typeblock ? parent_name : block_name);
  bool is_starblock = (block_decl == "*" ? true : false);

  // - add InputTmpl    : the autocomplete template that is used for all block types
  // - add InputName    : if is_typeblock then this will be dropped in after "type="
  // - add InputType    : block type - normal_top / normal_sub / type_top / type_sub
  // - add InputDefault : block [./declarator] name from above that will be used for
  //                      autocompletion of this block unless it is a StarBlock then
  //                      [./insert_name_here] will be used because any name is okay
  addLine("InputTmpl=MooseBlock");
  addLine("InputName=\"" + block_name + "\"");
  if (!is_typeblock)
    addLine(_level == 1 ? "InputType=normal_top" : "InputType=normal_sub");
  else
    addLine(_level == 1 ? "InputType=type_top" : "InputType=type_sub");
  if (!is_starblock)
    addLine("InputDefault=\"" + block_decl + "\"");
  else
    addLine("InputDefault=\"insert_name_here\"");

  // add Description of block if it exists
  std::string description = block.contains("description") ? block["description"] : "";
  pcrecpp::RE("\"").GlobalReplace("'", &description);
  pcrecpp::RE("[\r\n]").GlobalReplace(" ", &description);
  if (!description.empty())
    addLine("Description=\"" + description + "\"");

  // ensure every block has no more than one string declarator node and if this is a
  // TypeBlock but not a StarBlock then also ensure that the block [./declarator] is
  // the expected block_decl from above which should be the name of the parent block
  addLine("decl{");
  _level++;
  addLine("MaxOccurs=1");
  if (is_typeblock && !is_starblock)
    addLine("ValEnums=[ \"" + block_decl + "\" ]");
  _level--;
  addLine("}");

  // if this block is the GlobalParams block then add a add "*/value" level
  if (block_name == "GlobalParams")
  {
    addLine("'*'{");
    _level++;
    addLine("'value'{");
    addLine("}");
    _level--;
    addLine("}");
  }

  // store parameters ---
  // first  : start with global parameters as a base
  // second : add or overwrite with any parameter inheritance
  // third  : add or overwrite with any local RegularParameters
  // fourth : add or overwrite with any local ActionParameters
  nlohmann::json parameters = _global_params;
  for (const auto & el : parameters_in.items())
    parameters[el.key()] = el.value();
  if (block.contains("parameters"))
  {
    for (const auto & el : block["parameters"].items())
      parameters[el.key()] = el.value();
  }

  if (block.contains("actions"))
  {
    for (const auto & el : block["actions"].items())
      if (el.value().contains("parameters"))
        for (const auto & param_el : el.value()["parameters"].items())
          parameters[param_el.key()] = param_el.value();
  }

  // store NormalBlock children ---
  // first  : start with any NormalBlock inheritance passed in as a base
  // second : add or overwrite these with any local NormalBlock children
  // third  : add star named child block if it exists
  nlohmann::json subblocks = subblocks_in;
  if (block.contains("subblocks"))
  {
    for (const auto & el : block["subblocks"].items())
      subblocks[el.key()] = el.value();
  }
  if (block.contains("star"))
    subblocks["*"] = block["star"];

  // store TypeBlock children ---
  // first  : start with ["types"] child block as a base
  // second : add ["subblock_types"] child block
  nlohmann::json typeblocks = block.contains("types") ? block["types"] : nlohmann::json();
  if (block.contains("subblock_types"))
    for (const auto & el : block["subblock_types"].items())
      typeblocks[el.key()] = el.value();

  // add parameters ---
  // if this block has a "type=" parameter with a specified default "type=" name and
  // if that default is also the name of a saved TypeBlock child then the parameters
  // belonging to that default saved TypeBlock child are added to this block as well
  // first  : start with default saved TypeBlock child's RegularParameters as a base
  // second : add or overwrite with default saved TypeBlock child's ActionParameters
  // third  : add or overwrite with parameters that were stored above for this block
  // fourth : either add newly stored parameters or add previously stored parameters
  if (parameters.contains("type") && parameters["type"].contains("default") &&
      parameters["type"]["default"].is_string() &&
      typeblocks.contains(parameters["type"]["default"].get<std::string>()))
  {
    std::string type_default = parameters["type"]["default"].get<std::string>();
    const nlohmann::json & default_block = typeblocks[type_default];
    if (default_block.contains("parameters"))
    {
      nlohmann::json default_child_params = default_block["parameters"];
      if (default_block.contains("actions"))
      {
        const nlohmann::json & default_actions = default_block["actions"];
        for (const auto & el : default_actions.items())
        {
          if (el.value().contains("parameters"))
            for (const auto & param_el : el.value()["parameters"].items())
              default_child_params[param_el.key()] = param_el.value();
        }
      }

      // unrequire the 'file' parameter added to the Mesh via the FileMesh TypeBlock
      // since MeshGenerators internally change the default block type from FileMesh
      if (block_name == "Mesh" && default_child_params.contains("file") &&
          default_child_params["file"].contains("required") &&
          default_child_params["file"]["required"].is_boolean())
        default_child_params["file"]["required"] = false;

      for (const auto & el : parameters.items())
        default_child_params[el.key()] = el.value();
      addParameters(default_child_params);
    }
  }
  else
    addParameters(parameters);

  // add previously stored NormalBlocks children recursively
  for (const auto & el : subblocks.items())
    addBlock(el.key(), el.value());

  // close block now because the parser stores TypeBlock children at this same level
  _level--;
  addLine("} % end block " + block_name + (is_typeblock ? "_type" : ""));

  // add all previously stored TypeBlock children recursively and pass the parameter
  // and NormalBlock children added at this level in as inheritance to all TypeBlock
  // children so that they may each also add them and pass in the name of this block
  // as well so that all TypeBlock children can add a rule ensuring that their block
  // [./declarator] is the name of this parent block unless this block is named star
  for (const auto & el : typeblocks.items())
    addBlock(el.key(), el.value(), true, block_name, parameters, subblocks);
}

// ***************************** addParameters ********************************** //
// add all of the information for each parameter of a block
// - parameter         :: add ChildAtLeastOne
// - parameter         :: add InputTmpl
// - parameter         :: add InputType
// - parameter         :: add InputName
// - parameter         :: add Description
// - parameter         :: add MinOccurs
// - parameter         :: add MaxOccurs
// - parameter's value :: add MinOccurs
// - parameter's value :: add MaxOccurs
// - parameter's value :: add ValType
// - parameter's value :: add ValEnums
// - parameter's value :: add InputChoices (options)
// - parameter's value :: add InputChoices (lookups)
// - parameter's value :: add MinValInc
// - parameter's value :: add InputDefault
// ****************************************************************************** //
void
SONDefinitionFormatter::addParameters(const nlohmann::json & params)
{

  // build list of any '_object_params_set_by_action' that are not required in input
  std::vector<std::string> action_set_params;
  if (params.contains("_object_params_set_by_action") &&
      params["_object_params_set_by_action"].contains("default"))
  {
    std::string opsba = nlohmann::to_string(params["_object_params_set_by_action"]["default"]);
    if (opsba.front() == '"' && opsba.back() == '"')
    {
      opsba.erase(opsba.begin());
      opsba.pop_back();
    }
    action_set_params = MooseUtils::split(MooseUtils::trim(opsba), " ");
  }

  for (const auto & el : params.items())
  {
    auto & name = el.key();
    auto & param = el.value();

    // skip '_object_params_set_by_action' parameters because they will not be input
    if (name == "_object_params_set_by_action")
      continue;

    // lambda to calculate relative path from the current level to the document root
    auto backtrack = [](int level)
    {
      std::string backtrack_path;
      for (int i = 0; i < level; ++i)
        backtrack_path += "../";
      return backtrack_path;
    };

    // capture the cpp_type and basic_type and strip off any unnecessary information
    std::string cpp_type = param["cpp_type"];
    std::string basic_type = param["basic_type"];
    bool is_array = false;
    if (cpp_type == "FunctionExpression" || cpp_type == "FunctionName" ||
        basic_type.compare(0, 6, "Array:") == 0 || cpp_type.compare(0, 13, "Eigen::Matrix") == 0)
      is_array = true;
    pcrecpp::RE(".+<([A-Za-z0-9_' ':]*)>.*").GlobalReplace("\\1", &cpp_type);
    pcrecpp::RE("(Array:)*(.*)").GlobalReplace("\\2", &basic_type);

    // *** ChildAtLeastOne of parameter
    // if parameter is required, not action set, and no default exists, then specify
    //   ChildAtLeastOne = [ "backtrack/GlobalParams/name/value" "name/value" ]
    auto def_ptr = param.find("default");
    std::string def;
    if (def_ptr != param.end())
      def = def_ptr->is_string() ? def_ptr->get<std::string>() : nlohmann::to_string(*def_ptr);
    def = MooseUtils::trim(def);
    if (param.contains("required") &&
        std::find(action_set_params.begin(), action_set_params.end(), name) ==
            action_set_params.end())
    {
      bool required = param["required"];
      if (required && def.empty())
        addLine("ChildAtLeastOne=[ \"" + backtrack(_level) + "GlobalParams/" + name +
                "/value\" \"" + name + "\" ]");
    }

    // *** open parameter
    addLine("'" + name + "'" + "{");
    _level++;

    // *** InputTmpl of parameter
    addLine("InputTmpl=MooseParam");

    // *** InputType of parameter
    if (is_array)
      addLine("InputType=key_array");
    else
      addLine("InputType=key_value");

    // *** InputName of parameter
    addLine("InputName=\"" + name + "\"");

    // *** Description of parameter
    if (param.contains("description"))
    {
      std::string description = param["description"];
      pcrecpp::RE("\"").GlobalReplace("'", &description);
      pcrecpp::RE("[\r\n]").GlobalReplace(" ", &description);
      if (!description.empty())
        addLine("Description=\"" + description + "\"");
    }

    // *** MaxOccurs=1 for each parameter
    addLine("MaxOccurs=1");

    // *** open parameter's value
    addLine("'value'{");
    _level++;

    // *** MinOccurs / MaxOccurs of parameter's value
    // is_array indicates the parameter value child may occur zero or multiple times
    if (!is_array)
    {
      addLine("MinOccurs=1");
      addLine("MaxOccurs=1");
    }

    // *** ValType of parameter's value
    if (basic_type == "Integer")
      addLine("ValType=Int");
    else if (basic_type == "Real")
      addLine("ValType=Real");

    // *** ValEnums / InputChoices of parameter's value
    if (basic_type.find("Boolean") != std::string::npos)
      addLine("ValEnums=[ true false 1 0 on off ]");
    else
    {
      std::string options = param["options"];
      if (!options.empty())
      {
        pcrecpp::RE(" ").GlobalReplace("\" \"", &options);
        if (!param["out_of_range_allowed"])
          addLine("ValEnums=[ \"" + options + "\" ]");
        else
          addLine("InputChoices=[ \"" + options + "\" ]");
      }
    }

    // *** InputChoices (lookups) of parameter's value
    // add any reserved_values then check if there are any paths associated with the
    // cpp_type in the assoc_types_map that was built before traversal and add those
    // paths relative to this node here as well
    std::string choices;
    if (param.contains("reserved_values"))
    {
      for (const auto & reserved : param["reserved_values"])
        choices += nlohmann::to_string(reserved) + " ";
    }

    for (const auto & path : _assoc_types_map[cpp_type])
      choices += "PATH:\"" + backtrack(_level) + path + "/decl\" ";
    if (!choices.empty())
      addLine("InputChoices=[ " + choices + "]");

    // *** MinValInc of parameter's value
    if (cpp_type.compare(0, 8, "unsigned") == 0 && basic_type == "Integer")
      addLine("MinValInc=0");

    // *** InputDefault of parameter's value
    if (!def.empty())
      addLine("InputDefault=\"" + def + "\"");

    // *** close parameter's value
    _level--;
    addLine("}");

    // *** close parameter
    _level--;
    addLine("}");
  }
}
