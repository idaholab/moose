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
    sys_header.number = sys_num;

    // Variables in the system
    for (const auto var_num : make_range(sys.n_vars()))
    {
      const auto & var = sys.variable(var_num);

      VariableHeader var_header;
      var_header.name = var.name();
      var_header.number = var_num;
      var_header.type = var.type();

      mooseAssert(!sys_header.variables.count(var.name()), "Already inserted");
      sys_header.variables.emplace(var.name(), var_header);
    }

    // Vectors in the system
    for (const auto vec_num : make_range(sys.n_vectors()))
      sys_header.vectors.push_back(sys.vector_name(vec_num));

    // System in this EquationSystems
    mooseAssert(!es_header.systems.count(sys.name()), "Already inserted");
    es_header.systems.emplace(sys.name(), sys_header);
  }

  return es_header;
}

std::vector<const DofObject *>
RestartableEquationSystems::orderDofObjects() const
{
  std::vector<const DofObject *> objects;
  auto add = [&objects](const auto begin, const auto end)
  {
    std::set<const DofObject *, CompareDofObjectsByID> ordered(begin, end);
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

  // Store each system
  for (const auto & [sys_name, sys_header] : es_header.systems)
  {
    const auto & sys = _es.get_system(sys_header.number);

    // Store each variable in the system
    for (const auto & [var_name, var_header] : sys_header.variables)
    {
      // Save scalars for last
      if (var_header.type.family == SCALAR)
        continue;

      // Lambda for storing vector values for this system and variable
      auto store_vec = [&ordered_objects, &stream, &sys_header, &var_header](const auto & vec)
      {
        // We need a separate stream here because we want to first store the size
        // so that we can skip these entries on load easily if needed
        std::ostringstream vec_stream;

        // Store for each component of each element and node
        for (const auto & obj : ordered_objects)
          for (const auto comp : make_range(obj->n_comp(sys_header.number, var_header.number)))
          {
            auto val = vec(obj->dof_number(sys_header.number, var_header.number, comp));
            dataStore(vec_stream, val, nullptr);
          }

        // First store the size so that we can skip this if needed, and then the data
        std::size_t vec_stream_size = static_cast<std::size_t>(vec_stream.tellp());
        dataStore(stream, vec_stream_size, nullptr);
        stream << vec_stream.str();
      };

      // Store the solution vector and then other vectors in the system
      store_vec(*sys.solution);
      for (const auto & vec_name : sys_header.vectors)
        store_vec(sys.get_vector(vec_name));
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

  // Load each system
  for (const auto & [sys_name, sys_header] : es_header.systems)
  {
    const bool has_sys = _es.has_system(sys_name);
    auto sys = has_sys ? &_es.get_system(sys_header.number) : nullptr;

    // Vectors that we need to close after setting; helps us avoid calling
    // close after each variable in the same vector, which ain't cheap
    std::set<NumericVector<Number> *> vectors_to_close;

    // Load each variable
    for (const auto & [var_name, var_header] : sys_header.variables)
    {
      // Save scalars for last
      if (var_header.type.family == SCALAR)
        continue;

      bool skip = !has_sys || !sys->has_variable(var_name);

      // Lambda for storing vector values for this system and variable
      auto load_vec =
          [&ordered_objects, &stream, &sys_header, &var_header, &vectors_to_close](auto vec)
      {
        // This will skip through the entries for this (sys,var,vector) if
        // we do not want to load it
        std::size_t size;
        dataLoad(stream, size, nullptr);
        if (!vec)
        {
          stream.seekg(size, std::ios_base::cur);
          return;
        }

        // We do wanna load it! Load for each component of each element and node
        for (const auto & obj : ordered_objects)
          for (const auto comp : make_range(obj->n_comp(sys_header.number, var_header.number)))
          {
            Real val;
            dataLoad(stream, val, nullptr);
            vec->set(obj->dof_number(sys_header.number, var_header.number, comp), val);
          }

        // Keep track of the vectors that we want to close
        vectors_to_close.insert(vec);
      };

      // Load the solution vector if we have this system, this variable, and this vector;
      // otherwise, skip through the entries
      load_vec(skip ? nullptr : sys->solution.get());

      // At this point, we're working on additional vectors, so skip it if we done
      if (_skip_additional_vectors)
        skip = true;
      // Load every other vector if... the same as above but with said vector
      for (const auto & vec_name : sys_header.vectors)
        load_vec((!skip && sys->have_vector(vec_name)) ? &sys->get_vector(vec_name) : nullptr);
    }

    // Close the vectors that we've written to
    for (auto & vec : vectors_to_close)
      vec->close();
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
  dataStore(stream, header.number, nullptr);
  dataStore(stream, header.variables, nullptr);
  dataStore(stream, header.vectors, nullptr);
}

void
dataLoad(std::istream & stream, RestartableEquationSystems::SystemHeader & header, void *)
{
  dataLoad(stream, header.name, nullptr);
  dataLoad(stream, header.type, nullptr);
  dataLoad(stream, header.number, nullptr);
  dataLoad(stream, header.variables, nullptr);
  dataLoad(stream, header.vectors, nullptr);
}

void
dataStore(std::ostream & stream, RestartableEquationSystems::VariableHeader & header, void *)
{
  dataStore(stream, header.name, nullptr);
  dataStore(stream, header.number, nullptr);
  dataStore(stream, header.type, nullptr);
}
void
dataLoad(std::istream & stream, RestartableEquationSystems::VariableHeader & header, void *)
{
  dataLoad(stream, header.name, nullptr);
  dataLoad(stream, header.number, nullptr);
  dataLoad(stream, header.type, nullptr);
}
