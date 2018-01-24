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

SONDefinitionFormatter::SONDefinitionFormatter() : _spaces(2), _level(0) {}

// ******************************** toString ************************************ //
// traverse the associated types array of cpp_types and absolute lookup paths and //
// transform the paths to work with our parsed hierarchy and store pairs in a map //
// of types to paths for use by the ExistsIn rule and store the global/parameters //
// object then add root blocks recursively and return constructed stream's string //
// ****************************************************************************** //
std::string
SONDefinitionFormatter::toString(const JsonVal & root)
{

  const std::map<std::string, std::string> json_path_regex_replacement_map = {
      {"/star/subblock_types/([A-Za-z0-9_]*)/", "/\\1_type/"},
      {"[A-Za-z0-9_]*/types/([A-Za-z0-9_]*)/", "\\1_type/"},
      {"/actions/[A-Za-z0-9_]*/parameters/", "/"},
      {"/parameters/", "/"},
      {"/subblocks/", "/"}};

  for (const auto & type : root["global"]["associated_types"].getMemberNames())
    for (const auto & path_iter : root["global"]["associated_types"][type])
    {
      std::string path = path_iter.asString();
      for (const auto & map_iter : json_path_regex_replacement_map)
        pcrecpp::RE(map_iter.first).GlobalReplace(map_iter.second, &path);
      _assoc_types_map[type].push_back(path);
    }

  _global_params = root["global"]["parameters"];
  _stream.clear();
  _stream.str("");
  for (const auto & name : root["blocks"].getMemberNames())
    addBlock(name, root["blocks"][name]);
  return _stream.str();
}

// ******************************** addLine ************************************* //
// add a single new-line-terminated and indented line to the stream               //
// ****************************************************************************** //
void
SONDefinitionFormatter::addLine(const std::string & line)
{
  _stream << std::string(!line.empty() * _level * _spaces, ' ') << line << "\n";
  return;
}

