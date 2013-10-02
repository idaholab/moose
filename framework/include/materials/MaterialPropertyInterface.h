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
 * must be inherieted following these two classes for the material property checks
 * to operate correctly.
 */
class MaterialPropertyInterface
{
public:
  MaterialPropertyInterface(InputParameters & parameters);

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
  std::vector<SubdomainID> getMaterialPropertyBlocks(const std::string & name);

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
  std::vector<BoundaryID> getMaterialPropertyBoundaryIDs(const std::string & name);

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

protected:

  /// Reference to the materail data class that stores properties
  MaterialData & _material_data;

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
};

template<typename T>
MaterialProperty<T> &
MaterialPropertyInterface::getMaterialProperty(const std::string & name)
{
  checkMaterialProperty(name);
  return _material_data.getProperty<T>(name);
}

template<typename T>
MaterialProperty<T> &
MaterialPropertyInterface::getMaterialPropertyOld(const std::string & name)
{
  return _material_data.getPropertyOld<T>(name);
}

template<typename T>
MaterialProperty<T> &
MaterialPropertyInterface::getMaterialPropertyOlder(const std::string & name)
{
  return _material_data.getPropertyOlder<T>(name);
}

template<typename T>
bool
MaterialPropertyInterface::hasMaterialProperty(const std::string & name)
{
  return _material_data.haveProperty<T>(name);
}

#endif //MATERIALPROPERTYINTERFACE_H
