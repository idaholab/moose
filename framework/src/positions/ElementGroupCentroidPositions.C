//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementGroupCentroidPositions.h"

registerMooseObject("MooseApp", ElementGroupCentroidPositions);

InputParameters
ElementGroupCentroidPositions::validParams()
{
  InputParameters params = Positions::validParams();
  params += BlockRestrictable::validParams();
  params.addClassDescription("Gets the Positions of the centroid of groups of elements. "
                             "Groups may be defined using subdomains or element extra ids.");

  // To enable extra element ID groups
  params.addParam<MooseEnum>("grouping_type", groupTypeEnum(), "Type of group of elements");
  params.addParam<std::vector<ExtraElementIDName>>("extra_id_name",
                                                   "Name(s) of the extra element ID(s) to use");
  params.addParam<std::vector<std::vector<unsigned int>>>(
      "extra_id",
      "Specific ID(s), for each extra id name, for grouping elements. "
      "If empty, all *valid* ids will be used to bin");

  // Order already arises from the block/ID bins
  params.set<bool>("auto_sort") = false;
  // Full replicated mesh is used on every rank to generate positions
  params.set<bool>("auto_broadcast") = false;

  return params;
}

ElementGroupCentroidPositions::ElementGroupCentroidPositions(const InputParameters & parameters)
  : Positions(parameters),
    BlockRestrictable(this),
    _mesh(_fe_problem.mesh()),
    _group_type(getParam<MooseEnum>("grouping_type"))
{
  // We would need reductions on volumes, not just positions to make this work for
  // distributed.
  _mesh.errorIfDistributedMesh(type());

  // We are not excluding using both block restriction and extra element ids
  if (_group_type == "extra_id" || _group_type == "block_and_extra_id")
  {
    _extra_id_names = getParam<std::vector<ExtraElementIDName>>("extra_id_name");
    for (const auto & name : _extra_id_names)
      _extra_id_indices.push_back(_mesh.getMesh().get_elem_integer_index(name));
    _extra_id_group_indices = getParam<std::vector<std::vector<unsigned int>>>("extra_id");

    if (_extra_id_group_indices.size() != _extra_id_names.size())
      paramError("extra_id",
                 "Number of extra id names and the indices to select must match. "
                 "If you want all indices for an extra id, use an empty vector entry");

    // Can only have so many groups though, considering 4D max capability
    if (_extra_id_indices.size() > unsigned(3 + blockRestricted()))
      mooseError("Positions currently supports only up to 4D storage");
  }
  else
  {
    if (isParamValid("extra_id_name"))
      paramError("extra_id_name",
                 "An extra id name was specified but elements are not grouped by extra ids");
    if (isParamValid("extra_ids"))
      paramError("extra_ids",
                 "An extra id was specified but elements are not grouped by extra ids");
  }

  // Insert subdomain as an extra element id to simplify computation logic
  if (blockRestricted() || !isParamValid("extra_id_name"))
  {
    _blocks_in_use = true;
    _extra_id_names.insert(_extra_id_names.begin(), "block");
    _extra_id_indices.insert(_extra_id_indices.begin(), std::numeric_limits<unsigned short>::max());
    _extra_id_group_indices.insert(_extra_id_group_indices.begin(), std::vector<unsigned int>());
    // Add real block restriction
    if (blockRestricted())
      for (const auto & block : blockIDs())
        _extra_id_group_indices[0].push_back(block);
    // Just add all blocks
    else
      for (const auto & block : meshBlockIDs())
        _extra_id_group_indices[0].push_back(block);
  }
  else
    _blocks_in_use = false;

  // Mesh is ready at construction
  initialize();
  // Sort the output (by XYZ) if requested
  finalize();
}