// ******************************* addBlock ************************************* //
// add parameters and recursively add NormalBlock children and TypeBlock children //
// ****************************************************************************** //
void
SONDefinitionFormatter::addBlock(const std::string & block_name,
                                 const JsonVal & block,
                                 bool is_typeblock,
                                 const std::string & parent_name,
                                 const JsonVal & parameters_in,
                                 const JsonVal & subblocks_in)
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
  std::string description = block["description"].asString();
  pcrecpp::RE("\"").GlobalReplace("'", &description);
  pcrecpp::RE("[\r\n]").GlobalReplace(" ", &description);
  if (!description.empty())
    addLine("Description=\"" + description + "\"");

  // add MinOccurs : optional because nothing available to specify block requirement
  addLine("MinOccurs=0");

  // add MaxOccurs : if a StarBlock then no limit / otherwise maximum one occurrence
  addLine(is_starblock ? "MaxOccurs=NoLimit" : "MaxOccurs=1");

  // ensure block has one string declarator node and if this is not a StarBlock then
  // also ensure that the block [./declarator] is the expected block_decl from above
  addLine("decl{");
  _level++;
  addLine("MinOccurs=1");
  addLine("MaxOccurs=1");
  addLine("ValType=String");
  if (!is_starblock)
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
    addLine("} % end *");
  }

  // store parameters ---
  // first  : start with global parameters as a base
  // second : add or overwrite with any parameter inheritance
  // third  : add or overwrite with any local RegularParameters
  // fourth : add or overwrite with any local ActionParameters
  JsonVal parameters = _global_params;
  for (const auto & name : parameters_in.getMemberNames())
    parameters[name] = parameters_in[name];
  for (const auto & name : block["parameters"].getMemberNames())
    parameters[name] = block["parameters"][name];
  for (const auto & act : block["actions"].getMemberNames())
    for (const auto & param : block["actions"][act]["parameters"].getMemberNames())
      parameters[param] = block["actions"][act]["parameters"][param];

  // store NormalBlock children ---
  // first  : start with any NormalBlock inheritance passed in as a base
  // second : add or overwrite these with any local NormalBlock children
  // third  : add star named child block if it exists
  JsonVal subblocks = subblocks_in;
  for (const auto & name : block["subblocks"].getMemberNames())
    subblocks[name] = block["subblocks"][name];
  if (block.isMember("star"))
    subblocks["*"] = block["star"];

  // store TypeBlock children ---
  // first  : start with ["types"] child block as a base
  // second : add ["subblock_types"] child block
  JsonVal typeblocks = block["types"];
  for (const auto & name : block["subblock_types"].getMemberNames())
    typeblocks[name] = block["subblock_types"][name];

  // add parameters ---
  // if this block has a "type=" parameter with a specified default "type=" name and
  // if that default is also the name of a ["types"] child block then the parameters
  // belonging to that default ["types"] child block are added to this block as well
  // first  : start with default ["types"] child block's RegularParameters as a base
  // second : add or overwrite with default ["types"] child block's ActionParameters
  // third  : add or overwrite with parameters that were stored above for this block
  // fourth : either add newly stored parameters or add previously stored parameters
  if (parameters.isMember("type") && parameters["type"].isMember("default") &&
      block["types"].isMember(parameters["type"]["default"].asString()))
  {
    std::string type_default = parameters["type"]["default"].asString();
    const JsonVal & default_block = block["types"][type_default];
    JsonVal default_child_params = default_block["parameters"];
    const JsonVal & default_actions = default_block["actions"];
    for (const auto & act : default_actions.getMemberNames())
      for (const auto & param : default_actions[act]["parameters"].getMemberNames())
        default_child_params[param] = default_actions[act]["parameters"][param];
    for (const auto & name : parameters.getMemberNames())
      default_child_params[name] = parameters[name];
    addParameters(default_child_params);
  }
  else
    addParameters(parameters);

  // add previously stored NormalBlocks children recursively
  for (const auto & name : subblocks.getMemberNames())
    addBlock(name, subblocks[name]);

  // close block now because the parser stores TypeBlock children at this same level
  _level--;
  addLine("} % end block " + block_name + (is_typeblock ? "_type" : ""));

  // add all previously stored TypeBlock children recursively and pass the parameter
  // and NormalBlock children added at this level in as inheritance to all TypeBlock
  // children so that they may each also add them and pass in the name of this block
  // as well so that all TypeBlock children can add a rule ensuring that their block
  // [./declarator] is the name of this parent block unless this block is named star
  for (const auto & name : typeblocks.getMemberNames())
    addBlock(name, typeblocks[name], true, block_name, parameters, subblocks);
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
// - parameter's value :: add InputChoices
// - parameter's value :: add ExistsIn
// - parameter's value :: add MinValInc
// - parameter's value :: add InputDefault
// ****************************************************************************** //
void
SONDefinitionFormatter::addParameters(const JsonVal & params)
{

  for (const auto & name : params.getMemberNames())
  {

    JsonVal param = params[name];

    // lambda to calculate relative path from the current level to the document root
    auto backtrack = [](int level) {
      std::string backtrack_path;
      for (int i = 0; i < level; ++i)
        backtrack_path += "../";
      return backtrack_path;
    };

    // capture the cpp_type and basic_type and strip off any unnecessary information
    std::string cpp_type = param["cpp_type"].asString();
    std::string basic_type = param["basic_type"].asString();
    bool is_array = false;
    if (cpp_type == "FunctionExpression" || basic_type.compare(0, 6, "Array:") == 0)
      is_array = true;
    pcrecpp::RE(".+<([A-Za-z0-9_' ':]*)>.*").GlobalReplace("\\1", &cpp_type);
    pcrecpp::RE("(Array:)*(.*)").GlobalReplace("\\2", &basic_type);

    // *** ChildAtLeastOne of parameter
    // if parameter is required and no default exists then outside its level specify
    //   ChildAtLeastOne = [ "backtrack/GlobalParams/name/value" "name/value" ]
    bool required = param["required"].asBool();
    std::string def = MooseUtils::trim(param["default"].asString());
    if (required && def.empty())
      addLine("ChildAtLeastOne=[ \"" + backtrack(_level) + "GlobalParams/" + name +
              "/value\"   \"" + name + "/value\"" + " ]");

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
    std::string description = param["description"].asString();
    pcrecpp::RE("\"").GlobalReplace("'", &description);
    pcrecpp::RE("[\r\n]").GlobalReplace(" ", &description);
    if (!description.empty())
      addLine("Description=\"" + description + "\"");

    // *** MinOccurs / MaxOccurs of parameter
    addLine("MinOccurs=0");
    addLine("MaxOccurs=1");

    // *** open parameter's value
    addLine("'value'{");
    _level++;

    // *** MinOccurs / MaxOccurs of parameter's value
    addLine("MinOccurs=1");
    addLine(is_array ? "MaxOccurs=NoLimit" : "MaxOccurs=1");

    // *** ValType of parameter's value
    if (basic_type == "Integer")
      addLine("ValType=Int");
    else if (basic_type == "Real")
      addLine("ValType=Real");
    else
      addLine("ValType=String");

    // *** ValEnums / InputChoices of parameter's value
    if (basic_type.find("Boolean") != std::string::npos)
      addLine("ValEnums=[ true false 1 0 ]");
    else
    {
      std::string options = param["options"].asString();
      if (!options.empty())
      {
        pcrecpp::RE(" ").GlobalReplace("\" \"", &options);
        if (!param["out_of_range_allowed"].asBool())
          addLine("ValEnums=[ \"" + options + "\" ]");
        else
          addLine("InputChoices=[ \"" + options + "\" ]");
      }
    }

    // *** ExistsIn of parameter's value
    // add any reserved_values and if this parameter's above transformed cpp_type is
    // "FunctionName" then add an ExpressionsAreOkay flag and check if there are any
    // paths associated with the cpp_type in the map that was built before traversal
    // then add those paths relative to this node here as well
    std::string paths;
    for (const auto & reserved : param["reserved_values"])
      paths += "EXTRA:\"" + reserved.asString() + "\" ";
    if (cpp_type == "FunctionName")
      paths += "EXTRA:\"ExpressionsAreOkay\" ";
    for (const auto & path : _assoc_types_map[cpp_type])
      paths += "\"" + backtrack(_level) + path + "/decl\" ";
    if (!paths.empty())
      addLine("ExistsIn=[ " + paths + "]");

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
    addLine("} % end parameter " + name);
  }
}
