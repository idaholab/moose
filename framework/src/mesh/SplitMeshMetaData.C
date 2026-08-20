//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SplitMeshMetaData.h"

#include "Action.h"
#include "ActionWarehouse.h"
#include "InputParameters.h"
#include "MeshGenerator.h"
#include "MeshMetaDataInterface.h"
#include "MooseApp.h"
#include "Parser.h"
#include "RestartableData.h"
#include "RestartableDataMap.h"
#include "RestartableDataReader.h"

#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <limits>
#include <set>
#include <sstream>
#include <utility>

namespace
{
const std::string split_mesh_meta_data_prefix = "split_mesh";
const std::string split_mesh_input_fingerprint_name = std::string(MeshMetaDataInterface::SYSTEM) +
                                                      "/" + split_mesh_meta_data_prefix +
                                                      "/input_fingerprint";
const std::string split_mesh_input_summary_name = std::string(MeshMetaDataInterface::SYSTEM) + "/" +
                                                  split_mesh_meta_data_prefix + "/input_summary";

/**
 * Computes a deterministic 64-bit FNV-1a hash for canonicalized split mesh input data.
 */
std::string
stableHash(const std::string & value)
{
  // FNV-1a offset basis and prime. The 16-character hex output below represents all 64 bits.
  std::uint64_t hash = 14695981039346656037ULL;
  for (const auto c : value)
  {
    hash ^= static_cast<unsigned char>(c);
    hash *= 1099511628211ULL;
  }

  std::ostringstream oss;
  oss << std::hex << std::setfill('0') << std::setw(16) << hash;
  return oss.str();
}

/**
 * Converts a parameter value to the same string representation used in input summaries.
 */
std::string
parameterValue(const libMesh::Parameters::Value & value)
{
  std::ostringstream oss;
  value.print(oss);
  auto str = oss.str();
  if (!str.empty() && str.back() == ' ')
    str.pop_back();
  return str;
}

/**
 * Determines whether a parameter should participate in split mesh consistency checks.
 */
bool
includeInSplitMeshFingerprint(const InputParameters & params,
                              const std::string & name,
                              const std::set<std::string> & ignored)
{
  if (ignored.count(name))
    return false;
  if (!params.isParamValid(name) || params.isPrivate(name))
    return false;

  return true;
}

/**
 * Appends sorted, user-visible parameter values to the split mesh input summary.
 */
void
appendParametersForSplitMeshFingerprint(std::ostringstream & oss,
                                        const InputParameters & params,
                                        const std::set<std::string> & ignored)
{
  for (const auto & [name, value] : params)
    if (includeInSplitMeshFingerprint(params, name, ignored))
      oss << "    " << name << " = " << parameterValue(*value) << "\n";
}

/**
 * Converts raw HIT field values into a stable representation for mesh input summaries.
 */
std::string
canonicalHitValue(const std::string & value)
{
  std::istringstream input(value);
  std::ostringstream output;
  output << std::setprecision(std::numeric_limits<Real>::max_digits10);

  std::string token;
  bool first = true;
  while (input >> token)
  {
    Real real_value = 0;
    std::istringstream token_input(token);
    token_input >> real_value;

    if (!first)
      output << " ";
    first = false;

    if (token_input && token_input.eof())
      output << real_value;
    else
      output << token;
  }

  return output.str();
}

/**
 * Collects Mesh block field values from the parsed input file.
 */
class SplitMeshInputWalker : public hit::Walker
{
public:
  /**
   * Constructs a walker that skips fields with paths present in \p ignored.
   */
  SplitMeshInputWalker(const std::set<std::string> & ignored) : _ignored(ignored) {}

  void
  walk(const std::string & fullpath, const std::string & /* nodepath */, hit::Node * n) override;

  /**
   * Gets the collected input paths and canonical values.
   */
  const std::vector<std::pair<std::string, std::string>> & values() const { return _values; }

private:
  /// Mesh input field paths excluded from the split mesh fingerprint.
  const std::set<std::string> & _ignored;

