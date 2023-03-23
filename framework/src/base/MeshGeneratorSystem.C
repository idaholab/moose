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
  : PerfGraphInterface(app.perfGraph(), "MeshGeneratorSystem"), ParallelObject(app), _app(app)
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

  // This should be a sub mesh generator. We can assume this because if we're in the middle of
  // constructing mesh generators (not "adding" them, where we simply store their parameters)
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

  // Make sure this mesh generator has one and _only_ one input, as "input"
  const auto param_name_mg_name_pairs = getMeshGeneratorParamDependencies(params, true);
  if (param_name_mg_name_pairs.size() != 1 || param_name_mg_name_pairs[0].first != "input")
    mooseError("While adding ",
               type,
               " '",
               name,
               "' via appendMeshGenerator():\nCan only append a mesh generator that takes a "
               "single input mesh generator via the parameter named 'input'");

  // If no final generator is set, we need to make sure that we have one; we will hit
  // this the first time we add an appended MeshGenerator and only need to do it once.
  // We'll then generate the final ordering within the execution phase. We'll also
  // clear the ordering because it could be invalid if we append any more generators,
  // and we'll be re-ordering within executeMeshgenerators() anyway (where we don't
  // keep track of any state for the sake of simpler logic)
  if (_final_generator_name.empty())
  {
    if (_mesh_generators.empty())
      mooseError("Cannot use appendMeshGenerator() because there is not a generator to append to!");

    createMeshGeneratorOrder();
    _ordered_mesh_generators.clear();
  }

  // Set the final generator as the input
  mooseAssert(hasMeshGenerator(_final_generator_name), "Missing final generator");
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
MeshGeneratorSystem::getMeshGeneratorParamDependencies(const InputParameters & params,
                                                       const bool allow_empty /* = false */) const
{
  std::vector<std::pair<std::string, MeshGeneratorName>> dependencies;

  auto add_dependency =
      [&dependencies, &allow_empty](const auto & param_name, const auto & dependency)
  {
    if (dependency.size() || allow_empty)
      dependencies.emplace_back(param_name, dependency);
  };

  for (const auto & [name, param] : params)
    if (const auto dependency =
            dynamic_cast<const Parameters::Parameter<MeshGeneratorName> *>(param.get()))
      add_dependency(name, dependency->get());
    else if (const auto dependencies =
                 dynamic_cast<const Parameters::Parameter<std::vector<MeshGeneratorName>> *>(
                     param.get()))
    {
      if (allow_empty && dependencies->get().empty())
        add_dependency(name, std::string());
      for (const auto & dependency : dependencies->get())
        add_dependency(name, dependency);
    }

  return dependencies;
}

void
MeshGeneratorSystem::createAddedMeshGenerators()
{
  mooseAssert(_app.actionWarehouse().getCurrentTaskName() == "create_added_mesh_generators",
              "Should not run now");

  // No generators were added via addMeshGenerator()
  if (_mesh_generator_params.empty())
    return;

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
  mooseAssert(_final_generator_name.empty(), "Should be unset at this point");

  // Set the final generator if we have one set by the user
  // and if so make sure it also exists
  const auto & moose_mesh = _app.actionWarehouse().getMesh();
  if (moose_mesh->parameters().have_parameter<std::string>("final_generator") &&
      moose_mesh->isParamValid("final_generator"))
  {
    mooseAssert(moose_mesh->type() == "MeshGeneratorMesh",
                "Assumption for mesh type is now invalid");

    _final_generator_name = moose_mesh->getParam<std::string>("final_generator");
    if (!hasMeshGenerator(_final_generator_name))
      moose_mesh->paramError("final_generator",
                             "The forced final MeshGenerator '",
                             _final_generator_name,
                             "' does not exist");
  }
}

