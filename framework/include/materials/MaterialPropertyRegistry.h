//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <string>
#include <unordered_map>
#include <vector>

class MaterialData;
class MaterialPropertyInterface;
class MaterialPropertyStorage;

/**
 * Registry class for material property IDs and names.
 *
 * Not thread safe.
 */
class MaterialPropertyRegistry
{
public:
  /**
   * @return True if a material property is registered with the name \p name
   */
  bool hasProperty(const std::string & name) const { return _name_to_id.count(name); }

  /**
   * @return True if a material property is registered with the ID \p id
   */
  bool hasProperty(const unsigned int id) const { return id < _id_to_name.size(); }

  /**
   * Key that restricts writing data to the registry.
   */
  class WriteKey
  {
    friend class MaterialPropertyStorage;
    WriteKey() {}
    WriteKey(const WriteKey &) {}
  };

  /**
   * @return The property ID for the given name, adding the property and
   * creating a new ID if it hasn't already been created.
   *
   * Protected with the MaterialPropertyRegistry::WriteKey.
   */
  unsigned int addOrGetID(const std::string & name, const WriteKey);

  /**
   * @return The property ID for the property with the name \p name
   *
   * Will not create an ID if one does not exist, unlike addOrGetPropertyId
   */
  unsigned int getID(const std::string & name) const;

  /**
   * @return The property name for the property with the ID \p id
   */
  const std::string & getName(const unsigned int id) const;

  /**
   * @return The mapping of material property name to material property ID
   */
  const std::unordered_map<std::string, unsigned int> & getNamesToIDs() const
  {
    return _name_to_id;
  }

private:
  /// Map of material property name -> material property id
  std::unordered_map<std::string, unsigned int> _name_to_id;
  /// Map of material property id -> material property name
  std::vector<std::string> _id_to_name;
};
