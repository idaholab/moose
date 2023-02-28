//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshGeneratorSystem.h"

#include "MooseApp.h"
#include "MeshGenerator.h"
#include "DependencyResolver.h"

MeshGeneratorSystem::MeshGeneratorSystem(MooseApp & app)
  : PerfGraphInterface(app.perfGraph(), "MeshGeneratorSystem"),
    ParallelObject(app),
    _app(app),
    _popped_final_mesh_generator(false)
{
}

void
MeshGeneratorSystem::addMeshGenerator(const std::string & type,
                                      const std::string & name,
                                      const InputParameters & params)
{
  mooseAssert(!_mesh_generator_params.count(name), "Already exists");
  _mesh_generator_params.emplace(
      std::piecewise_construct, std::forward_as_tuple(name), std::forward_as_tuple(type, params));

  // This is a sub mesh generator
  if (_app.constructingMeshGenerators())
    createMeshGenerator(name);
}

const MeshGenerator &
MeshGeneratorSystem::appendMeshGenerator(const std::string & type,
                                         const std::string & name,
                                         InputParameters params)
{
  if (!appendingMeshGenerators())
    mooseError("Can only call appendMeshGenerator() during the append_mesh_generator task");

  if (!params.have_parameter<MeshGeneratorName>("input"))
    mooseError("While adding ",
               type,
               " '",
               name,
               "' via appendMeshGenerator():\n\nCannot append a mesh generator that does not take "
               "input mesh generators via an 'input' parameter");

  // If no final generator is set, we need to make sure that we have one
  if (_final_generator_name.empty())
    createMeshGeneratorOrder();

  // Set the final generator as the input
  params.set<MeshGeneratorName>("input") = _final_generator_name;

  // Keep track of the new final generator
  _final_generator_name = name;

  // Need to add this to the param map so that createMeshGenerator can use it
  mooseAssert(!_mesh_generator_params.count(name), "Already exists");
  _mesh_generator_params.emplace(
      std::piecewise_construct, std::forward_as_tuple(name), std::forward_as_tuple(type, params));

  return *createMeshGenerator(name);
}

std::vector<std::pair<std::string, MeshGeneratorName>>
MeshGeneratorSystem::getMeshGeneratorParamDependencies(const InputParameters & params) const
{
  std::vector<std::pair<std::string, MeshGeneratorName>> dependencies;

  auto add_dependency = [&dependencies](const auto & param_name, const auto & dependency)
  {
    if (dependency.size())
      dependencies.emplace_back(param_name, dependency);
  };

  for (const auto & [name, param] : params)
    if (const auto dependency =
            dynamic_cast<const Parameters::Parameter<MeshGeneratorName> *>(param.get()))
      add_dependency(name, dependency->get());
    else if (const auto dependencies =
                 dynamic_cast<const Parameters::Parameter<std::vector<MeshGeneratorName>> *>(
                     param.get()))
      for (const auto & dependency : dependencies->get())
        add_dependency(name, dependency);

  return dependencies;
}

void
MeshGeneratorSystem::createAddedMeshGenerators()
{
  mooseAssert(_app.actionWarehouse().getCurrentTaskName() == "create_added_mesh_generators",
              "Should not run now");

  DependencyResolver<std::string> resolver;

  // Define the dependencies known so far
  for (const auto & [name, type_params_pair] : _mesh_generator_params)
  {
    resolver.addItem(name);
    for (const auto & param_dependency_pair :
         getMeshGeneratorParamDependencies(type_params_pair.second))
      resolver.addEdge(param_dependency_pair.second, name);
  }

  std::vector<std::vector<std::string>> ordered_generators;
  try
  {
    ordered_generators = resolver.getSortedValuesSets();
  }
  catch (CyclicDependencyException<std::string> & e)
  {
    mooseError("Cyclic dependencies detected in mesh generation: ",
               MooseUtils::join(e.getCyclicDependencies(), " <- "));
  }

  // Construct all of the mesh generators that we know exist
  for (const auto & generator_names : ordered_generators)
    for (const auto & generator_name : generator_names)
      if (_mesh_generator_params.count(generator_name))
        createMeshGenerator(generator_name);

  mooseAssert(_mesh_generator_params.empty(), "Should be empty");
}

