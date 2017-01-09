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

#ifndef MATERIALPROPERTYINTERFACE_H
#define MATERIALPROPERTYINTERFACE_H

// MOOSE includes
#include "MooseTypes.h"
#include "MaterialProperty.h"
#include "MaterialData.h"
#include "FEProblem.h"
#include "InputParameters.h"

// Forward declarations
class MaterialPropertyInterface;
class MooseObject;

template<>
InputParameters validParams<MaterialPropertyInterface>();


/**
 * \class MaterialPropertyInterface
 * \brief An interface for accessing Materials
 *
 * Any object that needs material properties should inherit this interface.
 * If your object is also restricted to blocks and/or boundaries via the
 * BlockRestrictable and/or BoundaryRestrictable class, then MaterialPropertyInterface
 * must be inherited following these two classes for the material property checks
 * to operate correctly.
 */
class MaterialPropertyInterface
{
public:

  ///@{
  /**
   * Constructor.
   *
   * @param parameters The objects input parameters
   * @param block_ids A reference to the block ids (optional)
   * @param boundary_ids A reference to the boundary ids (optional)
   *
   * This class has four constructors:
   *   (1) not restricted to boundaries or blocks
   *   (2) restricted to only blocks
   *   (3) restricted to only boundaries
   *   (4) restricted to both blocks and boundaries
   */
  MaterialPropertyInterface(const MooseObject * moose_object);
  MaterialPropertyInterface(const MooseObject * moose_object, const std::set<SubdomainID> & block_ids);
  MaterialPropertyInterface(const MooseObject * moose_object, const std::set<BoundaryID> & boundary_ids);
  MaterialPropertyInterface(const MooseObject * moose_object, const std::set<SubdomainID> & block_ids, const std::set<BoundaryID> & boundary_ids);
  ///@}

  ///@{
  /**
   * Retrieve reference to material property or one of it's old or older values.
   * The name required by this method is the name that is hard-coded into
   * your source code as the input parameter key. If no input parameter is found
   * this behaves like the getMaterialPropertyByName family as a fall back.
   * @param name The name of the parameter key of the material property to retrieve
   * @return Reference to the desired material property
   */
  template<typename T>
  const MaterialProperty<T> & getMaterialProperty(const std::string & name);
  template<typename T>
  const MaterialProperty<T> & getMaterialPropertyOld(const std::string & name);
  template<typename T>
  const MaterialProperty<T> & getMaterialPropertyOlder(const std::string & name);
  ///@}

  ///@{
  /**
   * Retrieve reference to material property or its old or older value
   * The name required by this method is the name defined in the input file.
   * @param name The name of the material property to retrieve
   * @return Reference to the material property with the name 'name'
   */
  template<typename T>
  const MaterialProperty<T> & getMaterialPropertyByName(const MaterialPropertyName & name);
  template<typename T>
  const MaterialProperty<T> & getMaterialPropertyOldByName(const MaterialPropertyName & name);
  template<typename T>
  const MaterialProperty<T> & getMaterialPropertyOlderByName(const MaterialPropertyName & name);
  ///@}

  /**
   * Retrieve pointer to a material property with the mesh blocks where it is defined
   * The name required by this method is the name defined in the input file.
   * This function can be thought as the combination of getMaterialPropertyByName and getMaterialPropertyBlocks.
   * It can be called after the action of all actions.
   * @param name The name of the material property to retrieve
   * @return Pointer to the material property with the name 'name' and the set of blocks where the property is valid
   */
  template<typename T>
  std::pair<const MaterialProperty<T> *, std::set<SubdomainID> > getBlockMaterialProperty(const MaterialPropertyName & name);

  /**
   * Return a material property that is initialized to zero by default and does
   * not need to (but can) be declared by another material.
   */
  template<typename T>
  const MaterialProperty<T> & getZeroMaterialProperty(const std::string & prop_name);

  /**
   * Retrieve the block ids that the material property is defined
   * @param name The name of the material property
   * @return A vector the the block ids for the property
   */
  std::set<SubdomainID> getMaterialPropertyBlocks(const std::string & name);

  /**
   * Retrieve the block names that the material property is defined
   * @param name The name of the material property
   * @return A vector the the block names for the property
   */
  std::vector<SubdomainName> getMaterialPropertyBlockNames(const std::string & name);

  /**
   * Retrieve the boundary ids that the material property is defined
   * @param name The name of the material property
   * @return A vector the the boundary ids for the property
   */
  std::set<BoundaryID> getMaterialPropertyBoundaryIDs(const std::string & name);

