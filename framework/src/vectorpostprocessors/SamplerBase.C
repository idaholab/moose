/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

// MOOSE includes
#include "SamplerBase.h"
#include "IndirectSort.h"
#include "VectorPostprocessor.h"

template <>
InputParameters
validParams<SamplerBase>()
{
  InputParameters params = emptyInputParameters();

  MooseEnum sort_options("x y z id");
  params.addRequiredParam<MooseEnum>("sort_by", sort_options, "What to sort the samples by");

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
  for (auto i = beginIndex(values); i < values.size(); ++i)
    _values[i]->emplace_back(values[i]);
}

void
SamplerBase::initialize()
{
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
  for (auto i = beginIndex(vec_ptrs); i < vec_ptrs.size(); ++i)
  {
    for (auto j = beginIndex(sorted_indices); j < sorted_indices.size(); ++j)
      tmp_vector[j] = (*vec_ptrs[i])[sorted_indices[j]];

    // Swap vector storage with sorted vector
    vec_ptrs[i]->swap(tmp_vector);
  }
}

void
SamplerBase::threadJoin(const SamplerBase & y)
{
  _x.insert(_x.end(), y._x.begin(), y._x.end());
  _y.insert(_y.end(), y._y.begin(), y._y.end());
  _z.insert(_z.end(), y._z.begin(), y._z.end());

  _id.insert(_id.end(), y._id.begin(), y._id.end());

  for (unsigned int i = 0; i < _variable_names.size(); i++)
    _values[i]->insert(_values[i]->end(), y._values[i]->begin(), y._values[i]->end());
}
