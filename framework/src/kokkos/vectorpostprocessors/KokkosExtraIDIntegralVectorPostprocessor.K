//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosExtraIDIntegralVectorPostprocessor.h"

#include "MooseVariable.h"
#include "MooseMesh.h"
#include "MooseMeshUtils.h"

#include "libmesh/mesh_base.h"

registerKokkosUserObject("MooseApp", KokkosExtraIDIntegralVectorPostprocessor);
registerKokkosReducer("MooseApp", KokkosExtraIDIntegralVectorPostprocessor);

InputParameters
KokkosExtraIDIntegralVectorPostprocessor::validParams()
{
  InputParameters params = ElementVectorPostprocessor::validParams();
  params.addCoupledVar("variable",
                       "The names of the variables that this VectorPostprocessor operates on");
  params.addParam<std::vector<MaterialPropertyName>>(
      "mat_prop", "The names of material properties that this VectorPostprocessor operates on");
  params.addRequiredParam<std::vector<ExtraElementIDName>>(
      "id_name", "List of extra element ID names by which to separate integral(s).");
  params.addParam<bool>("average", false, "Whether or not to compute volume average");
  MooseEnum mode("atomic reduction", "atomic");
  params.addParam<MooseEnum>("calculation_mode",
                             mode,
                             "Whether to use atomic addition or reduction for parallel summation");
  params.addClassDescription("Integrates or averages variables based on extra element IDs");
  return params;
}

KokkosExtraIDIntegralVectorPostprocessor::KokkosExtraIDIntegralVectorPostprocessor(
    const InputParameters & parameters)
  : ElementVectorPostprocessor(parameters),
    _mesh(_fe_problem.mesh()),
    _average(getParam<bool>("average")),
    _mode(getParam<MooseEnum>("calculation_mode")),
    _nvar(isParamValid("variable") ? coupledComponents("variable") : 0),
    _nprop(isParamValid("mat_prop") ? getParam<std::vector<MaterialPropertyName>>("mat_prop").size()
                                    : 0),
    _prop_names(isParamValid("mat_prop") ? getParam<std::vector<MaterialPropertyName>>("mat_prop")
                                         : std::vector<MaterialPropertyName>()),
    _extra_id(getParam<std::vector<ExtraElementIDName>>("id_name")),
    _n_extra_id(_extra_id.size())
{
  if (!_nvar && !_nprop)
    mooseError("Neither 'variable' nor 'mat_prop' was specified.");

  if (_nvar + _nprop > _cache_size)
    mooseError("Cannot have more than ",
               _cache_size,
               " variables and material properties at the same time.");

  // create map of element ids to parsed vpp ids
  _unique_vpp_id_map =
      MooseMeshUtils::getExtraIDUniqueCombinationMap(_mesh.getMesh(), blockIDs(), _extra_id);

  // set up variable vector containing parsed extra ids
  for (unsigned int i = 0; i < _n_extra_id; ++i)
  {
    const auto id_type = _extra_id[i];
    auto & p = declareVector("Level-" + std::to_string(i) + "-" + id_type);
    // collect local map of local vpp id to extra id
    std::map<dof_id_type, dof_id_type> extra_ids;
    for (const auto elem : _mesh.getMesh().active_local_element_ptr_range())
    {
      if (!hasBlocks(elem->subdomain_id()))
        continue;
      auto vpp_id = _unique_vpp_id_map[elem->id()];
      if (extra_ids.find(vpp_id) == extra_ids.end())
        extra_ids[vpp_id] = elem->get_extra_integer(getElementIDIndexByName(id_type));
    }
    // create global map of local vpp id to extra id
    comm().set_union(extra_ids);
    p.resize(extra_ids.size());
    for (auto it : extra_ids)
      p[it.first] = it.second;
    _extra_ids.push_back(&p);
  }

  _vector_size = _extra_ids[0]->size();

  std::vector<std::string> vector_names;

  _integrals.create(_nvar + _prop_names.size());
  _props.create(_prop_names.size());

  unsigned int i = 0;

  // declare vectors containing variable integral values
  for (; i < _nvar; ++i)
  {
    vector_names.push_back(getVar("variable", i)->name());
    auto & p = declareVector(vector_names.back());
    p.resize(_vector_size);
    if (_mode == "atomic")
      _integrals[i].createDevice(p.size());
    else
      _integrals[i].init(p.size());
    _integrals[i].aliasHost(p.data());
  }

  if (_nvar)
    _var_values = kokkosCoupledValues("variable");

  // declare vectors containing material property integral values
  for (unsigned int iprop = 0; iprop < _nprop; ++iprop, ++i)
  {
    vector_names.push_back(_prop_names[iprop]);
    _props[iprop] = getKokkosMaterialPropertyByName<Real>(vector_names.back());
    auto & p = declareVector(vector_names.back());
    p.resize(_vector_size);
    if (_mode == "atomic")
      _integrals[i].createDevice(p.size());
    else
      _integrals[i].init(p.size());
    _integrals[i].aliasHost(p.data());
  }

  _integrals.copyToDevice();
  _props.copyToDevice();
}

