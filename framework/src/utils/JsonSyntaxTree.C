//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "JsonSyntaxTree.h"

#include "MooseEnum.h"
#include "MultiMooseEnum.h"
#include "ExecFlagEnum.h"
#include "Builder.h"
#include "pcrecpp.h"
#include "Action.h"
#include "AppFactory.h"
#include "Registry.h"
#include "MooseUtils.h"
#include "Syntax.h"
#include "AppFactory.h"
#include "FileLineInfo.h"

#include "libmesh/vector_value.h"

// C++ includes
#include <algorithm>
#include <cctype>

JsonSyntaxTree::JsonSyntaxTree(const Syntax & syntax,
                               const ActionFactory & action_factory,
                               const Factory & factory,
                               const std::optional<std::string> & search /* = {} */)
  : _syntax(syntax), _action_factory(action_factory), _factory(factory), _search(search)
{
  // Registry holds a map with labels (ie MooseApp) as keys and a vector of RegistryEntry
  // as values. We need the reverse map: given an action or object name then get the label.
  for (const auto & [label, entries] : Registry::allActions())
    for (const auto & entry : entries)
      _action_label_map[entry->_classname] = std::make_pair(label, entry->_file);
  for (const auto & [label, entries] : Registry::allObjects())
    for (const auto & entry : entries)
      _object_label_map[entry->name()] = std::make_pair(label, entry->_file);
}

