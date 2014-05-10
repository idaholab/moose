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

#include "NodalValueSampler.h"

template<>
InputParameters validParams<NodalValueSampler>()
{
  InputParameters params = validParams<NodalVariableVectorPostprocessor>();

  MooseEnum sort_options("x,y,z,id");

  params.addRequiredParam<MooseEnum>("sort_by", sort_options, "What to sort the samples by");

  return params;
}

NodalValueSampler::NodalValueSampler(const std::string & name, InputParameters parameters) :
    NodalVariableVectorPostprocessor(name, parameters),
    _sort_by(getParam<MooseEnum>("sort_by")),
    _x(registerVector("x")),
    _y(registerVector("y")),
    _z(registerVector("z")),
    _id(registerVector("id"))
{
  _values.resize(_coupled_moose_vars.size());
  _values_tmp.resize(_coupled_moose_vars.size());

  for (unsigned int i=0; i<_coupled_moose_vars.size(); i++)
    _values[i] = &registerVector(_coupled_moose_vars[i]->name());
}

void
NodalValueSampler::initialize()
{
  _x_tmp.clear();
  _y_tmp.clear();
  _z_tmp.clear();
  _id_tmp.clear();

  for (unsigned int i=0; i<_coupled_moose_vars.size(); i++)
    _values_tmp[i].clear();
}

void
NodalValueSampler::execute()
{
  _x_tmp.push_back((*_current_node)(0));
  _y_tmp.push_back((*_current_node)(1));
  _z_tmp.push_back((*_current_node)(2));

  _id_tmp.push_back(_current_node->id());

  for (unsigned int i=0; i<_coupled_moose_vars.size(); i++)
    _values_tmp[i].push_back(_coupled_moose_vars[i]->nodalSln()[_qp]);
}

void
NodalValueSampler::finalize()
{
  // Get the values from everywhere
  _communicator.allgather(_x_tmp, false);
  _communicator.allgather(_y_tmp, false);
  _communicator.allgather(_z_tmp, false);
  _communicator.allgather(_id_tmp, false);

  for (unsigned int i=0; i<_coupled_moose_vars.size(); i++)
    _communicator.allgather(_values_tmp[i], false);

  std::vector<unsigned int> sorted_indices;

  switch(_sort_by)
  {
    case 0: // x
      MooseUtils::getSortIndices(_x_tmp, std::less<Real>(), sorted_indices);
      break;
    case 1: // y
      MooseUtils::getSortIndices(_y_tmp, std::less<Real>(), sorted_indices);
      break;
    case 2: // z
      MooseUtils::getSortIndices(_z_tmp, std::less<Real>(), sorted_indices);
      break;
    case 3: // id
      MooseUtils::getSortIndices(_id_tmp, std::less<Real>(), sorted_indices);
      break;
  }

  for (unsigned int i=0; i<_coupled_moose_vars.size(); i++)
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
    unsigned int index = sorted_indices[i];

    _x[i] = _x_tmp[index];
    _y[i] = _y_tmp[index];
    _z[i] = _z_tmp[index];
    _id[i] = _id_tmp[index];
  }
}

void
NodalValueSampler::threadJoin(const UserObject & y)
{
  const NodalValueSampler & vpp = static_cast<const NodalValueSampler &>(y);

  _x_tmp.insert(_x_tmp.end(), vpp._x_tmp.begin(), vpp._x_tmp.end());
  _y_tmp.insert(_y_tmp.end(), vpp._y_tmp.begin(), vpp._y_tmp.end());
  _z_tmp.insert(_z_tmp.end(), vpp._z_tmp.begin(), vpp._z_tmp.end());

  _id_tmp.insert(_id_tmp.end(), vpp._id_tmp.begin(), vpp._id_tmp.end());

  for (unsigned int i=0; i<_coupled_moose_vars.size(); i++)
    _values_tmp[i].insert(_values_tmp[i].end(), vpp._values_tmp[i].begin(), vpp._values_tmp[i].end());
}
