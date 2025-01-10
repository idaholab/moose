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
#include "libmesh/elem.h"
#include "libmesh/node.h"

const std::string RestartableEquationSystems::SystemHeader::system_solution_name =
    "SYSTEM_SOLUTION";

RestartableEquationSystems::RestartableEquationSystems(libMesh::MeshBase & mesh)
  : _es(mesh), _load_all_vectors(true)
{
}

RestartableEquationSystems::EquationSystemsHeader
RestartableEquationSystems::buildHeader(
    const std::vector<const libMesh::DofObject *> & ordered_objects) const
{
  EquationSystemsHeader es_header;

  // Systems
  for (const auto sys_num : make_range(_es.n_systems()))
  {
    const auto & sys = _es.get_system(sys_num);

    SystemHeader sys_header;
    sys_header.name = sys.name();
    sys_header.type = sys.system_type();

    // Variables in the system
    for (const auto var_num : make_range(sys.n_vars()))
    {
      const auto & var = sys.variable(var_num);

      VariableHeader var_header;
      var_header.name = var.name();
      var_header.type = var.type();
      var_header.size = 0;
      var_header.variable = &var;

      mooseAssert(_es.comm().verify("sys_" + sys.name() + "_var_" + var.name()),
                  "Out of order in parallel");
      mooseAssert(!sys_header.variables.count(var.name()), "Already inserted");

      // Non-SCALAR variable
      if (var.type().family != SCALAR)
      {
        for (const auto & obj : ordered_objects)
          var_header.size += sizeof(Real) * obj->n_comp(sys.number(), var.number());
      }
      // SCALAR variable on the last rank
      else if (_es.processor_id() == _es.n_processors() - 1)
      {
        std::vector<dof_id_type> scalar_dofs;
        sys.get_dof_map().SCALAR_dof_indices(scalar_dofs, var.number());
        var_header.size += sizeof(Real) * scalar_dofs.size();
      }

      sys_header.variables.emplace(var.name(), var_header);
    }

    // System vector
    auto & sys_vec_header = sys_header.vectors[SystemHeader::system_solution_name];
    sys_vec_header.name = SystemHeader::system_solution_name;
    sys_vec_header.vector = sys.solution.get();
    sys_vec_header.type = sys_vec_header.vector->type();
    for (const auto vec_num : make_range(sys.n_vectors()))
    {
      mooseAssert(_es.comm().verify("sys_" + sys.name() + "_vec_" + sys.vector_name(vec_num)),
                  "Out of order in parallel");
      const auto & name = sys.vector_name(vec_num);
      mooseAssert(!sys_header.vectors.count(name), "Already inserted");
      auto & vec_header = sys_header.vectors[name];
      vec_header.name = sys.vector_name(vec_num);
      vec_header.projections = sys.vector_preservation(vec_header.name);
      vec_header.vector = &sys.get_vector(vec_header.name);
      vec_header.type = vec_header.vector->type();
    }

    // System in this EquationSystems
    mooseAssert(!es_header.systems.count(sys.name()), "Already inserted");
    es_header.systems.emplace(sys.name(), sys_header);
  }

  // Setup the positions in each vector for easy access later
  std::size_t offset = 0;
  for (auto & sys_name_header_pair : es_header.systems)
  {
    auto & sys_header = sys_name_header_pair.second;
    for (auto & vec_name_header_pair : sys_header.vectors)
    {
      auto & vec_header = vec_name_header_pair.second;
      for (const auto & var_name_header_pair : sys_header.variables)
      {
        const auto & var_header = var_name_header_pair.second;
        mooseAssert(!vec_header.variable_offset.count(var_header.name), "Already inserted");
        vec_header.variable_offset[var_header.name] = offset;
        offset += var_header.size;
      }
    }
  }

  es_header.data_size = offset;

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
  // Order objects (elements and then nodes) by ID for storing
  const auto ordered_objects = orderDofObjects();

  // Store the header (systems, variables, vectors)
  EquationSystemsHeader es_header = buildHeader(ordered_objects);
  dataStore(stream, es_header, nullptr);

  // Store the ordered objects so we can do a sanity check on if we're
  // loading the same thing
  {
    std::vector<dof_id_type> ordered_objects_ids(ordered_objects.size());
    for (const auto i : index_range(ordered_objects))
      ordered_objects_ids[i] = ordered_objects[i]->id();
    dataStore(stream, ordered_objects_ids, nullptr);
  }

#ifndef NDEBUG
  const std::size_t data_initial_position = static_cast<std::size_t>(stream.tellp());
#endif

  // Store each system
  for (const auto & sys_name_header_pair : es_header.systems)
  {
    const auto & sys_header = sys_name_header_pair.second;
    const auto & sys = _es.get_system(sys_header.name);

    // Store each vector and variable
    for (const auto & vec_name_header_pair : sys_header.vectors)
    {
      const auto & vec_header = vec_name_header_pair.second;
      const auto & vec = *vec_header.vector;
      for (const auto & var_name_header_pair : sys_header.variables)
      {
        const auto & var_header = var_name_header_pair.second;
        const auto & var = *var_header.variable;

#ifndef NDEBUG
        const std::size_t var_initial_position = stream.tellp();
#endif

        // Non-SCALAR variable
        if (var.type().family != SCALAR)
        {
          // Store for each component of each element and node
          for (const auto & obj : ordered_objects)
            for (const auto comp : make_range(obj->n_comp(sys.number(), var.number())))
            {
              auto val = vec(obj->dof_number(sys.number(), var.number(), comp));
              dataStore(stream, val, nullptr);
            }
        }
        // SCALAR variable on the last rank
        else if (_es.processor_id() == _es.n_processors() - 1)
        {
          const auto & dof_map = sys.get_dof_map();
          std::vector<dof_id_type> scalar_dofs;
          dof_map.SCALAR_dof_indices(scalar_dofs, var.number());
          for (const auto dof : scalar_dofs)
          {
            auto val = vec(dof);
            dataStore(stream, val, nullptr);
          }
        }

#ifndef NDEBUG
        const std::size_t data_offset = var_initial_position - data_initial_position;
        mooseAssert(vec_header.variable_offset.at(var_header.name) == data_offset,
                    "Invalid offset");

        const std::size_t current_position = static_cast<std::size_t>(stream.tellp());
        const std::size_t var_size = current_position - var_initial_position;
        mooseAssert(var_size == sys_header.variables.at(var.name()).size, "Incorrect assumed size");
#endif
      }
    }
  }

  mooseAssert((data_initial_position + es_header.data_size) ==
                  static_cast<std::size_t>(stream.tellp()),
              "Incorrect assumed size");
}

