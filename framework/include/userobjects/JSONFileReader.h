//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"

/**
 * User object that reads a JSON file and makes its data available to other objects
 */
class JSONFileReader : public GeneralUserObject
{
public:
  static InputParameters validParams();

  JSONFileReader(const InputParameters & parameters);

  /// Required implementation of a pure virtual function (not used)
  virtual void initialize() override {}

  /// Required implementation of a pure virtual function (not used)
  virtual void finalize() override {}

  /// Read the file again
  virtual void execute() override { read(_filename); }

  /// Getters for scalar values
  /**
   * Get a scalar that is directly indexed at the root of the JSON file/tree
   * @param scalar_name the name of the desired scalar
   * @param scalar reference to the scalar that will be set with the scalar in the JSON
   */
  void getScalar(const std::string & scalar_name, Real & scalar) const;

  /**
   * Get a scalar in the JSON file/tree using the keys in the 'scalar_keys' one by one to
   * traverse the JSON tree down to the requested scalar
   * @param scalar_keys the keys in descending order to access the scalar
   * @param scalar reference to the scalar that will be set with the scalar in the JSON
   */
  void getScalar(const std::vector<std::string> & scalar_keys, Real & scalar) const;

  /// Getter for vector values
  /**
   * Get a vector that is directly indexed at the root of the JSON file/tree
   * @param vector_name the name of the desired vector
   * @param scalar reference to the vector that will be set with the vector in the JSON
   */
  void getVector(const std::string & vector_name, std::vector<Real> & vector) const;
  /**
   * Get a vector in the JSON file/tree using the keys in the 'vector_keys' vector one by one
   * to traverse the JSON tree down to the requested vector
   * @param vector_keys the keys in descending order to access the vector
   * @param vector reference to the vector that will be set with the vector in the JSON
   */
  void getVector(const std::vector<std::string> & vector_keys, std::vector<Real> & vector) const;

private:
  /**
   * Read the JSON file and load it into _root
   * @param filename the name of the file
   */
  void read(const FileName & filename);

  /**
   * Get a real number from a node and handle potential type conversions needed
   * @param node a node from the JSON tree
   * @return the real number desired
   */
  Real getReal(const nlohmann::json & node) const;

  /// Database filename
  const FileName & _filename;
  /// JSON data
  nlohmann::json _root;
};