nlohmann::json
JsonSyntaxTree::build()
{
  // Reset root json node
  _root = std::make_unique<nlohmann::json>();
  auto & root = *_root;

  // Add syntax types
  for (const auto & [path, type] : _syntax.getAssociatedTypes())
  {
    if (!_search || MooseUtils::wildCardMatch(path, *_search))
      getJson(path)["associated_types"].push_back(type);
    if (!_search)
      root["global"]["associated_types"][type].push_back(path);
  }

  // Build a list of all the actions appearing in the syntax
  const auto & associated_actions = _syntax.getAssociatedActions();
  std::vector<std::pair<std::string, Syntax::ActionInfo>> all_names(associated_actions.begin(),
                                                                    associated_actions.end());
  // If the task is empty, that means we need to figure out which task
  // goes with this syntax for the purpose of building the MooseObject
  // part of the tree; query the ActionFactory for the registration info
  for (auto & name_act_info_pair : all_names)
  {
    auto & act_info = name_act_info_pair.second;
    if (act_info._task == "")
      act_info._task = _action_factory.getTaskName(act_info._action);
  }

  // Build a cache of registered objects (with a base) to their parameters.
  // The action loop that follows below will search for objects that match
  // an action's syntax, which requires knowing all object params for each
  // action and we don't want to rebuild the params every time
  std::vector<std::pair<std::string, const InputParameters>> object_params;
  const auto & objects = _factory.registeredObjects();
  object_params.reserve(objects.size());
  for (const auto & [type, entry_ptr] : _factory.registeredObjects())
  {
    auto params = entry_ptr->buildParameters();
    if (params.hasBase())
    {
      params.set<std::string>("type") = type;
      object_params.emplace_back(type, std::move(params));
    }
  }

  // Add all the actions to the JSON tree, except for ActionComponents (below)
  for (const auto & [syntax, act_info] : all_names)
  {
    const std::string & action = act_info._action;
    const std::string & task = act_info._task;
    const auto action_obj_params = _action_factory.getValidParams(action);
    const bool params_added = addParameters("",
                                            syntax,
                                            false,
                                            action,
                                            true,
                                            action_obj_params,
                                            _syntax.getLineInfo(syntax, action, ""),
                                            "");

    if (params_added)
      for (const auto & task : _action_factory.getTasksByAction(action))
      {
        const auto line_info = _action_factory.getLineInfo(action, task);
        auto & entry = getJson("", syntax, false);
        if (line_info.isValid())
          entry[action]["tasks"][task]["file_info"][line_info.file()] = line_info.line();
      }

    // If this action is a MooseObject action, we will loop over all of the
    // registered MooseObjects and will add those that have associated
    // bases matching the current task
    if (action_obj_params.have_parameter<bool>("isObjectAction") &&
        action_obj_params.get<bool>("isObjectAction"))
    {
      for (const auto & [moose_obj_name, moose_obj_params] : object_params)
      {
        // Now that we know that this is a MooseObjectAction we need to see if it has been
        // restricted in any way by the user.
        const auto & buildable_types = action_obj_params.getBuildableTypes();

        // See if the current Moose Object syntax belongs under this Action's block
        if ((buildable_types.empty() || // Not restricted
             std::find(buildable_types.begin(), buildable_types.end(), moose_obj_name) !=
                 buildable_types.end()) && // Restricted but found
            _syntax.verifyMooseObjectTask(moose_obj_params.getBase(),
                                          task) &&          // and that base is associated
            action_obj_params.mooseObjectSyntaxVisibility() // and the Action says it's visible
        )
        {
          std::string name;
          size_t pos = 0;
          bool is_action_params = false;
          bool is_type = false;
          if (syntax[syntax.size() - 1] == '*')
          {
            pos = syntax.size();

            if (!action_obj_params.collapseSyntaxNesting())
              name = syntax.substr(0, pos - 1) + moose_obj_name;
            else
            {
              name = syntax.substr(0, pos - 1) + "/<type>/" + moose_obj_name;
              is_action_params = true;
            }
          }
          else
          {
            name = syntax + "/<type>/" + moose_obj_name;
            is_type = true;
          }

          addParameters(syntax,
                        name,
                        is_type,
                        moose_obj_name,
                        is_action_params,
                        moose_obj_params,
                        _factory.getLineInfo(moose_obj_name),
                        _factory.associatedClassName(moose_obj_name));
        }
      }

      // Same thing for ActionComponents, which, while they are not MooseObjects, should behave
      // similarly syntax-wise
      if (syntax != "ActionComponents/*")
        continue;

      const auto its = _action_factory.getActionsByTask("list_component");
      for (const auto & task_action_pair : as_range(its.first, its.second))
      {
        // Get the name and parameters
        const auto & component_name = task_action_pair.second;
        auto component_params = _action_factory.getValidParams(component_name);

        // We currently do not have build-type restrictions on this action that adds
        // action-components

        // See if the current Moose Object syntax belongs under this Action's block
        // and it is visible
        if (action_obj_params.mooseObjectSyntaxVisibility())
        {
          // The logic for Components is a little simpler here for now because syntax like
          // Executioner/TimeIntegrator/type= do not exist for components
          std::string name;
          if (syntax[syntax.size() - 1] == '*')
          {
            size_t pos = syntax.size();
            name = syntax.substr(0, pos - 1) + component_name;
          }
          component_params.set<std::string>("type") = component_name;

          // We add the parameters as for an object, because we want to fit them to be
          // added to json["AddActionComponentAction"]["subblock_types"]
          addParameters(syntax,
                        /*syntax_path*/ name,
                        /*is_type*/ false,
                        "AddActionComponentAction",
                        /*is_action=*/false,
                        component_params,
                        _action_factory.getLineInfo(component_name, "list_component"),
                        component_name);
        }
      }
    }
  }

  // Helper for adding an application's params
  const auto add_app_params = [this](const std::string & type,
                                     InputParameters params,
                                     const std::string & file,
                                     const int line)
  {
    params.set<std::string>("type") = type;
    addParameters("Application",
                  "Application/<type>/" + type,
                  true,
                  type,
                  false,
                  params,
                  FileLineInfo(file, line),
                  "");
  };

  // Add registered applications to blocks/Application/types
  for (const auto & [type, build_info] : AppFactory::instance().registeredObjectBuildInfos())
    add_app_params(type, build_info->buildParameters(), build_info->file, build_info->line);
  // Even though MooseApp isn't a registered object, it is useful to
  // reference its parameters
  add_app_params("MooseApp", MooseApp::validParams(), "", 0);

  // Add "global" entries (parameters and registered_apps)
  if (!_search)
    addGlobal();

  return std::move(_root);
}

