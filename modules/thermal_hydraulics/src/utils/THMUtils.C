//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "THMUtils.h"
#include "MooseUtils.h"
#include "MooseTypes.h"
#include "libmesh/vector_value.h"
#include "metaphysicl/parallel_dualnumber.h"
#include "metaphysicl/parallel_numberarray.h"
#include "metaphysicl/parallel_semidynamicsparsenumberarray.h"

namespace THM
{

void
computeOrthogonalDirections(const RealVectorValue & n_unnormalized,
                            RealVectorValue & t1,
                            RealVectorValue & t2)
{
  const RealVectorValue n = n_unnormalized.unit();

  if (MooseUtils::absoluteFuzzyEqual(std::abs(n(0)), 1.0))
  {
    t1 = RealVectorValue(0, 1, 0);
    t2 = RealVectorValue(0, 0, 1);
  }
  else
  {
    // Gram-Schmidt process to get first
    RealVectorValue ex(1, 0, 0);
    t1 = ex - (ex * n) * n;
    t1 = t1.unit();

    // use cross-product to get second
    t2 = n.cross(t1);
    t2 = t2.unit();
  }
}

void
allGatherADVectorMap(const Parallel::Communicator & comm,
                     std::map<dof_id_type, std::vector<ADReal>> & this_map)
{
  std::vector<std::map<dof_id_type, std::vector<ADReal>>> all_maps;
  comm.allgather(this_map, all_maps);
  for (auto & one_map : all_maps)
    for (auto & it : one_map)
      this_map[it.first] = it.second;
}

void
allGatherADVectorMapSum(const Parallel::Communicator & comm,
                        std::map<dof_id_type, std::vector<ADReal>> & this_map)
{
  std::vector<std::map<dof_id_type, std::vector<ADReal>>> all_maps;
  comm.allgather(this_map, all_maps);
  this_map.clear();
  for (auto & one_map : all_maps)
    for (auto & it : one_map)
      if (this_map.find(it.first) == this_map.end())
        this_map[it.first] = it.second;
      else
      {
        auto & existing = this_map[it.first];
        for (const auto i : index_range(existing))
          existing[i] += it.second[i];
      }
}
}
