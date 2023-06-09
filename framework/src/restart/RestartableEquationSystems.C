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

      mooseAssert(_es.comm().verify("sys_" + sys.name() + "_var_" + var.name()),
                  "Out of order in parallel");
      mooseAssert(!sys_header.variables.count(var.name()), "Already inserted");

      sys_header.variables.emplace(var.name(), var_header);
    }

    // Vectors in the system
    sys_header.vectors.resize(sys.n_vectors());
    for (const auto vec_num : make_range(sys.n_vectors()))
    {
      mooseAssert(_es.comm().verify("sys_" + sys.name() + "_vec_" + sys.vector_name(vec_num)),
                  "Out of order in parallel");

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

  // Store each system
  for (const auto & sys_name_header_pair : es_header.systems)
  {
    const auto & sys_header = sys_name_header_pair.second;
    const auto & sys = *sys_header.sys;

    // The vectors we need to store (solution and then additional vectors)
    std::vector<const libMesh::NumericVector<libMesh::Number> *> vectors(
        sys_header.vectors.size() + 1, nullptr);
    vectors[0] = sys_header.sys->solution.get();
    for (const auto i : index_range(sys_header.vectors))
      vectors[i + 1] = sys_header.vectors[i].vec;

    // Store each vector (see above)
    for (auto & vec : vectors)
    {
      // Store each variable in the system for this vector
      for (const auto & var_name_header_pair : sys_header.variables)
      {
        const auto & var = *var_name_header_pair.second.var;

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
              auto val = (*vec)(obj->dof_number(sys.number(), var.number(), comp));
              dataStore(vec_stream, val, nullptr);
            }
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
            auto val = (*vec)(dof);
            dataStore(vec_stream, val, nullptr);
          }
        }

        // First store the size so that we can skip this if needed upon load, and then the data
        std::size_t vec_stream_size = static_cast<std::size_t>(vec_stream.tellp());
        dataStore(stream, vec_stream_size, nullptr);
        stream << vec_stream.str();
      }
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
  std::set<std::string> vecs_to_close;

  // Fill the mapping in the header, which right now just maps
  // systems/vectors/variables by name
  // Loop over systems
  for (auto & sys_name_header_pair : es_header.systems)
  {
    auto & sys_header = sys_name_header_pair.second;
    if (_es.has_system(sys_header.name))
      sys_header.to_sys = &_es.get_system(sys_header.name);

    auto & sys = sys_header.to_sys;

    // Loop over variables in the system
    for (auto & var_name_header_pair : sys_header.variables)
    {
      auto & var_header = var_name_header_pair.second;
      mooseAssert(_es.comm().verify("sys_" + sys_header.name + "_var_" + var_header.name),
                  "Out of order in parallel");

      if (sys && sys->has_variable(var_header.name))
        var_header.to_var = &sys->variable(sys->variable_number(var_header.name));
      auto & var = var_header.to_var;

      if (var && var->type() != var_header.type)
        mooseError("Cannot map variable '", var_header.name, " in restart due to a type mismatch");
    }

    // Loop over vectors in the system
    for (auto & vec_header : sys_header.vectors)
      if (sys && sys->have_vector(vec_header.name))
        vec_header.to_vec = &sys->get_vector(vec_header.name);
  }

  // Actually load each system based on the mapping from above
  for (auto & sys_name_header_pair : es_header.systems)
  {
    auto & sys_header = sys_name_header_pair.second;
    auto & sys = sys_header.to_sys;

    // The vectors we need to load (solution and then additional vectors)
    std::vector<libMesh::NumericVector<libMesh::Number> *> vectors(sys_header.vectors.size() + 1,
                                                                   nullptr);
    if (sys_header.to_sys)
      vectors[0] = sys_header.to_sys->solution.get();
    if (!_skip_additional_vectors)
      for (const auto i : index_range(sys_header.vectors))
        vectors[i + 1] = sys_header.vectors[i].to_vec;

    // Load each vector
    for (auto & vec : vectors)
    {
      // Whether or not we should close this vector (true if we wrote to it)
      bool should_close_vec = false;

      // Load each variable that contributes to this vector
      for (auto & var_name_header_pair : sys_header.variables)
      {
        auto & var_header = var_name_header_pair.second;
        auto & var = var_header.to_var;

        // We store the number of entries first so we can skip if needed
        std::size_t size;
        dataLoad(stream, size, nullptr);

        // Skip if we don't have somewhere to fill into
        mooseAssert(_es.comm().verify(bool(vec)), "Inconsistent vector state");
        mooseAssert(_es.comm().verify(bool(sys)), "Inconsistent system state");
        mooseAssert(_es.comm().verify(bool(var)), "Inconsistent variable state");
        if (!vec || !sys || !var)
        {
          stream.seekg(size, std::ios_base::cur);
          continue;
        }

        // We're gonna write to this vector, so we need to close it
        should_close_vec = true;

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
        // SCALAR variable on the last rank
        else if (_es.processor_id() == _es.n_processors() - 1)
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
      }

      if (should_close_vec)
        vec->close();
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
