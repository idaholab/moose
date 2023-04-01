//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObject.h"
#include "MeshMetaDataInterface.h"
#include "MooseApp.h"

// Included so mesh generators don't need to include this when constructing MeshBase objects
#include "MooseMesh.h"

#include "libmesh/mesh_base.h"
#include "libmesh/parameters.h"

class MooseMesh;
namespace libMesh
{
class ReplicatedMesh;
class DistributedMesh;
}

/**
 * MeshGenerators are objects that can modify or add to an existing mesh.
 */
class MeshGenerator : public MooseObject, public MeshMetaDataInterface
{
public:
  /**
   * Comparator for MeshGenerators that sorts by name
   */
  struct Comparator
  {
    bool operator()(const MeshGenerator * const & a, const MeshGenerator * const & b) const
    {
      return a->name() < b->name();
    }
  };

  /**
   * Constructor
   *
   * @param parameters The parameters object holding data for the class to use.
   */
  static InputParameters validParams();

  MeshGenerator(const InputParameters & parameters);

  /**
   * Generate / modify the mesh
   */
  virtual std::unique_ptr<MeshBase> generate() = 0;

  /**
   * Internal generation method - this is what is actually called
   * within MooseApp to execute the MeshGenerator.
   */
  [[nodiscard]] std::unique_ptr<MeshBase> generateInternal();

  /**
   * @returns The names of the MeshGenerators that were requested in the getMesh methods
   */
  const std::set<MeshGeneratorName> & getRequestedMeshGenerators() const
  {
    return _requested_mesh_generators;
  }
  /**
   * @returns The names of the MeshGenerators that were requested in the declareMeshForSub
   * methods
   */
  const std::set<MeshGeneratorName> & getRequestedMeshGeneratorsForSub() const
  {
    return _requested_mesh_generators_for_sub;
  }

  /**
   * Class that is used as a parameter to add[Parent/Child]() that allows only
   * MooseApp methods to call said methods
   */
  class AddParentChildKey
  {
    friend class MeshGeneratorSystem;
    AddParentChildKey() {}
    AddParentChildKey(const AddParentChildKey &) {}
  };

  /**
   * Adds the MeshGenerator \p mg as a parent.
   *
   * Protected by the AddParentChildKey so that parents can only be
   * added by the MooseApp.
   */
  void addParentMeshGenerator(const MeshGenerator & mg, const AddParentChildKey);
  /**
   * Adds the MeshGenerator \p mg as a child.
   *
   * Protected by the AddParentChildKey so that parents can only be
   * added by the MooseApp.
   */
  void addChildMeshGenerator(const MeshGenerator & mg, const AddParentChildKey);

  /**
   * Gets the MeshGenerators that are parents to this MeshGenerator.
   */
  const std::set<const MeshGenerator *, Comparator> & getParentMeshGenerators() const
  {
    return _parent_mesh_generators;
  }
  /**
   * Gets the MeshGenerators that are children to this MeshGenerator.
   */
  const std::set<const MeshGenerator *, Comparator> & getChildMeshGenerators() const
  {
    return _child_mesh_generators;
  }
  /**
   * Gets the MeshGenerators that are children to this MeshGenerator.
   */
  const std::set<const MeshGenerator *, Comparator> & getSubMeshGenerators() const
  {
    return _sub_mesh_generators;
  }

  /**
   * @returns Whether or not the MeshGenerator with the name \p name is a parent of this
   * MeshGenerator.
   */
  bool isParentMeshGenerator(const MeshGeneratorName & name, const bool direct = true) const;

  /**
   * @returns Whether or not the MeshGenerator with the name \p name is a child of this
   * MeshGenerator.
   */
  bool isChildMeshGenerator(const MeshGeneratorName & name, const bool direct = true) const;

  /**
   * @returns Whether or not the name \p name is registered as a "null" mesh, that is,
   * a MeshGenerator that will not represent an input mesh when requested via getMesh.
   *
   * See declareNullMeshName().
   */
  bool isNullMeshName(const MeshGeneratorName & name) const { return _null_mesh_names.count(name); }