nlohmann::json &
JsonSyntaxTree::getJson(const std::string & path)
{
  auto & root = *_root;

  const auto paths = MooseUtils::split(path, "/");
  mooseAssert(paths.size() > 0, "path is empty");
  auto * next = &(root["blocks"][paths[0]]);

  for (auto pit = paths.begin() + 1; pit != paths.end(); ++pit)
  {
    if (*pit == "*")
      // It has an action syntax as a parent
      next = &(*next)["star"];
    else if (*pit == "<type>")
      next = &(*next)["types"];
    else
      next = &(*next)["subblocks"][*pit];
  }
  return *next;
}

nlohmann::json &
JsonSyntaxTree::getJson(const std::string & parent, const std::string & path, const bool is_type)
{
  if (parent.empty())
  {
    auto & j = getJson(path);
    if (path.back() == '*' && !j.contains("subblock_types"))
      j["subblock_types"] = nlohmann::json();
    else if (path.back() != '*' && !j.contains("types"))
      j["types"] = nlohmann::json();
    return j["actions"];
  }

  auto & parent_json = getJson(parent);
  const auto paths = MooseUtils::split(path, "/");
  std::string key = "subblock_types";
  if (is_type)
    key = "types";
  auto & val = parent_json[key][paths.back()];
  return val;
}

size_t
JsonSyntaxTree::setParams(const InputParameters & params,
                          const bool search_match,
                          nlohmann::json & all_params)
{
  size_t count = 0;
  for (const auto & name_param_pair : params)
  {
    const auto & param_name = name_param_pair.first;

    // Make sure we want to see this parameter
    if (params.isPrivate(param_name) ||
        (_search && !search_match && !MooseUtils::wildCardMatch(param_name, *_search)))
      continue;

    ++count;
    auto & param_json = all_params[param_name];

    param_json["required"] = params.isParamRequired(param_name);

    // Only output default if it has one
    if (params.isParamValid(param_name))
      param_json["default"] = buildOutputString(name_param_pair);
    else if (params.hasDefaultCoupledValue(param_name))
      param_json["default"] = params.defaultCoupledValue(param_name);

    bool out_of_range_allowed = false;
    std::map<MooseEnumItem, std::string> docs;
    param_json["options"] = buildOptions(name_param_pair, out_of_range_allowed, docs);
    if (!nlohmann::to_string(param_json["options"]).empty())
    {
      param_json["out_of_range_allowed"] = out_of_range_allowed;
      if (!docs.empty())
      {
        nlohmann::json jdocs;
        for (const auto & doc : docs)
          jdocs[doc.first.name()] = doc.second;
        param_json["option_docs"] = jdocs;
      }
    }
    auto reserved_values = params.reservedValues(param_name);
    for (const auto & reserved : reserved_values)
      param_json["reserved_values"].push_back(reserved);

    std::string t = MooseUtils::prettyCppType(params.type(param_name));
    param_json["cpp_type"] = t;
    param_json["basic_type"] = basicCppType(t);
    if (const auto group_ptr = params.queryParameterGroup(param_name))
      param_json["group_name"] = *group_ptr;
    else
      param_json["group_name"] = "";
    param_json["name"] = param_name;

    std::string doc = params.getDocString(param_name);
    MooseUtils::escape(doc);
    param_json["description"] = doc;

    param_json["doc_unit"] = params.getDocUnit(param_name);

    param_json["controllable"] = params.isControllable(param_name);
    param_json["deprecated"] = params.isParamDeprecated(param_name);

    if (const auto cl_metadata = params.queryCommandLineMetadata(param_name))
    {
      auto & cl_entry = param_json["command_line"];
      cl_entry["syntax"] = cl_metadata->syntax;
      cl_entry["global"] = cl_metadata->global;
      cl_entry["input_enabled"] = cl_metadata->input_enabled;
    }
  }
  return count;
}

