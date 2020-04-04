//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SamplerBase.h"

// MOOSE includes
#include "IndirectSort.h"
#include "InputParameters.h"
#include "MooseEnum.h"
#include "MooseError.h"
#include "VectorPostprocessor.h"

#include "libmesh/point.h"

InputParameters
SamplerBase::validParams()
{
  InputParameters params = emptyInputParameters();

  MooseEnum sort_options("x y z id");
  params.addRequiredParam<MooseEnum>("sort_by", sort_options, "What to sort the samples by");

  // The value from this VPP is naturally already on every processor
  // TODO: Make this not the case!  See #11415
  params.set<bool>("_auto_broadcast") = false;

  return params;
}

SamplerBase::SamplerBase(const InputParameters & parameters,
                         VectorPostprocessor * vpp,
                         const libMesh::Parallel::Communicator & comm)
  : _sampler_params(parameters),
    _vpp(vpp),
    _comm(comm),
    _sort_by(parameters.get<MooseEnum>("sort_by")),
    _x(vpp->declareVector("x")),
    _y(vpp->declareVector("y")),
    _z(vpp->declareVector("z")),
    _id(vpp->declareVector("id"))
{
}

void
SamplerBase::setupVariables(const std::vector<std::string> & variable_names)
{
  _variable_names = variable_names;
  _values.reserve(variable_names.size());

  for (const auto & variable_name : variable_names)
    _values.push_back(&_vpp->declareVector(variable_name));
}

void
SamplerBase::addSample(const Point & p, const Real & id, const std::vector<Real> & values)
{
  _x.push_back(p(0));
  _y.push_back(p(1));
  _z.push_back(p(2));

  _id.push_back(id);

  mooseAssert(values.size() == _variable_names.size(), "Mismatch of variable names to vector size");
  for (MooseIndex(values) i = 0; i < values.size(); ++i)
    _values[i]->emplace_back(values[i]);
}

void
SamplerBase::initialize()
{
  // Don't reset the vectors if we want to retain history
  if (_vpp->containsCompleteHistory() && _comm.rank() == 0)
    return;

  _x.clear();
  _y.clear();
  _z.clear();
  _id.clear();

  std::for_each(
      _values.begin(), _values.end(), [](VectorPostprocessorValue * vec) { vec->clear(); });
}

void
SamplerBase::finalize()
{
  /**
   * We have several vectors that all need to be processed in the same way.
   * Rather than enumerate them all, let's just create a vector of pointers
   * and work on them that way.
   */
  constexpr auto NUM_ID_VECTORS = 4;

  std::vector<VectorPostprocessorValue *> vec_ptrs;
  vec_ptrs.reserve(_values.size() + NUM_ID_VECTORS);
  // Initialize the pointer vector with the position and ID vectors
  vec_ptrs = {{&_x, &_y, &_z, &_id}};
  // Now extend the vector by all the remaining values vector before processing
  vec_ptrs.insert(vec_ptrs.end(), _values.begin(), _values.end());

  // Gather up each of the partial vectors
  for (auto vec_ptr : vec_ptrs)
    _comm.allgather(*vec_ptr, /* identical buffer lengths = */ false);

  // Now create an index vector by using an indirect sort
  std::vector<std::size_t> sorted_indices;
  Moose::indirectSort(vec_ptrs[_sort_by]->begin(), vec_ptrs[_sort_by]->end(), sorted_indices);

  /**
   * We now have one sorted vector. The remaining vectors need to be sorted according to that
   * vector.
   * We'll need a temporary vector to hold values as we map them according to the sorted indices.
   * After that, we'll swap the vector contents with the sorted vector to get the values
   * back into the original vector.
   */
  // This vector is used as temp storage to sort each of the remaining vectors according to the
  // first
  auto vector_length = sorted_indices.size();
  VectorPostprocessorValue tmp_vector(vector_length);

#ifndef NDEBUG
  for (const auto vec_ptr : vec_ptrs)
    if (vec_ptr->size() != vector_length)
      mooseError("Vector length mismatch");
#endif

  // Sort each of the vectors using the same sorted indices
  for (auto & vec_ptr : vec_ptrs)
  {
    for (MooseIndex(sorted_indices) j = 0; j < sorted_indices.size(); ++j)
      tmp_vector[j] = (*vec_ptr)[sorted_indices[j]];

    // Swap vector storage with sorted vector
    vec_ptr->swap(tmp_vector);
  }
}

void
SamplerBase::threadJoin(const SamplerBase & y)
{
  _x.insert(_x.end(), y._x.begin(), y._x.end());
  _y.insert(_y.end(), y._y.begin(), y._y.end());
  _z.insert(_z.end(), y._z.begin(), y._z.end());

  _id.insert(_id.end(), y._id.begin(), y._id.end());

  for (MooseIndex(_variable_names) i = 0; i < _variable_names.size(); i++)
    _values[i]->insert(_values[i]->end(), y._values[i]->begin(), y._values[i]->end());
}