  /**
   * Retrieve the boundary namess that the material property is defined
   * @param name The name of the material property
   * @return A vector the the boundary names for the property
   */
  std::vector<BoundaryName> getMaterialPropertyBoundaryNames(const std::string & name);

  ///@{
  /**
   * Return a Material object reference for calling compute directly.
   * @param The name of the input parameter or explicit material name.
   */
  Material & getMaterial(const std::string & name);
  Material & getMaterialByName(const std::string & name);
  ///@}

  ///@{
  /**
   * Check if the material property exists
   * @param name the name of the property to query
   * @return true if the property exists, otherwise false
   */
  template<typename T>
  bool hasMaterialProperty(const std::string & name);
  template<typename T>
  bool hasMaterialPropertyByName(const std::string & name);
  ///@}

  /**
   * Derived classes can declare whether or not they work with
   * stateful material properties.  See, for example, DiracKernel.  By
   * default, they are allowed.
   */
  void statefulPropertiesAllowed(bool);

  /**
   * Returns true if getMaterialProperty() has been called, false otherwise.
   */
  bool getMaterialPropertyCalled() const { return _get_material_property_called; }

protected:
  /// Parameters of the object with this interface
  const InputParameters & _mi_params;

  /// The name of the object that this interface belongs to
  const std::string _mi_name;

  /// The type of data
  Moose::MaterialDataType _material_data_type;

  /// Pointer to the material data class that stores properties
  MooseSharedPointer<MaterialData> _material_data;

  /// Reference to the FEProblemBase class
  FEProblemBase & _mi_feproblem;

  /// Current threaded it
  const THREAD_ID _mi_tid;

  /**
   * A helper method for checking material properties
   * This method was required to avoid a compiler problem with the template
   * getMaterialProperty method
   */
  void checkMaterialProperty(const std::string & name);

  /**
   * A proxy method for _mi_feproblem.markMatPropRequested(name)
   */
  void markMatPropRequested(const std::string &);

  /**
   * Small helper to look up a material property name through the input parameter keys
   */
  std::string deducePropertyName(const std::string & name);

  /**
   * Helper function to parse default material property values. This is implemented
   * as a specialization for supported types and returns NULL in all other cases.
   */
  template<typename T>
  const MaterialProperty <T> * defaultMaterialProperty(const std::string & name);

  /**
   * A helper method for extracting the Material object from the MaterialWarehouse. In general, this method
   * should not be used, please use `getMaterial` or `getMaterialByName`.
   * @param The name of the material to retrieve.
   * @return A shared pointer to the Material object.
   */
   virtual MooseSharedPointer<Material> getMaterialSharedPointerByName(const std::string & name);

  /**
   * True by default. If false, this class throws an error if any of
   * the stateful material properties interfaces are used.
   */
  bool _stateful_allowed;

  /**
   * Initialized to false.  Gets set to true when getMaterialProperty()
   * is called.  Clients of this class can inquire whether getMaterialProperty()
   * has been called by calling getMaterialPropertyCalled().
   */
  bool _get_material_property_called;

  /// Storage vector for MaterialProperty<Real> default objects
  std::vector<std::unique_ptr<MaterialProperty<Real>>> _default_real_properties;

private:
  /// An initialization routine needed for dual constructors
  void initializeMaterialPropertyInterface(const InputParameters & parameters);

  /// Check and throw an error if the execution has progerssed past the construction stage
  void checkExecutionStage();

  /// Empty sets for referencing when ids is not included
  const std::set<SubdomainID> _empty_block_ids;

  /// An empty set for referencing when boundary_ids is not included
  const std::set<BoundaryID> _empty_boundary_ids;

  /// Storage for the block ids created by BlockRestrictable
  const std::set<SubdomainID> _mi_block_ids;

  /// Storage for the boundary ids created by BoundaryRestrictable
  const std::set<BoundaryID> _mi_boundary_ids;
};

/**
 * Helper function templates to set a variable to zero.
 * Specializations may have to be implemented (for examples see
 * RankTwoTensor, RankFourTensor).
 */
template<typename T>
inline void mooseSetToZero(T & v)
{
  /**
   * The default for non-pointer types is to assign zero.
   * This should either do something sensible, or throw a compiler error.
   * Otherwise the T type is designed badly.
   */
  v = 0;
}
template<typename T>
inline void mooseSetToZero(T* &)
{
  mooseError("Cannot use pointer types for MaterialProperty derivatives.");
}

template<typename T>
const MaterialProperty<T> &
MaterialPropertyInterface::getMaterialProperty(const std::string & name)
{
  // Check if the supplied parameter is a valid input parameter key
  std::string prop_name = deducePropertyName(name);

  // Check if it's just a constant
  const MaterialProperty<T> * default_property = defaultMaterialProperty<T>(prop_name);
  if (default_property)
    return *default_property;

  return getMaterialPropertyByName<T>(prop_name);
}

