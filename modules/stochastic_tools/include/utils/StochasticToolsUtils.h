//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseUtils.h"
#include "libmesh/communicator.h"

namespace StochasticTools
{

/**
 * Custom type trait that has a ::value of true for types that can be gathered
 */
template <typename T>
struct canDefaultGather
{
  static constexpr bool value = false;
};
template <typename T>
struct canDefaultGather<std::vector<T>>
{
  static constexpr bool value = std::is_base_of<TIMPI::DataType, TIMPI::StandardType<T>>::value;
};
template <typename T>
struct canStochasticGather
{
  static constexpr bool value = false;
};
template <typename T>
struct canStochasticGather<std::vector<T>>
{
  static constexpr bool value = canStochasticGather<T>::value ||
                                std::is_base_of<TIMPI::DataType, TIMPI::StandardType<T>>::value ||
                                std::is_same<T, std::string>::value || std::is_same<T, bool>::value;
};

/*
 * Methods for gathering nested vectors
 */
template <typename T>
void
stochasticGather(const libMesh::Parallel::Communicator &, processor_id_type, T &)
{
  ::mooseError("Cannot gather values of type ", MooseUtils::prettyCppType<T>());
}
template <typename T,
          typename std::enable_if<canDefaultGather<std::vector<T>>::value, int>::type = 0>
void
stochasticGather(const libMesh::Parallel::Communicator & comm,
                 processor_id_type root_id,
                 std::vector<T> & val)
{
  comm.gather(root_id, val);
}
template <
    typename T,
    typename std::enable_if<canStochasticGather<std::vector<std::vector<T>>>::value, int>::type = 0>
void
stochasticGather(const libMesh::Parallel::Communicator & comm,
                 processor_id_type root_id,
                 std::vector<std::vector<T>> & val)
{
  // Get local vector sizes
  std::size_t num_local_vecs = val.size();
  std::vector<std::size_t> val_sizes;
  val_sizes.reserve(num_local_vecs);
  std::size_t num_local_vals = 0;
  for (const auto & v : val)
  {
    val_sizes.push_back(v.size());
    num_local_vals += v.size();
  }

  // Flatten the local vector of vectors
  std::vector<T> val_exp;
  val_exp.reserve(num_local_vals);
  for (auto & v : val)
    std::copy(v.begin(), v.end(), std::back_inserter(val_exp));

  // Gather the vector sizes and the flattened vector
  comm.gather(root_id, val_sizes);
  stochasticGather(comm, root_id, val_exp);

  // Build the vector of vectors from the gathered flatten vector
  if (comm.rank() == root_id)
  {
    val.resize(val_sizes.size());
    std::size_t ind = num_local_vals;
    for (std::size_t i = num_local_vecs; i < val_sizes.size(); ++i)
    {
      val[i].resize(val_sizes[i]);
      std::move(val_exp.begin() + ind, val_exp.begin() + ind + val_sizes[i], val[i].begin());
      ind += val_sizes[i];
    }
  }
}
// Gathering a vector of strings hasn't been implemented in libMesh, so just gonna do it the hard
// way
template <typename T>
void
stochasticGather(const libMesh::Parallel::Communicator & comm,
                 processor_id_type root_id,
                 std::vector<std::basic_string<T>> & val)
{
  std::vector<std::basic_string<T>> val_gath = val;
  comm.allgather(val_gath);
  if (comm.rank() == root_id)
    val = std::move(val_gath);
}
// Gathering bool is weird
template <typename A>
void
stochasticGather(const libMesh::Parallel::Communicator & comm,
                 processor_id_type root_id,
                 std::vector<bool, A> & val)
{
  std::vector<unsigned short int> temp(val.size());
  for (std::size_t i = 0; i < val.size(); ++i)
    temp[i] = val[i] ? 1 : 0;
  comm.gather(root_id, temp);
  if (comm.rank() == root_id)
  {
    val.resize(temp.size());
    for (std::size_t i = 0; i < temp.size(); ++i)
      val[i] = temp[i] == 1;
  }
}

/*
 * Methods for gathering nested vectors on all processors
 */
template <typename T>
void
stochasticAllGather(const libMesh::Parallel::Communicator &, T &)
{
  ::mooseError("Cannot gather values of type ", MooseUtils::prettyCppType<T>());
}
template <typename T,
          typename std::enable_if<canDefaultGather<std::vector<T>>::value, int>::type = 0>
void
stochasticAllGather(const libMesh::Parallel::Communicator & comm, std::vector<T> & val)
{
  comm.allgather(val);
}
template <
    typename T,
    typename std::enable_if<canStochasticGather<std::vector<std::vector<T>>>::value, int>::type = 0>
void
stochasticAllGather(const libMesh::Parallel::Communicator & comm, std::vector<std::vector<T>> & val)
{
  // Get local vector sizes
  std::size_t num_local_vecs = val.size();
  std::vector<std::size_t> val_sizes;
  val_sizes.reserve(num_local_vecs);
  std::size_t num_local_vals = 0;
  for (const auto & v : val)
  {
    val_sizes.push_back(v.size());
    num_local_vals += v.size();
  }

  // Flatten the local vector of vectors
  std::vector<T> val_exp;
  val_exp.reserve(num_local_vals);
  for (auto & v : val)
    std::copy(v.begin(), v.end(), std::back_inserter(val_exp));

  // Gather the vector sizes and the flattened vector
  comm.allgather(val_sizes);
  stochasticAllGather(comm, val_exp);

  // Build the vector of vectors from the gathered flatten vector
  val.resize(val_sizes.size());
  std::size_t ind = 0;
  for (std::size_t i = 0; i < val_sizes.size(); ++i)
  {
    val[i].resize(val_sizes[i]);
    std::move(val_exp.begin() + ind, val_exp.begin() + ind + val_sizes[i], val[i].begin());
    ind += val_sizes[i];
  }
}
// Gathering a vector of strings hasn't been implemented in libMesh, so just gonna do it the hard
// way
template <typename T>
void
stochasticAllGather(const libMesh::Parallel::Communicator & comm,
                    std::vector<std::basic_string<T>> & val)
{
  comm.allgather(val);
}
// Gathering bool is weird
template <typename A>
void
stochasticAllGather(const libMesh::Parallel::Communicator & comm, std::vector<bool, A> & val)
{
  std::vector<unsigned short int> temp(val.size());
  for (std::size_t i = 0; i < val.size(); ++i)
    temp[i] = val[i] ? 1 : 0;
  comm.allgather(temp);
  val.resize(temp.size());
  for (std::size_t i = 0; i < temp.size(); ++i)
    val[i] = temp[i] == 1;
}

/*
 * Methods for sorting vectors of vectors with elements inplace. For example:
 * {{7, 5, 2}, {3, 8, 6}, {9, 9, 1}} -> {{3, 5, 1}, {7, 8, 2}, {9, 9, 6}}
 */
template <typename T>
void
inplaceSort(std::vector<T> & values)
{
  std::sort(values.begin(), values.end());
}
template <typename T>
void
inplaceSort(std::vector<std::vector<T>> & values)
{
  if (values.empty())
    return;

  const std::size_t sz = values[0].size();
  mooseAssert(std::find_if(values.begin(),
                           values.end(),
                           [&sz](const std::vector<T> & val)
                           { return val.size() != sz; }) == values.end(),
              "All vectors must be same size to sort.");

  std::vector<T> vals(values.size());
  for (const auto & i : make_range(sz))
  {
    for (const auto & k : index_range(values))
      vals[k] = values[k][i];
    inplaceSort(vals);
    for (const auto & k : index_range(values))
      values[k][i] = std::move(vals[k]);
  }
}

/**
 * Reshape a vector into matrix-like vector of vectors
 *
 * @param vec Input vector to reshape
 * @param n Leading dimension size,
            number of columns if row-major, number of rows if column-major
 * @param row_major True if @param vec is in row-major format
 *                  see https://en.wikipedia.org/wiki/Row-_and_column-major_order
 * @return vector of vectors representing reshaped vector ([row][col])
 */
template <typename T>
std::vector<std::vector<T>>
reshapeVector(const std::vector<T> & vec, std::size_t n, bool row_major)
{
  const auto nelem = vec.size();
  const auto nrow = row_major ? nelem / n : n;
  const auto ncol = row_major ? n : nelem / n;
  if (nelem % n != 0)
    ::mooseError(
        "Reshaping dimensions (", nrow, ", ", ncol, ") does not match vector size (", nelem, ").");

  std::vector<std::vector<T>> mat(nrow, std::vector<T>(ncol));
  for (const auto & i : make_range(nrow))
    for (const auto & j : make_range(ncol))
    {
      const auto k = row_major ? (i * ncol + j) : (j * nrow + i);
      mat[i][j] = vec[k];
    }
  return mat;
}

} // StochasticTools namespace