  /**
   * Return whether or not to save the current mesh
   */
  bool hasSaveMesh();

  /**
   * Return the name of the saved mesh
   */
  const std::string & getSavedMeshName() const;

protected:
  /**
   * Methods for writing out attributes to the mesh meta-data store, which can be retrieved from
   * most other MOOSE systems and is recoverable.
   */
  ///@{
  template <typename T, typename... Args>
  T & declareMeshProperty(const std::string & data_name, Args &&... args);
  template <typename T>
  T & declareMeshProperty(const std::string & data_name, const T & data_value)
  {
    return declareMeshProperty<T, const T &>(data_name, data_value);
  }
  ///@}

  /**
   * Method for updating attributes to the mesh meta-data store, which can only be invoked in
   * the MeshGenerator generate routine only if the mesh generator property has already been
   * declared.
   */
  ///@{
  template <typename T, typename... Args>
  T & setMeshProperty(const std::string & data_name, Args &&... args);
  template <typename T>
  T & setMeshProperty(const std::string & data_name, const T & data_value)
  {
    return setMeshProperty<T, const T &>(data_name, data_value);
  }
  ///@}

  /**
   * Takes the name of a MeshGeneratorName parameter and then gets a pointer to the
   * Mesh that MeshGenerator is going to create.
   *
   * That MeshGenerator is made to be a dependency of this one, so
   * will generate() its mesh first.
   *
   * @param param_name The name of the parameter that contains the name of the MeshGenerator
   * @param allow_invalid If true, will allow for invalid parameters and will return a nullptr
   * mesh if said parameter does not exist
   * @return The Mesh generated by that MeshGenerator
   *
   * NOTE: You MUST catch this by reference!
   */
  [[nodiscard]] std::unique_ptr<MeshBase> & getMesh(const std::string & param_name,
                                                    const bool allow_invalid = false);
  /**
   * Like getMesh(), but for multiple generators.
   *
   * @returns The generated meshes
   */
  [[nodiscard]] std::vector<std::unique_ptr<MeshBase> *> getMeshes(const std::string & param_name);
  /**
   * Like \p getMesh(), but takes the name of another MeshGenerator directly.
   *
   * NOTE: You MUST catch this by reference!
   *
   * @return The Mesh generated by that MeshGenerator
   */
  [[nodiscard]] std::unique_ptr<MeshBase> &
  getMeshByName(const MeshGeneratorName & mesh_generator_name);
  /**
   * Like getMeshByName(), but for multiple generators.
   *
   * @returns The generated meshes
   */
  [[nodiscard]] std::vector<std::unique_ptr<MeshBase> *>
  getMeshesByName(const std::vector<MeshGeneratorName> & mesh_generator_names);

  /**
   * Declares that a MeshGenerator referenced in the InputParameters is to be
   * used as a dependency of a sub MeshGenerator created by this MeshGenerator
   * (see addSubMeshGenerator)
   *
   * You _must_ declare all such MeshGenerators that are passed by parameter to this
   * MeshGenerator but instead used in a sub MeshGenerator. This is in order to
   * declare the intention to use an input mesh as a dependency for a sub generator
   * instead of this one.
   */
  void declareMeshForSub(const std::string & param_name);
  /**
   * Like declareMeshForSub(), but for multiple generators.
   */
  void declareMeshesForSub(const std::string & param_name);
  /**
   * Like declareMeshForSub(), but takes the name of another MeshGenerator directly.
   */
  void declareMeshForSubByName(const MeshGeneratorName & mesh_generator_name);
  /**
   * Like declareMeshForSubByName(), but for multiple generators.
   */
  void declareMeshesForSubByName(const std::vector<MeshGeneratorName> & mesh_generator_names);