template<typename T>
const MaterialProperty<T> &
MaterialPropertyInterface::getMaterialPropertyOld(const std::string & name)
{
  if (!_stateful_allowed)
    mooseError("Stateful material properties not allowed for this object."
               " Old property for \"" << name << "\" was requested.");

  // Check if the supplied parameter is a valid input parameter key
  std::string prop_name = deducePropertyName(name);

  // Check if it's just a constant
  const MaterialProperty<T> * default_property = defaultMaterialProperty<T>(prop_name);
  if (default_property)
    return *default_property;

  return getMaterialPropertyOldByName<T>(prop_name);
}

template<typename T>
const MaterialProperty<T> &
MaterialPropertyInterface::getMaterialPropertyOlder(const std::string & name)
{
  if (!_stateful_allowed)
    mooseError("Stateful material properties not allowed for this object."
               " Older property for \"" << name << "\" was requested.");

  // Check if the supplied parameter is a valid input parameter key
  std::string prop_name = deducePropertyName(name);

  // Check if it's just a constant
  const MaterialProperty<T> * default_property = defaultMaterialProperty<T>(prop_name);
  if (default_property)
    return *default_property;

  return getMaterialPropertyOlderByName<T>(prop_name);
}

// General version for types that do not accept default values
template<typename T>
const MaterialProperty<T> *
MaterialPropertyInterface::defaultMaterialProperty(const std::string & /*name*/)
{
  return NULL;
}

// Forward declare explicit specializations
template<>
const MaterialProperty<Real> *
MaterialPropertyInterface::defaultMaterialProperty(const std::string & name);

template<typename T>
const MaterialProperty<T> &
MaterialPropertyInterface::getMaterialPropertyByName(const MaterialPropertyName & name)
{
  checkExecutionStage();
  checkMaterialProperty(name);

  // mark property as requested
  markMatPropRequested(name);

  // Update the boolean flag.
  _get_material_property_called = true;

  return _material_data->getProperty<T>(name);
}


template<typename T>
const MaterialProperty<T> &
MaterialPropertyInterface::getMaterialPropertyOldByName(const MaterialPropertyName & name)
{
  if (!_stateful_allowed)
    mooseError("Stateful material properties not allowed for this object."
               " Old property for \"" << name << "\" was requested.");

  // mark property as requested
  markMatPropRequested(name);

  return _material_data->getPropertyOld<T>(name);
}

template<typename T>
const MaterialProperty<T> &
MaterialPropertyInterface::getMaterialPropertyOlderByName(const MaterialPropertyName & name)
{
  if (!_stateful_allowed)
    mooseError("Stateful material properties not allowed for this object."
               " Older property for \"" << name << "\" was requested.");

  // mark property as requested
  markMatPropRequested(name);

  return _material_data->getPropertyOlder<T>(name);
}

template<typename T>
std::pair<const MaterialProperty<T> *, std::set<SubdomainID> >
MaterialPropertyInterface::getBlockMaterialProperty(const MaterialPropertyName & name)
{
  if (_mi_block_ids.empty())
    mooseError("getBlockMaterialProperty must be called by a block restrictable object");

  if (!hasMaterialPropertyByName<T>(name))
    return std::pair<const MaterialProperty<T> *, std::set<SubdomainID> >(NULL, std::set<SubdomainID>());

  return std::pair<const MaterialProperty<T> *, std::set<SubdomainID> >(&_material_data->getProperty<T>(name), _mi_feproblem.getMaterialPropertyBlocks(name));
}

template<typename T>
bool
MaterialPropertyInterface::hasMaterialProperty(const std::string & name)
{
  // Check if the supplied parameter is a valid input parameter key
  std::string prop_name = deducePropertyName(name);
  return _material_data->haveProperty<T>(prop_name);
}

template<typename T>
bool
MaterialPropertyInterface::hasMaterialPropertyByName(const std::string & name)
{
  return _material_data->haveProperty<T>(name);
}

template<typename T>
const MaterialProperty<T> &
MaterialPropertyInterface::getZeroMaterialProperty(const std::string & /*prop_name*/)
{
  // static zero property storage
  static MaterialProperty<T> zero;

  // resize to accomodate maximum number of qpoints
  unsigned int nqp = _mi_feproblem.getMaxQps();
  zero.resize(nqp);

  // set values for all qpoints to zero
  for (unsigned int qp = 0; qp < nqp; ++qp)
    mooseSetToZero<T>(zero[qp]);

  return zero;
}

#endif //MATERIALPROPERTYINTERFACE_H
