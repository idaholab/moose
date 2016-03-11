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

template<>
InputParameters validParams<SamplerBase>()
{
  InputParameters params = emptyInputParameters();

  MooseEnum sort_options("x y z id");
  params.addRequiredParam<MooseEnum>("sort_by", sort_options, "What to sort the samples by");

  return params;
}

SamplerBase::SamplerBase(const InputParameters & parameters, VectorPostprocessor * vpp, const libMesh::Parallel::Communicator & comm) :
    _sampler_params(parameters),
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

  _values.resize(variable_names.size());
  _values_tmp.resize(variable_names.size());

  for (unsigned int i=0; i<variable_names.size(); i++)
    _values[i] = &_vpp->declareVector(variable_names[i]);
}

void
SamplerBase::addSample(const Point & p, const Real & id, const std::vector<Real> & values)
{
  _x_tmp.push_back(p(0));
  _y_tmp.push_back(p(1));
  _z_tmp.push_back(p(2));

  _id_tmp.push_back(id);

  for (unsigned int i=0; i<_variable_names.size(); i++)
    _values_tmp[i].push_back(values[i]);
}

void
SamplerBase::initialize()
{
  _x_tmp.clear();
  _y_tmp.clear();
  _z_tmp.clear();
  _id_tmp.clear();

  for (unsigned int i=0; i<_variable_names.size(); i++)
    _values_tmp[i].clear();
}

void
SamplerBase::finalize()
{
  // Get the values from everywhere
  _comm.allgather(_x_tmp, false);
  _comm.allgather(_y_tmp, false);
  _comm.allgather(_z_tmp, false);
  _comm.allgather(_id_tmp, false);

  for (unsigned int i=0; i<_variable_names.size(); i++)
    _comm.allgather(_values_tmp[i], false);

  // Next... figure out the correct sorted positions of each value
  std::vector<size_t> sorted_indices;

  switch (_sort_by)
  {
    case 0: // x
      Moose::indirectSort(_x_tmp.begin(), _x_tmp.end(), sorted_indices);
      break;
    case 1: // y
      Moose::indirectSort(_y_tmp.begin(), _y_tmp.end(), sorted_indices);
      break;
    case 2: // z
      Moose::indirectSort(_z_tmp.begin(), _z_tmp.end(), sorted_indices);
      break;
    case 3: // id
      Moose::indirectSort(_id_tmp.begin(), _id_tmp.end(), sorted_indices);
      break;
  }

  // Use the sorted_indices to copy out of the temporary vectors and into the actual output vectors
  for (unsigned int i=0; i<_variable_names.size(); i++)
  {
    _values[i]->resize(sorted_indices.size());

    for (unsigned int j=0; j<sorted_indices.size(); j++)
      (*_values[i])[j] = _values_tmp[i][sorted_indices[j]];
  }

  _x.resize(sorted_indices.size());
  _y.resize(sorted_indices.size());
  _z.resize(sorted_indices.size());
  _id.resize(sorted_indices.size());

  for (unsigned int i=0; i<sorted_indices.size(); i++)
  {
    size_t index = sorted_indices[i];

    _x[i] = _x_tmp[index];
    _y[i] = _y_tmp[index];
    _z[i] = _z_tmp[index];
    _id[i] = _id_tmp[index];
  }
}

void
SamplerBase::threadJoin(const SamplerBase & y)
{
  _x_tmp.insert(_x_tmp.end(), y._x_tmp.begin(), y._x_tmp.end());
  _y_tmp.insert(_y_tmp.end(), y._y_tmp.begin(), y._y_tmp.end());
  _z_tmp.insert(_z_tmp.end(), y._z_tmp.begin(), y._z_tmp.end());

  _id_tmp.insert(_id_tmp.end(), y._id_tmp.begin(), y._id_tmp.end());

  for (unsigned int i=0; i<_variable_names.size(); i++)
    _values_tmp[i].insert(_values_tmp[i].end(), y._values_tmp[i].begin(), y._values_tmp[i].end());
}