  /**
   * Build a \p MeshBase object whose underlying type will be determined by the Mesh input file
   * block
   * @param dim The logical dimension of the mesh, e.g. 3 for hexes/tets, 2 for quads/tris. If the
   * caller doesn't specify a value for \p dim, then the value in the \p Mesh input file block will
   * be used
   */
  [[nodiscard]] std::unique_ptr<MeshBase>
  buildMeshBaseObject(unsigned int dim = libMesh::invalid_uint);

  /**
   * Build a replicated mesh
   * @param dim The logical dimension of the mesh, e.g. 3 for hexes/tets, 2 for quads/tris. If the
   * caller doesn't specify a value for \p dim, then the value in the \p Mesh input file block will
   * be used
   */
  [[nodiscard]] std::unique_ptr<ReplicatedMesh>
  buildReplicatedMesh(unsigned int dim = libMesh::invalid_uint);

  /**
   * Build a distributed mesh that has correct remote element removal behavior and geometric
   * ghosting functors based on the simulation objects
   * @param dim The logical dimension of the mesh, e.g. 3 for hexes/tets, 2 for quads/tris. If the
   * caller doesn't specify a value for \p dim, then the value in the \p Mesh input file block will
   * be used
   */
  [[nodiscard]] std::unique_ptr<DistributedMesh>
  buildDistributedMesh(unsigned int dim = libMesh::invalid_uint);

  /**
   * Construct a "subgenerator", a different MeshGenerator subclass
   * that will be added to the same MooseApp on the fly.
   *
   * @param type The type of MeshGenerator
   * @param name The name of the MeshGenerator
   * @param extra_input_parameters ... Additional InputParameters to pass
   *
   * Sub generators must be added in the order that they are executed.
   *
   * Any input dependencies for a sub generator that come from inputs for
   * this generator must first be declared with the declareMesh(es)ForSub()
   * method.
   *
   * You can use the output of a sub generator as a mesh in this generator
   * by calling getMesh() with the sub generator's name _after_ adding
   * said sub generator.
   */
  template <typename... Ts>
  void addMeshSubgenerator(const std::string & type,
                           const std::string & name,
                           Ts... extra_input_parameters);

  /**
   * Construct a "subgenerator", as above.  User code is responsible
   * for constructing valid InputParameters.
   *
   * @param type The type of MeshGenerator
   * @param name The name of the MeshGenerator
   * @param params The parameters to use to construct the MeshGenerator
   */
  void
  addMeshSubgenerator(const std::string & type, const std::string & name, InputParameters params);

  /**
   * Registers the name \p name as a "null" mesh, which is a MeshGenerator used in
   * InputParameters that will not represent an input mesh when requested via getMesh.
   *
   * An example use case for this is when you as a developer want users to represent a hole
   * in a mesh pattern that is defined in input.
   *
   */
  void declareNullMeshName(const MeshGeneratorName & name);

  /// References to the mesh and displaced mesh (currently in the ActionWarehouse)
  const std::shared_ptr<MooseMesh> & _mesh;

private:
  /**
   * Override of the default prefix to use when getting mesh properties.
   *
   * Until we support getting mesh properties from other mesh generators (which is coming),
   * we will default to properties returned by this generator.
   */
  virtual std::string meshPropertyPrefix(const std::string &) const override final
  {
    return name();
  }

  /**
   * Helper for performing error checking in the getMesh methods.
   */
  void checkGetMesh(const MeshGeneratorName & mesh_generator_name,
                    const std::string & param_name) const;

  /**
   * Helper for getting a MeshGeneratorName parameter
   */
  const MeshGeneratorName * getMeshGeneratorNameFromParam(const std::string & param_name,
                                                          const bool allow_invalid) const;
  /**
   * Helper for getting a std::vector<MeshGeneratorName> parameter
   */
  const std::vector<MeshGeneratorName> &
  getMeshGeneratorNamesFromParam(const std::string & param_name) const;

  /**
   * Helper for getting a writable reference to a mesh property, used in
   * declareMeshProperty and setMeshProperty.
   */
  RestartableDataValue & setMeshPropertyHelper(const std::string & data_name);