void
ElementGroupCentroidPositions::initialize()
{
  clearPositions();
  // By default, initialize should be called on meshChanged()

  // If users did not specify a value for an extra element integer, they want all the bins
  // We'll need to loop through the mesh to find the element extra ids
  for (const auto i : index_range(_extra_id_group_indices))
  {
    auto & indices = _extra_id_group_indices[i];
    if (indices.empty())
      for (const auto & elem : _mesh.getMesh().active_element_ptr_range())
      {
        auto local_id = id(*elem, _extra_id_indices[i], _blocks_in_use && i == 0);
        if (std::find(indices.begin(), indices.end(), local_id) == indices.end())
          indices.push_back(local_id);
      }
  }

  // Allocate some vectors holding the volumes
  std::vector<Real> volumes;
  std::vector<std::vector<Real>> volumes_2d;
  std::vector<std::vector<std::vector<Real>>> volumes_3d;
  std::vector<std::vector<std::vector<std::vector<Real>>>> volumes_4d;

  // Default indexing: blocks, extra_id_name_1, extra_id_name_2 ...
  // Allocate vectors holding the positions
  if (_extra_id_names.size() == 1)
  {
    _positions.resize(_extra_id_group_indices[0].size());
    volumes.resize(_positions.size());
  }
  else if (_extra_id_names.size() == 2)
  {
    _positions_2d.resize(_extra_id_group_indices[0].size());
    volumes_2d.resize(_positions_2d.size());
    for (auto & pos_group : _positions_2d)
      pos_group.resize(_extra_id_group_indices[1].size());
    for (auto & vol_group : volumes_2d)
      vol_group.resize(_extra_id_group_indices[1].size());
  }
  else if (_extra_id_names.size() == 3)
  {
    _positions_3d.resize(_extra_id_group_indices[0].size());
    volumes_3d.resize(_positions_3d.size());
    for (auto & vec_pos_group : _positions_3d)
    {
      vec_pos_group.resize(_extra_id_group_indices[1].size());
      for (auto & pos_group : vec_pos_group)
        pos_group.resize(_extra_id_group_indices[2].size());
    }
    for (auto & vec_vol_group : volumes_3d)
    {
      vec_vol_group.resize(_extra_id_group_indices[1].size());
      for (auto & vol_group : vec_vol_group)
        vol_group.resize(_extra_id_group_indices[2].size());
    }
  }
  else if (_extra_id_names.size() == 4)
  {
    _positions_4d.resize(_extra_id_group_indices[0].size());
    volumes_4d.resize(_positions_4d.size());
    for (auto & vec_vec_pos_group : _positions_4d)
    {
      vec_vec_pos_group.resize(_extra_id_group_indices[1].size());
      for (auto & vec_pos_group : vec_vec_pos_group)
      {
        vec_pos_group.resize(_extra_id_group_indices[2].size());
        for (auto & pos_group : vec_pos_group)
          pos_group.resize(_extra_id_group_indices[3].size());
      }
    }
    for (auto & vec_vec_vol_group : volumes_4d)
    {
      vec_vec_vol_group.resize(_extra_id_group_indices[1].size());
      for (auto & vec_vol_group : vec_vec_vol_group)
      {
        vec_vol_group.resize(_extra_id_group_indices[2].size());
        for (auto & vol_group : vec_vol_group)
          vol_group.resize(_extra_id_group_indices[3].size());
      }
    }
  }
  else
    mooseError("Too much dimensionality for positions");

  // To simplify retrieving the final positions vector
  auto getNestedPositions =
      [this](const std::vector<unsigned int> & indices) -> std::vector<Point> &
  {
    mooseAssert(indices[_extra_id_names.size() - 1] == 0, "Indexing issue");
    if (_extra_id_names.size() == 1)
      return _positions;
    else if (_extra_id_names.size() == 2)
      return _positions_2d[indices[0]];
    else if (_extra_id_names.size() == 3)
      return _positions_3d[indices[0]][indices[1]];
    else
      return _positions_4d[indices[0]][indices[1]][indices[2]];
  };
  auto getNestedVolumes = [this, &volumes, &volumes_2d, &volumes_3d, &volumes_4d](
                              const std::vector<unsigned int> & indices) -> std::vector<Real> &
  {
    mooseAssert(indices[_extra_id_names.size() - 1] == 0, "Indexing issue");
    if (_extra_id_names.size() == 1)
      return volumes;
    else if (_extra_id_names.size() == 2)
      return volumes_2d[indices[0]];
    else if (_extra_id_names.size() == 3)
      return volumes_3d[indices[0]][indices[1]];
    else
      return volumes_4d[indices[0]][indices[1]][indices[2]];
  };

  std::vector<std::map<unsigned int, unsigned int>> positions_indexing(
      _extra_id_group_indices.size());

  // Make index maps to go from the extra element id to the positions array index
  for (const auto i : index_range(_extra_id_group_indices))
  {
    auto & indices = _extra_id_group_indices[i];
    auto j = 0;
    for (const auto extra_id : indices)
      positions_indexing[i][extra_id] = j++;
  }

  for (const auto & elem : _mesh.getMesh().active_element_ptr_range())
  {
    // Pre-compute the centroid, this is expensive but may be used for multiple element ids
    const auto centroid = elem->true_centroid();
    const auto volume = elem->volume();

    // Keeps track of indices in multi-D arrays
    std::vector<unsigned int> previous_indices(4);

    for (const auto i : index_range(_extra_id_names))
    {
      auto iter =
          positions_indexing[i].find(id(*elem, _extra_id_indices[i], _blocks_in_use && (i == 0)));
      if (iter == positions_indexing[i].end())
        break;
      else if (_extra_id_names.size() == i + 1)
      {
        getNestedPositions(previous_indices)[iter->second] += volume * centroid;
        getNestedVolumes(previous_indices)[iter->second] += volume;
        break;
      }
      else
        previous_indices[i] = iter->second;
    }
  }

  // Report the zero volumes
  unsigned int num_zeros = 0;
  if (_extra_id_names.size() == 1)
    for (const auto & vol : volumes)
      if (MooseUtils::absoluteFuzzyEqual(vol, 0))
        num_zeros++;
  if (_extra_id_names.size() == 2)
    for (const auto & vol_vec : volumes_2d)
      for (const auto & vol : vol_vec)
        if (MooseUtils::absoluteFuzzyEqual(vol, 0))
          num_zeros++;
  if (_extra_id_names.size() == 3)
    for (const auto & vol_vec_vec : volumes_3d)
      for (const auto & vol_vec : vol_vec_vec)
        for (const auto & vol : vol_vec)
          if (MooseUtils::absoluteFuzzyEqual(vol, 0))
            num_zeros++;
  if (_extra_id_names.size() == 4)
    for (const auto & vol_vec_vec_vec : volumes_4d)
      for (const auto & vol_vec_vec : vol_vec_vec_vec)
        for (const auto & vol_vec : vol_vec_vec)
          for (const auto & vol : vol_vec)
            if (MooseUtils::absoluteFuzzyEqual(vol, 0))
              num_zeros++;
  if (num_zeros)
    mooseWarning(std::to_string(num_zeros) +
                 " zero volume bins detected during group centroid position calculation. "
                 "The corresponding positions will be removed from consideration.");

  // Renormalize by the total bin volumes
  if (_extra_id_names.size() == 1)
    for (MooseIndex(_positions) i = 0; i < _positions.size(); i++)
    {
      if (volumes[i] != 0)
        _positions[i] /= volumes[i];
      else
      {
        _positions.erase(_positions.begin() + i);
        volumes.erase(volumes.begin() + i);
        i--;
      }
    }
  else if (_extra_id_names.size() == 2)
    for (const auto i : index_range(_positions_2d))
      for (MooseIndex(_positions) j = 0; j < _positions_2d[i].size(); j++)
      {
        if (volumes_2d[i][j] != 0)
          _positions_2d[i][j] /= volumes_2d[i][j];
        else
        {
          _positions_2d[i].erase(_positions.begin() + j);
          volumes_2d[i].erase(volumes.begin() + j);
          j--;
        }
      }
  else if (_extra_id_names.size() == 3)
    for (const auto i : index_range(_positions_3d))
      for (const auto j : index_range(_positions_3d[i]))
        for (MooseIndex(_positions) k = 0; k < _positions_3d[i][j].size(); k++)
        {
          if (volumes_3d[i][j][k] != 0)
            _positions_3d[i][j][k] /= volumes_3d[i][j][k];
          else
          {
            _positions_3d[i][j].erase(_positions_3d[i][j].begin() + k);
            volumes_3d[i][j].erase(volumes_3d[i][j].begin() + k);
            k--;
          }
        }
  else if (_extra_id_names.size() == 4)
    for (const auto i : index_range(_positions_4d))
      for (const auto j : index_range(_positions_4d[i]))
        for (const auto k : index_range(_positions_4d[i][j]))
          for (MooseIndex(_positions) l = 0; l < _positions_4d[i][j][k].size(); l++)
          {
            if (volumes_4d[i][j][k][l] != 0)
              _positions_4d[i][j][k][l] /= volumes_4d[i][j][k][l];
            else
            {
              _positions_4d[i][j][k].erase(_positions_4d[i][j][k].begin() + l);
              volumes_4d[i][j][k].erase(volumes_4d[i][j][k].begin() + l);
              l--;
            }
          }

  // Fill the 1D position vector
  unrollMultiDPositions();
  _initialized = true;
}

unsigned int
ElementGroupCentroidPositions::id(const Elem & elem, unsigned int id_index, bool use_subdomains)
{
  mooseAssert(!use_subdomains || (id_index == std::numeric_limits<unsigned short>::max()),
              "We do not expect a valid element extra integer index for subdomains");
  if (use_subdomains)
    return elem.subdomain_id();
  else
    return elem.get_extra_integer(id_index);
}