void
KokkosExtraIDIntegralVectorPostprocessor::initialSetup()
{
  _unique_vpp_ids.create(_unique_vpp_id_map.size());

  for (const auto elem : _mesh.getMesh().active_local_element_ptr_range())
    _unique_vpp_ids[kokkosMesh().getContiguousElementID(elem)] = _unique_vpp_id_map[elem->id()];

  _unique_vpp_ids.moveToDevice();
}

void
KokkosExtraIDIntegralVectorPostprocessor::initialize()
{
  if (_mode == "atomic")
  {
    for (auto & integral : _integrals)
      integral = 0;

    if (_average)
    {
      _volumes.create(_vector_size);
      _volumes = 0;
    }
  }
  else
    allocateReductionBuffer((_integrals.size() + _average) * _vector_size);
}

void
KokkosExtraIDIntegralVectorPostprocessor::compute()
{
  if (_mode == "atomic")
    dispatch();
  else
    reduce();
}

void
KokkosExtraIDIntegralVectorPostprocessor::finalize()
{
  if (_mode == "atomic")
  {
    for (auto & integral : _integrals)
    {
      integral.copyToHost();

      libmesh_call_mpi(MPI_Allreduce(MPI_IN_PLACE,
                                     integral.data(),
                                     integral.size(),
                                     libMesh::Parallel::StandardType<Real>(),
                                     libMesh::Parallel::OpFunction<Real>::sum(),
                                     comm().get()));
    }

    if (_average)
    {
      _volumes.copyToHost();

      libmesh_call_mpi(MPI_Allreduce(MPI_IN_PLACE,
                                     _volumes.data(),
                                     _volumes.size(),
                                     libMesh::Parallel::StandardType<Real>(),
                                     libMesh::Parallel::OpFunction<Real>::sum(),
                                     comm().get()));

      for (auto & integral : _integrals)
        for (unsigned int i = 0; i < integral.size(); ++i)
          integral[i] /= _volumes[i];
    }
  }
  else
  {
    libmesh_call_mpi(MPI_Allreduce(MPI_IN_PLACE,
                                   _reduction_buffer.data(),
                                   _reduction_buffer.size(),
                                   libMesh::Parallel::StandardType<Real>(),
                                   libMesh::Parallel::OpFunction<Real>::sum(),
                                   comm().get()));

    for (unsigned int i = 0; i < _integrals.size(); ++i)
      for (unsigned int j = 0; j < _vector_size; ++j)
      {
        _integrals[i][j] = _reduction_buffer[j + i * _vector_size];

        if (_average)
          _integrals[i][j] /= _reduction_buffer[j + _integrals.size() * _vector_size];
      }
  }
}