  /// The names of the MeshGenerators that were requested in the getMesh methods
  std::set<MeshGeneratorName> _requested_mesh_generators;
  /// The names of the MeshGenerators that were requested in the declareMeshForSub methods
  std::set<MeshGeneratorName> _requested_mesh_generators_for_sub;
  /// The meshes that were requested by this MeshGenerator; used to verify that
  /// any input meshes that are requested are properly released after generation
  std::vector<std::pair<std::string, std::unique_ptr<MeshBase> *>> _requested_meshes;

  /// A nullptr to use for when inputs aren't specified
  std::unique_ptr<MeshBase> _null_mesh = nullptr;

  /// The MeshGenerators that are parents to this MeshGenerator
  std::set<const MeshGenerator *, Comparator> _parent_mesh_generators;
  /// The MeshGenerators that are children to this MeshGenerator
  std::set<const MeshGenerator *, Comparator> _child_mesh_generators;
  /// The sub MeshGenerators constructed by this MeshGenerator
  std::set<const MeshGenerator *, Comparator> _sub_mesh_generators;

  /// The declared "null" mesh names that will not represent a mesh in input; see declareNullMeshName()
  std::set<std::string> _null_mesh_names;

  /// A user-defined name to save the mesh
  const std::string & _save_with_name;
};

template <typename T, typename... Args>
T &
MeshGenerator::declareMeshProperty(const std::string & data_name, Args &&... args)
{
  if (!_app.constructingMeshGenerators())
    mooseError("Can only call declareMeshProperty() during MeshGenerator construction");

  // We sort construction ordering so that we _must_ declare before retrieving
  if (hasMeshProperty(data_name))
    mooseError("While declaring mesh property '",
               data_name,
               "' with type '",
               MooseUtils::prettyCppType<T>(),
               "',\nsaid property has already been declared with type '",
               setMeshPropertyHelper(data_name).type(),
               "'");

  const auto full_name = meshPropertyName(data_name);
  auto new_T_value =
      std::make_unique<RestartableData<T>>(full_name, nullptr, std::forward<Args>(args)...);
  auto value = &_app.registerRestartableData(
      full_name, std::move(new_T_value), 0, false, MooseApp::MESH_META_DATA);
  mooseAssert(value->declared(), "Should be declared");

  RestartableData<T> * T_value = dynamic_cast<RestartableData<T> *>(value);
  mooseAssert(T_value, "Bad cast");

  return T_value->set();
}

template <typename T, typename... Args>
T &
MeshGenerator::setMeshProperty(const std::string & data_name, Args &&... args)
{
  if (_app.actionWarehouse().getCurrentTaskName() != "execute_mesh_generators")
    mooseError("Updating mesh meta data with setMeshProperty() can only be called during "
               "MeshGenerator::generate()");

  if (!hasMeshProperty(data_name))
    mooseError("Failed to get the mesh property '", data_name, "'");
  RestartableDataValue * value = &setMeshPropertyHelper(data_name);
  RestartableData<T> * T_value = dynamic_cast<RestartableData<T> *>(value);
  if (!T_value)
    mooseError("While retrieving mesh property '",
               data_name,
               "' with type '",
               MooseUtils::prettyCppType<T>(),
               "',\nthe property was found with type '",
               value->type(),
               "'");

  // Set the value if someone provided arguments to set it to
  if constexpr (sizeof...(args) > 0)
    T_value->set() = T(std::forward<Args>(args)...);

  return T_value->set();
}

template <typename... Ts>
void
MeshGenerator::addMeshSubgenerator(const std::string & type,
                                   const std::string & name,
                                   Ts... extra_input_parameters)
{
  InputParameters subgenerator_params = _app.getFactory().getValidParams(type);

  subgenerator_params.setParameters(extra_input_parameters...);

  addMeshSubgenerator(type, name, subgenerator_params);
}