void
RestartableEquationSystems::load(std::istream & stream)
{
  // Load the header (systems, variables, vectors)
  // We do this first so that the loader can make informed decisions
  // on what to put where based on everything that is available
  _loaded_header.systems.clear();
  dataLoad(stream, _loaded_header, nullptr);

  // Order objects (elements and then node) by ID for storing
  _loaded_ordered_objects = orderDofObjects();

  // Sanity check on if we're loading the same thing
  {
    std::vector<dof_id_type> from_ordered_objects_ids;
    dataLoad(stream, from_ordered_objects_ids, nullptr);
    if (_loaded_ordered_objects.size() != from_ordered_objects_ids.size())
      mooseError("RestartableEquationSystems::load(): Number of previously stored elements/nodes (",
                 _loaded_ordered_objects.size(),
                 ") does not "
                 "match the current number of elements/nodes (",
                 from_ordered_objects_ids.size(),
                 ")");
    for (const auto i : index_range(_loaded_ordered_objects))
      if (_loaded_ordered_objects[i]->id() != from_ordered_objects_ids[i])
        mooseError("RestartableEquationSystems::load(): Id of previously stored element/node (",
                   _loaded_ordered_objects[i]->id(),
                   ") does not "
                   "match the current element/node id (",
                   from_ordered_objects_ids[i],
                   ")");
  }

  _loaded_stream_data_begin = static_cast<std::size_t>(stream.tellg());

  // Load everything that we have available at the moment
  for (const auto & sys_name_header_pair : _loaded_header.systems)
  {
    const auto & sys_header = sys_name_header_pair.second;
    if (!_es.has_system(sys_header.name))
      continue;
    auto & sys = _es.get_system(sys_header.name);

    bool modified_sys = false;

    for (const auto & vec_name_header_pair : sys_header.vectors)
    {
      bool modified_vec = false;

      const auto & vec_header = vec_name_header_pair.second;
      const bool is_solution = vec_header.name == SystemHeader::system_solution_name;

      if (!is_solution && !sys.have_vector(vec_header.name))
      {
        if (_load_all_vectors)
          sys.add_vector(vec_header.name, vec_header.projections, vec_header.type);
        else
          continue;
      }

      auto & vec = is_solution ? *sys.solution : sys.get_vector(vec_header.name);

      for (const auto & var_name_header_pair : sys_header.variables)
      {
        const auto & var_header = var_name_header_pair.second;
        if (!sys.has_variable(var_header.name))
          continue;
        const auto & var = sys.variable(sys.variable_number(var_header.name));
        if (var.type() != var_header.type)
          continue;

        restore(sys_header, vec_header, var_header, sys, vec, var, stream);
        modified_vec = true;
      }

      if (modified_vec)
      {
        vec.close();
        modified_sys = true;
      }
    }

    if (modified_sys)
      sys.update();
  }

  // Move the stream to the end of our data so that we make RestartableDataReader happy
  stream.seekg(_loaded_stream_data_begin + _loaded_header.data_size);
}

