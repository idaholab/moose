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

#include <map>
#include <string>

#include "MaterialProperty.h"
#include "InputParameters.h"
#include "MaterialData.h"

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
  MaterialPropertyInterface(const std::string & name, InputParameters & parameters);

  /**
   * Retrieve reference to material property (current time)
   * @param name The name of the material property
   * @return Reference to the desired material property
   */
  template<typename T>
  MaterialProperty<T> & getMaterialProperty(const std::string & name);

  /**
   * Retrieve reference to material property (old time)
   * @param name The name of the material property
   * @return Reference to the desired material property
   */
  template<typename T>
  MaterialProperty<T> & getMaterialPropertyOld(const std::string & name);

  /**
   * Retrieve reference to material property (older time)
   * @param name The name of the material property
   * @return Reference to the desired material property
   */
  template<typename T>
  MaterialProperty<T> & getMaterialPropertyOlder(const std::string & name);

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

  /**
   * Check if the material property exists
   * @param name the name of the property to query
   * @return true if the property exists, otherwise false
   */
  template<typename T>
  bool hasMaterialProperty(const std::string & name);

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
  /// The name of the object that this interface belongs to
  std::string _mi_name;

  /// Pointer to the material data class that stores properties
  MaterialData * _material_data;

  /// Reference to the FEProblem class
  FEProblem & _mi_feproblem;

  /// Storage for the block ids created by BlockRestrictable
  std::vector<SubdomainID> _mi_block_ids;

  /// Storage for the boundary ids created by BoundaryRestrictable
  std::vector<BoundaryID> _mi_boundary_ids;

  /**
   * A helper method for checking material properties
   * This method was required to avoid a compiler problem with the templated
   * getMaterialProperty method
   */
  void checkMaterialProperty(const std::string & name);

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
};

template<typename T>
MaterialProperty<T> &
MaterialPropertyInterface::getMaterialProperty(const std::string & name)
{
  checkMaterialProperty(name);

  if (!_stateful_allowed && _material_data->getMaterialPropertyStorage().hasStatefulProperties())
    mooseError("Error: Stateful material properties not allowed for this object.");

  // Update the boolean flag.
  _get_material_property_called = true;

  return _material_data->getProperty<T>(name);
}

template<typename T>
MaterialProperty<T> &
MaterialPropertyInterface::getMaterialPropertyOld(const std::string & name)
{
  if (!_stateful_allowed)
    mooseError("Error: Stateful material properties not allowed for this object.");

  return _material_data->getPropertyOld<T>(name);
}

template<typename T>
MaterialProperty<T> &
MaterialPropertyInterface::getMaterialPropertyOlder(const std::string & name)
{
  if (!_stateful_allowed)
    mooseError("Error: Stateful material properties not allowed for this object.");

  return _material_data->getPropertyOlder<T>(name);
}

template<typename T>
bool
MaterialPropertyInterface::hasMaterialProperty(const std::string & name)
{
  return _material_data->haveProperty<T>(name);
}

#endif //MATERIALPROPERTYINTERFACE_H