void
JsonSyntaxTree::addGlobal()
{
  auto & root = *_root;

  const auto params = Moose::Builder::validParams();
  nlohmann::json jparams;
  setParams(params, true, jparams);
  root["global"]["parameters"] = jparams;

  // Just create a list of registered app names
  nlohmann::json apps;
  auto & factory = AppFactory::instance();
  for (const auto & name_bi_pair : factory.registeredObjects())
    apps.push_back(name_bi_pair.first);

  root["global"]["registered_apps"] = apps;
}

bool
JsonSyntaxTree::addParameters(const std::string & parent,
                              const std::string & path,
                              bool is_type,
                              const std::string & action,
                              bool is_action,
                              const InputParameters & params,
                              const FileLineInfo & lineinfo,
                              const std::string & classname)
{
  if (action == "EmptyAction")
    return false;

  nlohmann::json all_params;
  bool search_match = _search && (MooseUtils::wildCardMatch(path, *_search) ||
                                  MooseUtils::wildCardMatch(action, *_search) ||
                                  MooseUtils::wildCardMatch(parent, *_search));
  auto count = setParams(params, search_match, all_params);

  // no parameters that matched the search string
  if (_search && count == 0)
    return false;

  nlohmann::json & json = getJson(parent, path, is_type);

  const auto fill_base = [&all_params, &params, &lineinfo](nlohmann::json & entry)
  {
    entry["parameters"] = all_params;
    entry["description"] = params.getClassDescription();
    if (lineinfo.isValid())
      entry["file_info"][lineinfo.file()] = lineinfo.line();
  };

  if (is_action)
  {
    auto & entry = json[action];
    fill_base(entry);
    entry["action_path"] = path;
    const auto & [label, file] = getActionLabel(action);
    entry["label"] = label;
    entry["register_file"] = file;
  }
  else
  {
    fill_base(json);

    if (params.hasBase())
      json["moose_base"] = params.getBase();

    json["syntax_path"] = path;
    json["parent_syntax"] = parent;
    // We do this for ActionComponents which are registered as Actions but
    // dumped to the syntax tree as Objects
    if (params.hasBase() && params.getBase() == "Action")
    {
      const auto & [label, file] = getActionLabel(action);
      json["label"] = label;
      json["register_file"] = file;
    }
    // Applications don't have an object label
    else if (params.hasBase() && params.getBase() == "Application")
    {
      json["register_file"] = lineinfo.file();
    }
    else
    {
      const auto & [label, file] = getObjectLabel(path);
      json["label"] = label;
      json["register_file"] = file;
    }
    if (lineinfo.isValid() && !classname.empty())
      json["class"] = classname;
  }
  return true;
}

std::string
JsonSyntaxTree::buildOptions(
    const std::iterator_traits<InputParameters::const_iterator>::value_type & p,
    bool & out_of_range_allowed,
    std::map<MooseEnumItem, std::string> & docs)
{
  const libMesh::Parameters::Value * val = MooseUtils::get(p.second);

  std::string options;

  // Set options for enum and vector<enum> classes
  const auto set_enum_options = [&options, &out_of_range_allowed, &docs](const auto & enum_value)
  {
    out_of_range_allowed = enum_value.isOutOfRangeAllowed();
    options = enum_value.getRawNames();
    docs = enum_value.getItemDocumentation();
  };
  if (auto param = dynamic_cast<const libMesh::Parameters::Parameter<MooseEnum> *>(val))
    set_enum_options(param->get());
  else if (auto param = dynamic_cast<const libMesh::Parameters::Parameter<MultiMooseEnum> *>(val))
    set_enum_options(param->get());
  else if (auto param = dynamic_cast<const libMesh::Parameters::Parameter<ExecFlagEnum> *>(val))
    set_enum_options(param->get());
  else if (auto param =
               dynamic_cast<const libMesh::Parameters::Parameter<std::vector<MooseEnum>> *>(val))
    set_enum_options(param->get()[0]);
  else if (auto param =
               dynamic_cast<const libMesh::Parameters::Parameter<std::vector<MultiMooseEnum>> *>(
                   val))
    set_enum_options(param->get()[0]);

  return options;
}