void
MeshGeneratorSystem::createMeshGeneratorOrder()
{
  mooseAssert(_app.constructingMeshGenerators() ||
                  _app.actionWarehouse().getCurrentTaskName() == "execute_mesh_generators",
              "Incorrect call time");

  TIME_SECTION("createMeshGeneratorOrder", 1, "Ordering Mesh Generators");

  _ordered_mesh_generators.clear();

  // We dare not sort these based on address!
  DependencyResolver<MeshGenerator *, MeshGenerator::Comparator> resolver;

  // Add all of the dependencies into the resolver
  for (const auto & it : _mesh_generators)
  {
    MeshGenerator * mg = it.second.get();
    resolver.addItem(mg);
    for (const auto & dep_mg : mg->getParentMeshGenerators())
      resolver.addEdge(&getMeshGeneratorInternal(dep_mg->name()), mg);
  }

  // ...and sort them
  try
  {
    _ordered_mesh_generators = resolver.getSortedValuesSets();
  }
  // It is _quite_ hard to trigger this to test it. I've tried to no avail.
  // Now that we...
  // - check and sort up front
  // - know if dependencies exist at the time of requesing them
  // - require that sub generators depend only on other sub generators in an object's
  //   tree + input dependencies that we explicitly declare
  // I don't think it's possible. But we'll leave it here anyway and it
  // definitely will not be covered
  catch (CyclicDependencyException<MeshGenerator *, MeshGenerator::Comparator> & e)
  {
    const auto & cycle = e.getCyclicDependencies();
    std::vector<std::string> names(cycle.size());
    for (const auto i : index_range(cycle))
      names[i] = cycle[i]->name();

    mooseError("Cyclic dependencies detected in mesh generation: ",
               MooseUtils::join(names, " <- "));
  }

  if (_final_generator_name.size())
  {
    if (!hasMeshGenerator(_final_generator_name))
      mooseError("The forced final MeshGenerator '", _final_generator_name, "' does not exist");
  }
  else if (_ordered_mesh_generators.size())
  {
    auto & final_generators = _ordered_mesh_generators.back();
    _final_generator_name = final_generators.back()->name();

    // See if we have multiple independent trees of generators
    const auto ancestor_list = resolver.getAncestors(final_generators.back());
    if (ancestor_list.size() != resolver.size())
    {
      // Need to remove duplicates and possibly perform a difference so we'll import out list
      // into a set for these operations.
      std::set<MeshGenerator *> ancestors(ancestor_list.begin(), ancestor_list.end());
      // Get all of the items from the resolver so we can compare against the tree from the
      // final generator we just pulled.
      const auto & allValues = resolver.getSortedValues();
      decltype(ancestors) all(allValues.begin(), allValues.end());

      decltype(ancestors) ind_tree;
      std::set_difference(all.begin(),
                          all.end(),
                          ancestors.begin(),
                          ancestors.end(),
                          std::inserter(ind_tree, ind_tree.end()));

      std::ostringstream oss;
      oss << "Your MeshGenerator tree contains multiple possible generator outputs :\n\""
          << final_generators.back()->name()
          << " and one or more of the following from an independent set: \"";
      bool first = true;
      for (const auto & gen : ind_tree)
      {
        if (!first)
          oss << ", ";
        else
          first = false;

        oss << gen->name();
      }
      oss << "\"\n\nThis may be due to a missing dependency or may be intentional. Please "
             "select the final MeshGenerator in\nthe [Mesh] block with the \"final_generator\" "
             "parameter or add additional dependencies to remove the ambiguity.";
      mooseError(oss.str());
    }
  }
}