  /// Collected pairs of input paths and canonical values.
  std::vector<std::pair<std::string, std::string>> _values;
};

/**
 * Records a canonical value for each non-ignored Mesh block field.
 */
void
SplitMeshInputWalker::walk(const std::string & fullpath,
                           const std::string & /* nodepath */,
                           hit::Node * n)
{
  if (n->type() != hit::NodeType::Field || _ignored.count(n->path()))
    return;

  const auto * field = dynamic_cast<const hit::Field *>(n);
  mooseAssert(field, "Expected a HIT field");
  _values.emplace_back(fullpath, canonicalHitValue(field->val()));
}

/**
 * Appends canonical Mesh block input values to the split mesh input summary.
 */
void
appendMeshInputForSplitMeshFingerprint(std::ostringstream & oss,
                                       Parser & parser,
                                       const std::set<std::string> & ignored)
{
  auto * mesh_node = parser.getRoot().find("Mesh");
  if (!mesh_node)
    return;

  SplitMeshInputWalker walker(ignored);
  mesh_node->walk(&walker, hit::NodeType::Field);

  auto values = walker.values();
  std::sort(values.begin(), values.end());

  oss << "[Mesh]\n";
  for (const auto & [path, value] : values)
    oss << "  " << path << " = " << value << "\n";
}
}

SplitMeshMetaData::SplitMeshMetaData(MooseApp & app) : _app(app) {}

std::string
SplitMeshMetaData::inputSummary()
{
  std::ostringstream oss;
  oss << "split-mesh-input-v1\n";

  const std::set<std::string> mesh_ignored = {
      "parallel_type", "use_split", "split_file", "_is_split"};

  oss << "[App]\n";
  oss << "  refinements = "
      << (_app.parameters().isParamSetByUser("refinements")
              ? _app.parameters().get<unsigned int>("refinements")
              : 0)
      << "\n";
  appendMeshInputForSplitMeshFingerprint(oss, _app.parser(), mesh_ignored);

  std::vector<const Action *> setup_mesh_actions;
  for (const auto action : _app.actionWarehouse().getActionListByName("setup_mesh"))
    setup_mesh_actions.push_back(action);

  std::sort(setup_mesh_actions.begin(),
            setup_mesh_actions.end(),
            [](const auto lhs, const auto rhs)
            {
              return std::make_pair(lhs->name(), lhs->type()) <
                     std::make_pair(rhs->name(), rhs->type());
            });

  for (const auto action : setup_mesh_actions)
  {
    oss << "[MeshAction]\n";
    oss << "  name = " << action->name() << "\n";
    oss << "  type = " << action->type() << "\n";
    appendParametersForSplitMeshFingerprint(oss, action->parameters(), mesh_ignored);
  }

  auto generator_names = _app.getMeshGeneratorNames();
  std::sort(generator_names.begin(), generator_names.end());
  for (const auto & name : generator_names)
  {
    const auto & generator = _app.getMeshGenerator(name);
    oss << "[Mesh/" << generator.name() << "]\n";
    oss << "    type = " << generator.type() << "\n";
    appendParametersForSplitMeshFingerprint(oss, generator.parameters(), {});
  }

  return oss.str();
}

std::string
SplitMeshMetaData::inputFingerprint(const std::string & summary)
{
  return stableHash(summary);
}

std::filesystem::path
SplitMeshMetaData::metaDataFolderBase(const std::filesystem::path & folder_base)
{
  const auto split_count = std::to_string(_app.n_processors());
  if (folder_base.filename() == split_count)
    return folder_base;

  const auto split_count_folder = folder_base / split_count;
  if (std::filesystem::exists(split_count_folder))
    return split_count_folder;

  return folder_base;
}