std::string
JsonSyntaxTree::buildOutputString(
    const std::iterator_traits<InputParameters::const_iterator>::value_type & p)
{
  const libMesh::Parameters::Value * val = MooseUtils::get(p.second);

  std::stringstream str;

  const auto set_components = [&str](const auto & value)
  { str << value(0) << " " << value(1) << " " << value(2); };

  // Point
  if (auto point = dynamic_cast<const InputParameters::Parameter<Point> *>(val))
    set_components(point->get());
  // RealVectorValue
  else if (auto point = dynamic_cast<const InputParameters::Parameter<RealVectorValue> *>(val))
    set_components(point->get());
  // General case, call the print operator
  else
    val->print(str);

  // remove additional '\n' possibly generated in output (breaks JSON parsing)
  std::string tmp_str = str.str();
  for (auto & ch : tmp_str)
    if (ch == '\n')
      ch = ' ';

  return tmp_str.substr(0, tmp_str.find("<RESIDUAL>"));
}

std::string
JsonSyntaxTree::basicCppType(const std::string & cpp_type)
{
  std::string s = "String";
  if (cpp_type.find("std::vector") != std::string::npos ||
      cpp_type.find("libMesh::VectorValue") != std::string::npos ||
      cpp_type.find("libMesh::TensorValue") != std::string::npos ||
      cpp_type.find("Eigen::Matrix") != std::string::npos)
  {
    // Get the template type and use its basic type for the array type
    pcrecpp::RE r("^[^<]+<\\s*(.*)\\s*>$");
    std::string t;
    r.FullMatch(cpp_type, &t);

    // Capture type just to the first comma for Eigen::Matrix<type,V,W,X,Y,Z>
    if (cpp_type.find("Eigen::Matrix") != std::string::npos)
      t = t.substr(0, t.find(","));

    s = "Array:" + basicCppType(t);
  }
  else if (cpp_type.find("MultiMooseEnum") != std::string::npos ||
           cpp_type.find("ExecFlagEnum") != std::string::npos ||
           cpp_type.find("VectorPostprocessorName") != std::string::npos ||
           cpp_type.find("std::map") != std::string::npos)
    s = "Array:String";
  else if (cpp_type.find("libMesh::Point") != std::string::npos)
    s = "Array:Real";
  else if (cpp_type == "int" || cpp_type == "unsigned int" || cpp_type == "short" ||
           cpp_type == "unsigned short" || cpp_type == "char" || cpp_type == "unsigned char" ||
           cpp_type == "long" || cpp_type == "unsigned long" || cpp_type == "long long" ||
           cpp_type == "unsigned long long")
    s = "Integer";
  else if (cpp_type == "double" || cpp_type == "float")
    s = "Real";
  else if (cpp_type == "bool")
    s = "Boolean";

  return s;
}

const std::pair<std::string, std::string> &
JsonSyntaxTree::getObjectLabel(const std::string & obj) const
{
  const auto paths = MooseUtils::split(obj, "/");
  if (const auto it = _object_label_map.find(paths.back()); it != _object_label_map.end())
    return it->second;
  static const std::pair<std::string, std::string> empty;
  return empty;
}

const std::pair<std::string, std::string> &
JsonSyntaxTree::getActionLabel(const std::string & action) const
{
  if (const auto it = _action_label_map.find(action); it != _action_label_map.end())
    return it->second;
  static const std::pair<std::string, std::string> empty;
  return empty;
}