void
MeshGeneratorSystem::executeMeshGenerators()
{
  libmesh_parallel_only(comm());

  // we do not need to do this when there are no mesh generators
  if (_mesh_generators.empty())
    return;

  // Order the generators
  createMeshGeneratorOrder();

  const auto & final_generators = _ordered_mesh_generators.back();
  if (_final_generator_name.empty())
    _final_generator_name = final_generators.back()->name();

  // Grab the outputs from the final generator so MeshGeneratorMesh can pick them up
  _final_generated_meshes.emplace_back(&getMeshGeneratorOutput(_final_generator_name));

  // Need to grab two if we're going to be making a displaced mesh
  if (_app.actionWarehouse().displacedMesh())
    _final_generated_meshes.emplace_back(&getMeshGeneratorOutput(_final_generator_name));

  // Run the MeshGenerators in the proper order
  for (const auto & generator_set : _ordered_mesh_generators)
  {
    for (const auto & generator : generator_set)
    {
      const auto & name = generator->name();

      auto current_mesh = generator->generateInternal();

      // Now we need to possibly give this mesh to downstream generators
      auto & outputs = _mesh_generator_outputs[name];

      if (outputs.size())
      {
        auto & first_output = *outputs.begin();

        first_output = std::move(current_mesh);

        const auto & copy_from = *first_output;

        auto output_it = ++outputs.begin();

        // For all of the rest we need to make a copy
        for (; output_it != outputs.end(); ++output_it)
          (*output_it) = copy_from.clone();
      }

      // Once we hit the generator we want, we'll terminate the loops (this might be the last
      // iteration anyway)
      if (_final_generator_name == name)
        return;
    }
  }
}

std::shared_ptr<MeshGenerator>
MeshGeneratorSystem::createMeshGenerator(const std::string & generator_name)
{
  libmesh_parallel_only(comm());
  mooseAssert(_app.constructingMeshGenerators(), "Should not run now");

  const auto find_params = _mesh_generator_params.find(generator_name);
  mooseAssert(find_params != _mesh_generator_params.end(), "Not added");
  const auto & [type, params] = find_params->second;
  mooseAssert(comm().verify(type + generator_name), "Inconsistent construction order");

  std::shared_ptr<MeshGenerator> mg =
      _app.getFactory().create<MeshGenerator>(type, generator_name, params);

  // Setup the children and parents
  for (const auto & dependency : mg->getRequestedMeshGenerators())
  {
    // We _shouldn't_ hit this; now that we enforce construction ordering we do
    // all of this error checking at construction time because the info is available
    mooseAssert(hasMeshGenerator(dependency), "Missing dependency");

    auto & dependency_mg = getMeshGeneratorInternal(dependency);
    mg->addParentMeshGenerator(dependency_mg, MeshGenerator::AddParentChildKey());
    dependency_mg.addChildMeshGenerator(*mg, MeshGenerator::AddParentChildKey());
  }

  // Loop through all of the MeshGeneratorName and std::vector<MeshGeneratorName>
  // parameters (meshes that we should depend on), and make sure that either:
  // - We directly depend on them and requested a mesh them
  // - We created a sub generator that depends on them and we declared it
  for (const auto & param_dependency_pair : getMeshGeneratorParamDependencies(mg->parameters()))
  {
    const auto & param_name = param_dependency_pair.first;
    const auto & dependency_name = param_dependency_pair.second;

    if (mg->isNullMeshName(dependency_name))
      continue;

    // True if this dependency was requested and is a parent
    if (mg->isParentMeshGenerator(dependency_name))
    {
      mooseAssert(mg->getRequestedMeshGenerators().count(dependency_name), "Wasn't requested");
      continue;
    }

    // Whether or not this is a dependency of at least one SubGenerator
    auto find_sub_dependency = std::find_if(mg->getSubMeshGenerators().begin(),
                                            mg->getSubMeshGenerators().end(),
                                            [&dependency_name](const auto & mg)
                                            { return mg->isParentMeshGenerator(dependency_name); });
    const auto is_sub_dependency = find_sub_dependency != mg->getSubMeshGenerators().end();

    // This should be used by a sub generator
    if (mg->getRequestedMeshGeneratorsForSub().count(dependency_name))
    {
      if (!is_sub_dependency)
        mg->mooseError("The sub generator dependency declared from MeshGenerator '",
                       dependency_name,
                       "' from the parameter '",
                       param_name,
                       "' was not used.\n\nDependencies that are declared by getMeshForSub() "
                       "must be used as an input to a sub generator generated by this object.");
    }
    // This was used by a sub generator but wasn't declared
    else if (is_sub_dependency)
      mg->mooseError("The MeshGenerator '",
                     dependency_name,
                     "' was referenced in the parameter '",
                     param_name,
                     "' and used in the sub ",
                     (*find_sub_dependency)->type(),
                     " '",
                     (*find_sub_dependency)->name(),
                     "', but was not declared as a sub dependency.\n\nTo correct this, declare the "
                     "mesh in the parameter '",
                     param_name,
                     "' as a sub dependency with getMeshForSub().");
    // Isn't used at all
    else
      mg->mooseError(
          "You failed to request the generated mesh(es) for the parameter '",
          param_name,
          "'.\n\nIn specific, the mesh from MeshGenerator '",
          dependency_name,
          "' was not requested.\n\nTo correct this, you should remove the parameter if the "
          "mesh(es)\nare not needed, or request the mesh(es) with getMesh()/getMeshes().");
  }

  mooseAssert(!_mesh_generators.count(generator_name), "Already created");
  _mesh_generators.emplace(generator_name, mg);
  mooseAssert(!_mesh_generator_outputs.count(generator_name), "Already exists");
  _mesh_generator_outputs[generator_name];
  _mesh_generator_params.erase(find_params);

  return mg;
}