void
MeshGeneratorSystem::createMeshGeneratorOrder()
{
  mooseAssert(_app.constructingMeshGenerators() ||
                  _app.actionWarehouse().getCurrentTaskName() == "execute_mesh_generators",
              "Incorrect call time");
  mooseAssert(_ordered_mesh_generators.empty(), "Already ordered");
  mooseAssert(_mesh_generators.size(), "No mesh generators to order");

  TIME_SECTION("createMeshGeneratorOrder", 1, "Ordering Mesh Generators");

  // We dare not sort these based on address!
  DependencyResolver<MeshGenerator *, MeshGenerator::Comparator> resolver;

  std::vector<std::string> save_in_generators;

  // Hold all the mesh generators marked to be saved
  for (const auto & it : _mesh_generators)
    if (it.second->hasSaveMesh())
      save_in_generators.push_back(it.second->name());

  // The mesh generator tree should have all the mesh generators that
  // The final generator depends on and all the mesh generators
  // with 'save in' flag. Here we loop over all the mesh generators
  // and conditionally add all of the dependencies into the resolver
  for (const auto & it : _mesh_generators)
  {
    MeshGenerator * mg = it.second.get();

    // The mesh generator has to meet one of the following conditions:
    // 1. the final mesh generator is not set at this point
    // 2. this mesh generator is the final one
    // 3. this mesh generator need to be saved
    // 4. the final mesh generator is set and is a child
    // 5. this mesh generator need to be saved and is a child
    if (_final_generator_name.empty() || mg->name() == _final_generator_name || mg->hasSaveMesh() ||
        (_final_generator_name.size() && mg->isChildMeshGenerator(_final_generator_name, false)) ||
        std::find_if(save_in_generators.begin(),
                     save_in_generators.end(),
                     [&mg](const auto & name)
                     { return mg->isChildMeshGenerator(name, false); }) != save_in_generators.end())
    {
      resolver.addItem(mg);
      for (const auto & dep_mg : mg->getParentMeshGenerators())
        resolver.addEdge(&getMeshGeneratorInternal(dep_mg->name()), mg);
    }
  }

  // ...and sort them
  try
  {
    _ordered_mesh_generators = resolver.getSortedValuesSets();
  }
  // It is _quite_ hard to trigger this to test it. I've tried to no avail.
  // Now that we...
  // - check and sort up front
  // - know if dependencies exist at the time of requesting them
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

  mooseAssert(_ordered_mesh_generators.size(), "No mesh generators found");

  const auto & final_generators = _ordered_mesh_generators.back();

  // We haven't forced a final generator yet
  if (_final_generator_name.empty())
  {
    mooseAssert(final_generators.size(), "Empty vector");

    // See if we have multiple independent trees of generators
    const auto ancestor_list = resolver.getAncestors(final_generators.back());
    if (ancestor_list.size() != resolver.size() && _final_generator_name.empty())
    {
      // Need to remove duplicates and possibly perform a difference so we'll import our list
      // into a set for these operations.
      std::set<MeshGenerator *, MeshGenerator::Comparator> ancestors(ancestor_list.begin(),
                                                                     ancestor_list.end());
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
      oss << "\"\n\nThis may be due to a missing dependency or may be intentional. Please either\n"
             "- check that all the mesh generators are connected as a tree and culminate in a "
             "single final mesh. Having one wrong 'input=mg' parameter is the most common error\n"
             "- add additional dependencies to remove the ambiguity if you are using a user-built "
             "MeshGenerator\n"
             "- if you intend to execute a subset of the defined generators (uncommon), select the"
             " final MeshGenerator in the [Mesh] block with the \"final_generator\" parameter.";
      mooseError(oss.str());
    }

    _final_generator_name = final_generators.back()->name();
  }
  else
    mooseAssert(hasMeshGenerator(_final_generator_name), "Missing the preset final generator");
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

  std::map<std::string, std::unique_ptr<MeshBase> *> to_save_in_meshes;

  // Loop over the MeshGenerators and save all meshes marked to to_save_in_meshes
  for (const auto & generator_set : _ordered_mesh_generators)
    for (const auto & generator : generator_set)
      if (generator->hasSaveMesh())
      {
        if (_final_generator_name == generator->name())
          generator->paramError("save_with_name",
                                "Cannot use the save in capability with the final mesh generator");
        to_save_in_meshes.emplace(generator->getSavedMeshName(),
                                  &getMeshGeneratorOutput(generator->name()));
      }

  // Grab the outputs from the final generator so MeshGeneratorMesh can pick it up
  to_save_in_meshes.emplace(mainMeshGeneratorName(),
                            &getMeshGeneratorOutput(_final_generator_name));

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
    }
  }

  // Grab all the valid save in meshes from the temporary map to_save_in_meshes
  // and store them in _save_in_meshes
  for (auto & [name, mesh_ptr] : to_save_in_meshes)
  {
    mooseAssert(mesh_ptr, "Invalid pointer");
    mooseAssert(*mesh_ptr, "Invalid pointer");
    if (name != mainMeshGeneratorName())
      mooseAssert(_save_in_meshes.count(name), "Mesh has not been requested for save");
    _save_in_meshes[name] = std::move(*mesh_ptr);
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

  if (mg->hasSaveMesh())
  {
    if (_save_in_meshes.count(mg->getSavedMeshName()))
      mg->paramError("save_with_name",
                     "The save with name '",
                     mg->getSavedMeshName(),
                     "' has already been used");
    _save_in_meshes.emplace(mg->getSavedMeshName(), nullptr);
  }

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
  // - We directly depend on them and requested a mesh from them
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
                       "' was not used.\n\nDependencies that are declared by declareMeshForSub() "
                       "must be used as an input to a sub generator created by this object.");
    }
    // This was used by a sub generator but wasn't declared
    else if (is_sub_dependency)
      mg->mooseError(
          "The MeshGenerator '",
          dependency_name,
          "' was referenced in the parameter '",
          param_name,
          "' and used in the sub generator ",
          (*find_sub_dependency)->type(),
          " '",
          (*find_sub_dependency)->name(),
          "', but was not declared as a sub dependency.\n\nTo correct this, modify the code of ",
          mg->type(),
          " to include a call to declareMesh(es)ForSub(\"",
          param_name,
          "\") in the constructor.");
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

std::vector<std::string>
MeshGeneratorSystem::getSavedMeshNames() const
{
  std::vector<std::string> names;
  for (auto & pair : _save_in_meshes)
    names.push_back(pair.first);
  return names;
}

std::unique_ptr<MeshBase>
MeshGeneratorSystem::getSavedMesh(const std::string & name)
{
  auto find_mesh = _save_in_meshes.find(name);
  if (find_mesh == _save_in_meshes.end())
    mooseError("Failed to find a saved mesh with the name '", name, "'");

  auto & mesh_unique_ptr = find_mesh->second;
  if (!mesh_unique_ptr)
    mooseError("While getting the saved mesh generator '",
               name,
               "', said mesh has already been retreived");

  return std::move(mesh_unique_ptr);
}

bool
MeshGeneratorSystem::appendingMeshGenerators() const
{
  return _app.actionWarehouse().getCurrentTaskName() == "append_mesh_generator";
}
