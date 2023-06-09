//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RestartableEquationSystems.h"

#include "DataIO.h"

#include "libmesh/dof_map.h"
#include "libmesh/dof_object.h"

RestartableEquationSystems::RestartableEquationSystems(libMesh::MeshBase & mesh,
                                                       const bool skip_additional_vectors)
  : _es(mesh), _skip_additional_vectors(skip_additional_vectors)
{
}

RestartableEquationSystems::EquationSystemsHeader
RestartableEquationSystems::buildHeader() const
{
  EquationSystemsHeader es_header;

  // Systems
  for (const auto sys_num : make_range(_es.n_systems()))
  {
    const auto & sys = _es.get_system(sys_num);

    SystemHeader sys_header;
    sys_header.name = sys.name();
    sys_header.type = sys.system_type();
    sys_header.sys = &sys;

    // Variables in the system
    for (const auto var_num : make_range(sys.n_vars()))
    {
      const auto & var = sys.variable(var_num);

      VariableHeader var_header;
      var_header.name = var.name();
      var_header.type = var.type();
      var_header.var = &var;

      mooseAssert(!sys_header.variables.count(var.name()), "Already inserted");
      sys_header.variables.emplace(var.name(), var_header);
    }

    // Vectors in the system
    sys_header.vectors.resize(sys.n_vectors());
    for (const auto vec_num : make_range(sys.n_vectors()))
    {
      auto & vec_header = sys_header.vectors[vec_num];
      vec_header.name = sys.vector_name(vec_num);
      vec_header.vec = &sys.get_vector(vec_num);
    }
    // System in this EquationSystems
    mooseAssert(!es_header.systems.count(sys.name()), "Already inserted");
    es_header.systems.emplace(sys.name(), sys_header);
  }

  return es_header;
}

std::vector<const libMesh::DofObject *>
RestartableEquationSystems::orderDofObjects() const
{
  std::vector<const libMesh::DofObject *> objects;
  auto add = [&objects](const auto begin, const auto end)
  {
    std::set<const libMesh::DofObject *, libMesh::CompareDofObjectsByID> ordered(begin, end);
    objects.insert(objects.end(), ordered.begin(), ordered.end());
  };

  const auto & mesh = _es.get_mesh();
  add(mesh.local_elements_begin(), mesh.local_elements_end());
  add(mesh.local_nodes_begin(), mesh.local_nodes_end());

  return objects;
}

void
RestartableEquationSystems::store(std::ostream & stream) const
{
  // Store the header (systems, variables, vectors)
  EquationSystemsHeader es_header = buildHeader();
  dataStore(stream, es_header, nullptr);

  // Order objects (elements and then nodes) by ID for storing
  const auto ordered_objects = orderDofObjects();

  // Store the ordered objects so we can do a sanity check on if we're
  // loading the same thing
  {
    std::vector<dof_id_type> ordered_objects_ids(ordered_objects.size());
    for (const auto i : index_range(ordered_objects))
      ordered_objects_ids[i] = ordered_objects[i]->id();
    dataStore(stream, ordered_objects_ids, nullptr);
  }

  // Lambda for storing vector values for a given system and variable
  auto store_vec = [this, &ordered_objects, &stream](
                       const NumericVector<Number> & vec, const System & sys, const Variable & var)
  {
    // SCALAR vars on the last proc only
    if (var.type().family == SCALAR && _es.processor_id() != _es.n_processors() - 1)
      return;

    // We need a separate stream here because we want to first store the size
    // so that we can skip these entries on load easily if needed
    std::ostringstream vec_stream;

    // Non-SCALAR variable
    if (var.type().family != SCALAR)
    {
      // Store for each component of each element and node
      for (const auto & obj : ordered_objects)
      {
        mooseAssert(obj, "Invalid object");
        for (const auto comp : make_range(obj->n_comp(sys.number(), var.number())))
        {
          auto val = vec(obj->dof_number(sys.number(), var.number(), comp));
          dataStore(vec_stream, val, nullptr);
        }
      }
    }
    // SCALAR variable
    else
    {
      const auto & dof_map = sys.get_dof_map();
      std::vector<dof_id_type> scalar_dofs;
      dof_map.SCALAR_dof_indices(scalar_dofs, var.number());
      for (const auto dof : scalar_dofs)
      {
        auto val = vec(dof);
        dataStore(vec_stream, val, nullptr);
      }
    }

    // First store the size so that we can skip this if needed upon load, and then the data
    std::size_t vec_stream_size = static_cast<std::size_t>(vec_stream.tellp());
    dataStore(stream, vec_stream_size, nullptr);
    stream << vec_stream.str();
  };

  // Store each system
  for (const auto & sys_name_header_pair : es_header.systems)
  {
    const auto & sys_header = sys_name_header_pair.second;
    const auto & sys = *sys_header.sys;

    // Store each variable in the system
    for (const auto & var_name_header_pair : sys_header.variables)
    {
      const auto & var = *var_name_header_pair.second.var;

      // Store the solution vector and then other vectors in the system
      store_vec(*sys.solution, sys, var);
      for (const auto & vec_header : sys_header.vectors)
        store_vec(*vec_header.vec, sys, var);
    }
  }
}