std::unique_ptr<MeshBase> &
MeshGeneratorSystem::getMeshGeneratorOutput(const MeshGeneratorName & name)
{
  mooseAssert(_app.constructingMeshGenerators() ||
                  _app.actionWarehouse().getCurrentTaskName() == "execute_mesh_generators",
              "Incorrect call time");

  auto it = _mesh_generator_outputs.find(name);
  mooseAssert(it != _mesh_generator_outputs.end(), "Not initialized");
  it->second.push_back(nullptr);
  return it->second.back();
}

bool
MeshGeneratorSystem::hasMeshGenerator(const MeshGeneratorName & name) const
{
  return _mesh_generators.count(name);
}

bool
MeshGeneratorSystem::hasMeshGeneratorParams(const MeshGeneratorName & name) const
{
  return _mesh_generator_params.count(name);
}

const MeshGenerator &
MeshGeneratorSystem::getMeshGenerator(const std::string & name) const
{
  const auto it = _mesh_generators.find(name);
  if (it == _mesh_generators.end())
    mooseError("Failed to find a MeshGenerator with the name '", name, "'");
  mooseAssert(it->second, "Invalid shared pointer");
  return *it->second;
}

std::vector<std::string>
MeshGeneratorSystem::getMeshGeneratorNames() const
{
  std::vector<std::string> names;
  for (auto & pair : _mesh_generators)
    names.push_back(pair.first);
  return names;
}

void
MeshGeneratorSystem::setFinalMeshGeneratorName(const std::string & generator_name,
                                               const SetFinalMeshGeneratorNameKey)
{
  mooseAssert(_final_generator_name.empty(), "Already set");
  _final_generator_name = generator_name;
}

std::unique_ptr<MeshBase>
MeshGeneratorSystem::getMeshGeneratorMesh(const bool check_unique /* = true */)
{
  if (_popped_final_mesh_generator == true)
    mooseError("MeshGeneratorSystem::getMeshGeneratorMesh is being called for a second time. You "
               "cannot do "
               "this because the final generated mesh was popped from its storage container the "
               "first time this method was called");

  if (_final_generated_meshes.empty())
    mooseError("No generated mesh to retrieve. Your input file should contain either a [Mesh] or "
               "block.");

  auto mesh_unique_ptr_ptr = _final_generated_meshes.front();
  _final_generated_meshes.pop_front();
  _popped_final_mesh_generator = true;

  if (check_unique && !_final_generated_meshes.empty())
    mooseError("Multiple generated meshes exist while retrieving the final Mesh. This means that "
               "the selection of the final mesh is non-deterministic.");

  return std::move(*mesh_unique_ptr_ptr);
}

bool
MeshGeneratorSystem::appendingMeshGenerators() const
{
  return _app.actionWarehouse().getCurrentTaskName() == "append_mesh_generator";
}