void
RestartableEquationSystems::restore(const SystemHeader & from_sys_header,
                                    const VectorHeader & from_vec_header,
                                    const VariableHeader & from_var_header,
                                    const libMesh::System & to_sys,
                                    libMesh::NumericVector<libMesh::Number> & to_vec,
                                    const libMesh::Variable & to_var,
                                    std::istream & stream)
{
#ifndef NDEBUG
  const auto sys_it = _loaded_header.systems.find(from_sys_header.name);
  mooseAssert(sys_it != _loaded_header.systems.end(), "System does not exist");
  const auto & sys_header = sys_it->second;
  mooseAssert(sys_header == from_sys_header, "Not my system");
  const auto vec_it = sys_header.vectors.find(from_vec_header.name);
  mooseAssert(vec_it != sys_header.vectors.end(), "Vector does not exist");
  const auto & vec_header = vec_it->second;
  mooseAssert(vec_header == from_vec_header, "Not my vector");
  const auto var_it = sys_header.variables.find(from_var_header.name);
  mooseAssert(var_it != sys_header.variables.end(), "Variable does not exist");
  const auto & var_header = var_it->second;
  mooseAssert(var_header == from_var_header, "Not my variable");
#endif

  const auto error =
      [&from_sys_header, &from_vec_header, &from_var_header, &to_sys, &to_var](auto... args)
  {
    mooseError("An error occured while restoring a system:\n",
               args...,
               "\n\nFrom system: ",
               from_sys_header.name,
               "\nFrom vector: ",
               from_vec_header.name,
               "\nFrom variable: ",
               from_var_header.name,
               "\nTo system: ",
               to_sys.name(),
               "\nTo variable: ",
               to_var.name());
  };

  if (from_var_header.type != to_var.type())
    error("Cannot restore to a variable of a different type");

  const auto offset = from_vec_header.variable_offset.at(from_var_header.name);
  stream.seekg(_loaded_stream_data_begin + offset);

  // Non-SCALAR variable
  if (to_var.type().family != SCALAR)
  {
    for (const auto & obj : _loaded_ordered_objects)
      for (const auto comp : make_range(obj->n_comp(to_sys.number(), to_var.number())))
      {
        Real val;
        dataLoad(stream, val, nullptr);
        to_vec.set(obj->dof_number(to_sys.number(), to_var.number(), comp), val);
      }
  }
  // SCALAR variable on the last rank
  else if (_es.processor_id() == _es.n_processors() - 1)
  {
    const auto & dof_map = to_sys.get_dof_map();
    std::vector<dof_id_type> scalar_dofs;
    dof_map.SCALAR_dof_indices(scalar_dofs, to_var.number());
    for (const auto dof : scalar_dofs)
    {
      Real val;
      dataLoad(stream, val, nullptr);
      to_vec.set(dof, val);
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
  dataStore(stream, header.data_size, nullptr);
}

void
dataLoad(std::istream & stream, RestartableEquationSystems::EquationSystemsHeader & header, void *)
{
  dataLoad(stream, header.systems, nullptr);
  dataLoad(stream, header.data_size, nullptr);
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
}

void
dataStore(std::ostream & stream, RestartableEquationSystems::VectorHeader & header, void *)
{
  dataStore(stream, header.name, nullptr);
  dataStore(stream, header.projections, nullptr);
  dataStore(stream, header.type, nullptr);
  dataStore(stream, header.variable_offset, nullptr);
}
void
dataLoad(std::istream & stream, RestartableEquationSystems::VectorHeader & header, void *)
{
  dataLoad(stream, header.name, nullptr);
  dataLoad(stream, header.projections, nullptr);
  dataLoad(stream, header.type, nullptr);
  dataLoad(stream, header.variable_offset, nullptr);
}
