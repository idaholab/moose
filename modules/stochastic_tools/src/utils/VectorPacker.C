//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorPacker.h"
#include "SerializerGuard.h"
#include "libmesh/parallel_algebra.h"
#include "libmesh/parallel_sync.h"

namespace libMesh
{
namespace Parallel
{

unsigned int
Packing<std::tuple<dof_id_type, dof_id_type, std::shared_ptr<DenseVector<Real>>>>::packable_size(
    const std::tuple<dof_id_type, dof_id_type, std::shared_ptr<DenseVector<Real>>> & object,
    const void *)
{
  unsigned int total_size = 0;

  total_size += 3;

  total_size += std::get<2>(object)->size();

  return total_size;
}

unsigned int
Packing<std::tuple<dof_id_type, dof_id_type, std::shared_ptr<DenseVector<Real>>>>::packed_size(
    typename std::vector<Real>::const_iterator in)
{
  unsigned int total_size = 0;

  total_size += *in++;

  total_size += 3;

  return total_size;
}

template <>
void
Packing<std::tuple<dof_id_type, dof_id_type, std::shared_ptr<DenseVector<Real>>>>::pack(
    const std::tuple<dof_id_type, dof_id_type, std::shared_ptr<DenseVector<Real>>> & object,
    std::back_insert_iterator<std::vector<Real>> data_out,
    const void *)
{
  // Data size
  const auto & dense_vector = std::get<2>(object);
  data_out = dense_vector->size();
  // IDs
  data_out = std::get<0>(object);
  data_out = std::get<1>(object);
  // Data
  const auto & vector = dense_vector->get_values();
  for (std::size_t i = 0; i < dense_vector->size(); ++i)
    data_out = vector[i];
}

template <>
std::tuple<dof_id_type, dof_id_type, std::shared_ptr<DenseVector<Real>>>
Packing<std::tuple<dof_id_type, dof_id_type, std::shared_ptr<DenseVector<Real>>>>::unpack(
    std::vector<Real>::const_iterator in, void *)
{
  // Number of points
  const std::size_t data_size = *in++;
  std::tuple<dof_id_type, dof_id_type, std::shared_ptr<DenseVector<Real>>> object;
  // IDs
  std::get<0>(object) = *in++;
  std::get<1>(object) = *in++;
  // Data
  auto & dense_vector = std::get<2>(object);
  dense_vector = std::make_shared<DenseVector<Real>>(data_size);
  auto & vector_values = dense_vector->get_values();
  for (std::size_t i = 0; i < data_size; ++i)
    vector_values[i] = *in++;
  return object;
}

} // namespace Parallel
} // namespace libMesh
