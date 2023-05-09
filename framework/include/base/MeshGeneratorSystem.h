//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InputParameters.h"
#include "PerfGraphInterface.h"

#include "libmesh/parallel_object.h"
#include "libmesh/mesh_base.h"

class MooseApp;
class MeshGeneratorMesh;
class MeshGenerator;

/**
 * System that manages MeshGenerators.
 *
 * To be owned by the MooseApp.
 */
class MeshGeneratorSystem : public PerfGraphInterface, public libMesh::ParallelObject
{
public:
  MeshGeneratorSystem(MooseApp & app);

  /**
   * Execute and clear the Mesh Generators data structure
   */
  void executeMeshGenerators();

  /**
   * Add a mesh generator that will act on the meshes in the system
   *
   * @param type The type of MeshGenerator
   * @param name The name of the MeshGenerator
   * @param params The params used to construct the MeshGenerator
   *
   * Internally, this will store the parameters for future construction
   * during the "add_mesh_generator" task. When called during the
   * "create_mesh_generator" task (i.e., when creating mesh subgenerators),
   * it will also construct the generator.
   *
   * We don't construct them yet because we want to create them in order
   * during createMeshGenerators() as much as possible so that we don't
   * need lazy construction for things like mesh properties.
   */
  void addMeshGenerator(const std::string & type,
                        const std::string & name,
                        const InputParameters & params);

  /**
   * Append a mesh generator that will act on the current final mesh generator in the system
   *
   * @param type The type of MeshGenerator
   * @param name The name of the MeshGenerator
   * @param params The params used to construct the MeshGenerator
   *
   * This MeshGenerator must have a parameter "input" of type MeshGeneratorName
   * for this to work, as said parameter is set to the current final generator
   *
   * Note: This function must be called during the append_mesh_generator task.
   */
  const MeshGenerator &
  appendMeshGenerator(const std::string & type, const std::string & name, InputParameters params);

  /**
   * Get a reference to a pointer that will be the output of the
   * MeshGenerator named name
   */
  [[nodiscard]] std::unique_ptr<MeshBase> & getMeshGeneratorOutput(const MeshGeneratorName & name);

  /**
   * Creates (constructs) all of the MeshGenerators that have been
   * declared using addMeshGenerator().
   *
   * This parses the input parameters of type <MeshGenerator> and
   * std::vector<MeshGeneratorName> to build the execution tree for
   * the generators, and constructs them in dependency order
   *
   * Sub generators are also generated within this phase, although the
   * dependency resolution described above is only for the dependencies
   * that we can parse using InputParameters. However, we require that
   * sub generators be constructed within _their_ dependency order
   * (except for the last one, which may be coupled to via the generator
   * creating said sub generator).
   *
   * Should only be called by the CreateAddedMeshGenerator action during
   * the create_added_mesh_generators task.
   */
  void createAddedMeshGenerators();

  /**
   * Get names of all mesh generators
   * Note: This function should be called after all mesh generators are added with the
   * 'add_mesh_generator' task. The returned value will be undefined and depends on the ordering
   * that mesh generators are added by MOOSE if the function is called during the
   * 'add_mesh_generator' task.
   */
  std::vector<std::string> getMeshGeneratorNames() const;

  /**
   * Get the saved mesh by name
   */
  [[nodiscard]] std::unique_ptr<MeshBase> getSavedMesh(const std::string & name);

  /**
   * Get all user-defined saved meshes except main and main_displaced
   */
  std::vector<std::string> getSavedMeshNames() const;

  /**
   * @returns Whether or not a mesh generator exists with the name \p name.
   */
  bool hasMeshGenerator(const MeshGeneratorName & name) const;

  /**
   * @returns The MeshGenerator with the name \p name.
   */
  const MeshGenerator & getMeshGenerator(const std::string & name) const;

  /**
   * Whether or not we know about the parameters for a MeshGenerator with the given name
   *
   * This is primarily for error checking. If MeshGenerator dependencies are screwed up,
   * someone could be looking for a MeshGenerator that hasn't been constructed yet.
   * With this, at least we can give the user some context that we know the generator
   * exists, just that the dependencies are hosed.
   */
  bool hasMeshGeneratorParams(const MeshGeneratorName & name) const;

  /**
   * Whether or not mesh generators are currently being appended (append_mesh_generator task)
   */
  bool appendingMeshGenerators() const;

  /**
   * The name reserved for the "main" mesh generator which is the one used for the numerical solver
   * downstream
   */
  static std::string mainMeshGeneratorName() { return "main"; };

  /**
   * @return Whether any of our mesh generators were of type \p BreakMeshByBlockGenerator
   */
  bool hasBreakMeshByBlockGenerator() const { return _has_bmbb; }

private:
  /**
   * Gets the MeshGeneratorNames that are referenced in an object's parameters.
   *
   * The result is returned as a pair of param name -> MeshGeneratorName in order
   * to provide context.
   *
   * The \p allow_empty param sets whether or not to report names that are empty.
   */
  std::vector<std::pair<std::string, MeshGeneratorName>>
  getMeshGeneratorParamDependencies(const InputParameters & params,
                                    const bool allow_empty = false) const;

  /**
   * Order all of the _mesh_generators into _ordered_mesh_generators for later
   * execution in executeMeshGenerators
   */
  void createMeshGeneratorOrder();

  /**
   * Internal method for actually constructing a mesh generator after it
   * has been declared externally in addMeshGenerator (in Actions).
   */
  std::shared_ptr<MeshGenerator> createMeshGenerator(const std::string & name);

  /**
   * Get a MeshGenerator with the name \p name.
   *
   * We add the "internal" here so that we allow objects that have non-const access to
   * MooseApp to call getMeshGenerator without a const_cast. If the name was the same,
   * you'd get an error about trying to access a private method.
   */
  MeshGenerator & getMeshGeneratorInternal(const std::string & name)
  {
    return const_cast<MeshGenerator &>(std::as_const(*this).getMeshGenerator(name));
  }

  /// The MooseApp that owns this system
  MooseApp & _app;

  /// The MeshGenerators declared using addMeshGenerator(), cleared after createMeshGenerators()
  /// Key is the name, pair contains the type and the params
  std::unordered_map<std::string, std::pair<std::string, InputParameters>> _mesh_generator_params;

  /// Owning storage for mesh generators, map of name -> MeshGenerator
  std::map<std::string, std::shared_ptr<MeshGenerator>> _mesh_generators;

  /// Holds the ordered mesh generators from createMeshGeneratorOrder() until they are executed in executeMeshGenerators()
  std::vector<std::vector<MeshGenerator *>> _ordered_mesh_generators;

  /// Holds the output for each mesh generator - including duplicates needed downstream
  std::map<std::string, std::list<std::unique_ptr<MeshBase>>> _mesh_generator_outputs;

  /// The final mesh generator name to use
  std::string _final_generator_name;

  /// Holds the map of save in mesh -> name
  std::map<std::string, std::unique_ptr<MeshBase>> _save_in_meshes;

  /// Whether any of the mesh generators are a \p BreakMeshByBlockGenerator
  bool _has_bmbb;
};