void
RestartableEquationSystems::load(std::istream & stream)
{
  // Load the header (systems, variables, vectors)
  // We do this first so that the loader can make informed decisions
  // on what to put where based on everything that is available
  EquationSystemsHeader es_header;
  dataLoad(stream, es_header, nullptr);

  // Order objects (elements and then node) by ID for storing
  const auto ordered_objects = orderDofObjects();

  // Sanity check on if we're loading the same thing
  {
    std::vector<dof_id_type> from_ordered_objects_ids;
    dataLoad(stream, from_ordered_objects_ids, nullptr);
    if (ordered_objects.size() != from_ordered_objects_ids.size())
      mooseError("Previously stored elements/nodes do not match the current element/nodes");
    for (const auto i : index_range(ordered_objects))
      if (ordered_objects[i]->id() != from_ordered_objects_ids[i])
        mooseError("Previously stored elements/nodes do not match the current element/nodes");
  }

  // Vectors that we need to close after setting; helps us avoid calling
  // close after each variable in the same vector, which ain't cheap
  std::set<NumericVector<Number> *> vecs_to_close;

  // Lambda for loading vector values for a given system and variable; will skip
  // through the data if anything is null
  auto load_vec =
      [this, &ordered_objects, &stream, &vecs_to_close](
          NumericVector<Number> * const vec, System * const sys, const Variable * const var)
  {
    // SCALAR vars on the last proc only
    if (var && var->type().family == SCALAR && _es.processor_id() != _es.n_processors() - 1)
      return;

    // This will skip through the entries for this (sys,var,vector) if
    // we do not want to load it
    std::size_t size;
    dataLoad(stream, size, nullptr);
    if (!vec || !sys || !var)
    {
      stream.seekg(size, std::ios_base::cur);
      return;
    }

    // Non-SCALAR variable
    if (var->type().family != SCALAR)
    {
      // Load for each component of each element and node
      for (const auto & obj : ordered_objects)
      {
        mooseAssert(obj, "Invalid object");
        for (const auto comp : make_range(obj->n_comp(sys->number(), var->number())))
        {
          Real val;
          dataLoad(stream, val, nullptr);
          vec->set(obj->dof_number(sys->number(), var->number(), comp), val);
        }
      }
    }
    // SCALAR variable
    else
    {
      const auto & dof_map = sys->get_dof_map();
      std::vector<dof_id_type> scalar_dofs;
      dof_map.SCALAR_dof_indices(scalar_dofs, var->number());
      for (const auto dof : scalar_dofs)
      {
        Real val;
        dataLoad(stream, val, nullptr);
        vec->set(dof, val);
      }
    }

    // Keep track of the vectors that we want to close
    vecs_to_close.insert(vec);
  };

  // Load each system
  for (const auto & [sys_name, sys_header] : es_header.systems)
  {
    mooseAssert(sys_name == sys_header.name, "Inconsistent name");
    auto * const sys = _es.has_system(sys_name) ? &_es.get_system(sys_header.name) : nullptr;

    // Load each variable
    for (const auto & [var_name, var_header] : sys_header.variables)
    {
      mooseAssert(var_name == var_header.name, "Inconsistent name");
      auto * const var = (sys && sys->has_variable(var_name))
                             ? &sys->variable(sys->variable_number(var_name))
                             : nullptr;

      if (var && var->type() != var_header.type)
        mooseError("Cannot map variable '", var_name, " in restart due to a type mismatch");

      // Load the solution vector if we have this system and variable
      load_vec(sys ? sys->solution.get() : nullptr, sys, var);
      // Load every other vector if... we're not skipping, and we have this system and variable
      for (const auto & vec_header : sys_header.vectors)
      {
        auto * const vec =
            (sys && var && !_skip_additional_vectors && sys->have_vector(vec_header.name))
                ? &sys->get_vector(vec_header.name)
                : nullptr;
        load_vec(vec, sys, var);
      }

      // Close the vectors that we've written to
      for (auto & vec : vecs_to_close)
        vec->close();
      vecs_to_close.clear();
    }
  }
}

void
dataStore(std::ostream & stream, RestartableEquationSystems & res, void *)
{
  res.store(stream);
}

void
dataLoad(std::istream & stream, RestartableEquationSystems & res, void *)
{
  res.load(stream);
}

void
dataStore(std::ostream & stream, RestartableEquationSystems::EquationSystemsHeader & header, void *)
{
  dataStore(stream, header.systems, nullptr);
}

void
dataLoad(std::istream & stream, RestartableEquationSystems::EquationSystemsHeader & header, void *)
{
  dataLoad(stream, header.systems, nullptr);
}

void
dataStore(std::ostream & stream, RestartableEquationSystems::SystemHeader & header, void *)
{
  dataStore(stream, header.name, nullptr);
  dataStore(stream, header.type, nullptr);
  dataStore(stream, header.variables, nullptr);
  dataStore(stream, header.vectors, nullptr);
}

void
dataLoad(std::istream & stream, RestartableEquationSystems::SystemHeader & header, void *)
{
  dataLoad(stream, header.name, nullptr);
  dataLoad(stream, header.type, nullptr);
  dataLoad(stream, header.variables, nullptr);
  dataLoad(stream, header.vectors, nullptr);
  header.sys = nullptr;
}

void
dataStore(std::ostream & stream, RestartableEquationSystems::VariableHeader & header, void *)
{
  dataStore(stream, header.name, nullptr);
  dataStore(stream, header.type, nullptr);
}
void
dataLoad(std::istream & stream, RestartableEquationSystems::VariableHeader & header, void *)
{
  dataLoad(stream, header.name, nullptr);
  dataLoad(stream, header.type, nullptr);
  header.var = nullptr;
}

void
dataStore(std::ostream & stream, RestartableEquationSystems::VectorHeader & header, void *)
{
  dataStore(stream, header.name, nullptr);
}
void
dataLoad(std::istream & stream, RestartableEquationSystems::VectorHeader & header, void *)
{
  dataLoad(stream, header.name, nullptr);
  header.vec = nullptr;
}
