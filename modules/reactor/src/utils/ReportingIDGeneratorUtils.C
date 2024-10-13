//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReportingIDGeneratorUtils.h"

using namespace libMesh;

std::vector<dof_id_type>
ReportingIDGeneratorUtils::getCellwiseIntegerIDs(
    const std::vector<std::unique_ptr<ReplicatedMesh>> & meshes,
    const std::vector<std::vector<unsigned int>> & pattern,
    const bool use_exclude_id,
    const std::vector<bool> & exclude_ids)
{
  dof_id_type n = 0;
  for (MooseIndex(pattern) i = 0; i < pattern.size(); ++i)
    for (MooseIndex(pattern[i]) j = 0; j < pattern[i].size(); ++j)
      n += meshes[pattern[i][j]]->n_elem();
  std::vector<dof_id_type> integer_ids;
  integer_ids.reserve(n);
  dof_id_type id = 0;
  for (MooseIndex(pattern) i = 0; i < pattern.size(); ++i)
    for (MooseIndex(pattern[i]) j = 0; j < pattern[i].size(); ++j)
    {
      const auto value =
          (use_exclude_id && exclude_ids[pattern[i][j]]) ? DofObject::invalid_id : id++;
      integer_ids.insert(integer_ids.end(), meshes[pattern[i][j]]->n_elem(), value);
    }
  return integer_ids;
}

std::vector<dof_id_type>
ReportingIDGeneratorUtils::getPatternIntegerIDs(
    const std::vector<std::unique_ptr<ReplicatedMesh>> & meshes,
    const std::vector<std::vector<unsigned int>> & pattern)
{
  dof_id_type n = 0;
  for (MooseIndex(pattern) i = 0; i < pattern.size(); ++i)
    for (MooseIndex(pattern[i]) j = 0; j < pattern[i].size(); ++j)
      n += meshes[pattern[i][j]]->n_elem();
  std::vector<dof_id_type> integer_ids;
  integer_ids.reserve(n);
  for (MooseIndex(pattern) i = 0; i < pattern.size(); ++i)
    for (MooseIndex(pattern[i]) j = 0; j < pattern[i].size(); ++j)
      integer_ids.insert(integer_ids.end(), meshes[pattern[i][j]]->n_elem(), pattern[i][j]);
  return integer_ids;
}

std::vector<dof_id_type>
ReportingIDGeneratorUtils::getManualIntegerIDs(
    const std::vector<std::unique_ptr<ReplicatedMesh>> & meshes,
    const std::vector<std::vector<unsigned int>> & pattern,
    const std::vector<std::vector<dof_id_type>> & id_pattern)
{
  dof_id_type n = 0;
  for (MooseIndex(pattern) i = 0; i < pattern.size(); ++i)
    for (MooseIndex(pattern[i]) j = 0; j < pattern[i].size(); ++j)
      n += meshes[pattern[i][j]]->n_elem();
  std::vector<dof_id_type> integer_ids;
  integer_ids.reserve(n);
  for (MooseIndex(pattern) i = 0; i < pattern.size(); ++i)
    for (MooseIndex(pattern[i]) j = 0; j < pattern[i].size(); ++j)
      integer_ids.insert(integer_ids.end(), meshes[pattern[i][j]]->n_elem(), id_pattern[i][j]);
  return integer_ids;
}

std::set<SubdomainID>
ReportingIDGeneratorUtils::getCellBlockIDs(
    const std::vector<std::unique_ptr<ReplicatedMesh>> & meshes,
    const std::vector<std::vector<unsigned int>> & pattern)
{
  std::set<SubdomainID> blks;
  for (MooseIndex(pattern) i = 0; i < pattern.size(); ++i)
    for (MooseIndex(pattern[i]) j = 0; j < pattern[i].size(); ++j)
    {
      std::set<SubdomainID> mesh_blks;
      meshes[pattern[i][j]]->subdomain_ids(mesh_blks);
      blks.insert(mesh_blks.begin(), mesh_blks.end());
    }
  return blks;
}

std::map<SubdomainID, unsigned int>
ReportingIDGeneratorUtils::getDuckBlockIDs(const MeshBase & mesh,
                                           const bool has_assembly_boundary,
                                           const std::set<subdomain_id_type> background_blk_ids,
                                           const std::set<SubdomainID> & blks)
{
  std::map<SubdomainID, unsigned int> blks_duct;
  if (has_assembly_boundary)
  {
    std::set<SubdomainID> mesh_blks;
    mesh.subdomain_ids(mesh_blks);
    unsigned int i = 0;
    for (const auto mesh_blk : mesh_blks)
      if (!blks.count(mesh_blk) && !background_blk_ids.count(mesh_blk))
        blks_duct[mesh_blk] = i++;
    // delete the first entry because it is for surrouding regions between the assembly duct and pin
    if (background_blk_ids.size() == 0)
      blks_duct.erase(blks_duct.begin());
  }
  return blks_duct;
}

void
ReportingIDGeneratorUtils::assignReportingIDs(
    MeshBase & mesh,
    const unsigned int extra_id_index,
    const ReportingIDGeneratorUtils::AssignType assign_type,
    const bool use_exclude_id,
    const std::vector<bool> & exclude_ids,
    const bool has_assembly_boundary,
    const std::set<subdomain_id_type> background_block_ids,
    const std::vector<std::unique_ptr<ReplicatedMesh>> & input_meshes,
    const std::vector<std::vector<unsigned int>> & pattern,
    const std::vector<std::vector<dof_id_type>> & id_pattern)
{
  std::vector<dof_id_type> integer_ids;
  // get reporting ID map
  // assumes that the entire mesh has elements of each individual mesh sequentially ordered.
  if (assign_type == AssignType::cell)
    integer_ids = getCellwiseIntegerIDs(input_meshes, pattern, use_exclude_id, exclude_ids);
  else if (assign_type == AssignType::pattern)
    integer_ids = getPatternIntegerIDs(input_meshes, pattern);
  else if (assign_type == AssignType::manual)
    integer_ids = getManualIntegerIDs(input_meshes, pattern, id_pattern);

  if (has_assembly_boundary)
  {
    // setup assembly duct information
    const std::set<SubdomainID> blks = getCellBlockIDs(input_meshes, pattern);
    const unsigned int duct_boundary_id =
        *std::max_element(integer_ids.begin(), integer_ids.end()) + 1;
    const std::map<SubdomainID, unsigned int> blks_duct =
        getDuckBlockIDs(mesh, has_assembly_boundary, background_block_ids, blks);

    // assign reporting IDs to individual elements
    unsigned int i = 0;
    unsigned int id = integer_ids[i];
    unsigned old_id = id;
    for (auto elem : mesh.element_ptr_range())
    {
      auto blk = elem->subdomain_id();
      // check whether the current element belongs to duct/surrouding regions or not
      if (!blks.count(blk))
      {
        // check whether the current element belongs to surroudning or duct regions
        if (!blks_duct.count(blk))
          // if the current element belongs to the surronding region
          elem->set_extra_integer(extra_id_index, old_id);
        else
          // if the current element belongs to the duct region
          elem->set_extra_integer(extra_id_index, duct_boundary_id + blks_duct.at(blk));
      }
      else
      {
        // if the current element belongs to pin regions
        elem->set_extra_integer(extra_id_index, id);
        ++i;
        old_id = id;
        if (i < integer_ids.size())
          id = integer_ids[i];
      }
    }
  }
  else
  {
    // assign reporting IDs to individual elements
    unsigned int i = 0;
    for (auto & elem : mesh.element_ptr_range())
      elem->set_extra_integer(extra_id_index, integer_ids[i++]);
  }
}
