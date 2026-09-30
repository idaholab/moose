//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "THMUtils.h"
#include "MooseUtils.h"
#include "MooseTypes.h"
#include "MooseApp.h"
#include "TransientBase.h"
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

std::string
parseConnectedComponentName(const BoundaryName & connection)
{
  const auto colon_pos = connection.rfind(':');
  if (colon_pos == std::string::npos)
    mooseError("Invalid connection '",
               connection,
               "'. Valid connection format is 'component_name:in' or 'component_name:out'.");
  return connection.substr(0, colon_pos);
}

Real
parseConnectionNormal(const BoundaryName & connection)
{
  const auto colon_pos = connection.rfind(':');
  const auto end_type =
      colon_pos == std::string::npos ? std::string() : connection.substr(colon_pos + 1);
  if (end_type == "in")
    return -1.0;
  else if (end_type == "out")
    return 1.0;
  else
    mooseError("Invalid connection '",
               connection,
               "'. The end type ('",
               end_type,
               "') must be 'in' or 'out'.");
}

bool
implicitTimeIntegrationFlag(MooseApp & app)
{
  const auto trex = dynamic_cast<TransientBase *>(app.getExecutioner());
  if (!trex)
    return true;

  const auto ti_type = trex->getTimeScheme();
  return !(ti_type == Moose::TI_EXPLICIT_TVD_RK_2 || ti_type == Moose::TI_EXPLICIT_MIDPOINT ||
           ti_type == Moose::TI_EXPLICIT_EULER);
}
}