std::vector<std::filesystem::path>
SplitMeshMetaData::write(const std::filesystem::path & folder_base)
{
  auto add_or_update = [this](const std::string & name, const std::string & value)
  {
    RestartableDataValue * stored_data = nullptr;
    if (_app.hasRestartableMetaData(name, MooseApp::MESH_META_DATA))
      stored_data = &_app.getRestartableMetaData(name, MooseApp::MESH_META_DATA, 0);
    else
    {
      auto data = std::make_unique<RestartableData<std::string>>(name, nullptr, value);
      stored_data =
          &_app.registerRestartableData(std::move(data), 0, false, MooseApp::MESH_META_DATA);
    }

    auto * string_data = dynamic_cast<RestartableData<std::string> *>(stored_data);
    mooseAssert(string_data, "Unexpected restartable data type");
    string_data->set() = value;
  };

  // Store the exact summary that was hashed so mismatch reports can show the input that produced
  // the stored fingerprint.
  const auto summary = inputSummary();
  add_or_update(split_mesh_input_fingerprint_name, inputFingerprint(summary));
  add_or_update(split_mesh_input_summary_name, summary);

  return _app.writeRestartableMetaData(MooseApp::MESH_META_DATA, metaDataFolderBase(folder_base));
}

void
SplitMeshMetaData::check(const std::filesystem::path & folder_base)
{
  // A split mesh is read before this point. The validation metadata is stored next to that split
  // configuration, so this restores only the split-mesh fingerprint fields, accepts legacy splits
  // that do not have them, and errors when the stored fingerprint differs from the current mesh
  // input fingerprint.
  const auto & map_name = _app.getRestartableDataMapName(MooseApp::MESH_META_DATA);
  const auto split_mesh_folder_base = metaDataFolderBase(folder_base);
  const auto meta_data_folder_base = MooseApp::metaDataFolderBase(split_mesh_folder_base, map_name);

  if (!RestartableDataReader::isAvailable(meta_data_folder_base))
  {
    if (_app.processor_id() == 0)
      mooseInfo("The pre-split mesh file '",
                split_mesh_folder_base,
                "' does not contain mesh meta data. The mesh input cannot be checked for "
                "consistency with the pre-split mesh.");
    return;
  }

  RestartableDataMap split_mesh_meta_data;
  auto fingerprint_data = std::make_unique<RestartableData<std::string>>(
      split_mesh_input_fingerprint_name, nullptr, std::string());
  auto * fingerprint = fingerprint_data.get();
  split_mesh_meta_data.addData(std::move(fingerprint_data));
  auto summary_data = std::make_unique<RestartableData<std::string>>(
      split_mesh_input_summary_name, nullptr, std::string());
  auto * summary = summary_data.get();
  split_mesh_meta_data.addData(std::move(summary_data));

  RestartableDataReader reader(_app, split_mesh_meta_data, _app.forceRestart());
  reader.setErrorOnLoadWithDifferentNumberOfProcessors(false);
  reader.setInput(meta_data_folder_base);
  reader.restore();

  if (!fingerprint->loaded())
  {
    if (_app.processor_id() == 0)
      mooseInfo("The pre-split mesh file '",
                split_mesh_folder_base,
                "' does not contain a mesh input fingerprint. The mesh input cannot be checked "
                "for consistency with the pre-split mesh.");
    return;
  }

  const auto current_summary = inputSummary();

  // Compare compact fingerprints in normal runs, but report the full summaries on mismatch so
  // users can see which mesh input changed.
  const auto current_fingerprint = inputFingerprint(current_summary);
  if (fingerprint->get() != current_fingerprint)
  {
    std::ostringstream oss;
    oss << "The pre-split mesh file '" << split_mesh_folder_base
        << "' was generated from different mesh input than the current run.\n\n"
        << "Stored mesh input fingerprint: " << fingerprint->get()
        << "\nCurrent mesh input fingerprint: " << current_fingerprint;

    if (summary->loaded())
      oss << "\n\nStored mesh input summary:\n" << summary->get();
    oss << "\nCurrent mesh input summary:\n"
        << current_summary
        << "\nRegenerate the split mesh with --split-mesh, or run without --use-split.";

    mooseError(oss.str());
  }
}
