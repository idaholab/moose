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
#include <memory>

#include "MooseObject.h"
#include "MooseMesh.h"

#include "libmesh/elem.h"

class InputParameters;
class MooseObject;

class ElementIDInterface
{
public:
  ElementIDInterface(const MooseObject * moose_object);
  static InputParameters validParams();

  virtual ~ElementIDInterface() {}

  /**
   * Gets index of an element integer with a parameter of the object derived from this interface
   * @param id_parameter_name Name of object parameter
   * @param comp Component number for vector of integer names
   * @return Index of the element integer
   */
  virtual unsigned int getElementIDIndex(const std::string & id_parameter_name,
                                         unsigned int comp = 0) const;

  /**
   * Return the accessing integer for an extra element integer with its name
   * @param id_name Name of element integer
   * @return Index of the element integer
   */
  virtual unsigned int getElementIDIndexByName(const std::string & id_name) const;

  /**
   * Gets an element integer with a parameter of the object derived from this interface
   * @param id_parameter_name Name of object parameter
   * @param comp Component number for vector of integer names
   * @return Integer for the current element
   */
  virtual const dof_id_type & getElementID(const std::string & id_parameter_name,
                                           unsigned int comp = 0) const;

  /**
   * Gets a neighbor element integer with a parameter of the object derived from this interface
   * @param id_parameter_name Name of object parameter
   * @param comp Component number for vector of integer names
   * @return Integer for the neighbor element
   */
  virtual const dof_id_type & getElementIDNeighbor(const std::string & id_parameter_name,
                                                   unsigned int comp = 0) const;

  /**
   * Gets an element integer with the element integer name
   * @param id_name Name of element integer
   * @return Integer for the current element
   */
  virtual const dof_id_type & getElementIDByName(const std::string & id_name) const;

  /**
   * Gets a neighbor element integer with the element integer name
   * @param id_name Name of element integer
   * @return Integer for the neighbor element
   */
  virtual const dof_id_type & getElementIDNeighborByName(const std::string & id_name) const;

  /**
   * Whether mesh has an element integer with a given name
   */
  bool hasElementID(const std::string & id_name) const { return _id_mesh->hasElementID(id_name); }

  /**
   * Return the maximum element ID for an element integer with its index
   */
  dof_id_type maxElementID(unsigned int elem_id_index) const
  {
    return _id_mesh->maxElementID(elem_id_index);
  }

  /**
   * Return the minimum element ID for an element integer with its index
   */
  dof_id_type minElementID(unsigned int elem_id_index) const
  {
    return _id_mesh->minElementID(elem_id_index);
  }

  /**
   * Whether two element integers are identical for all elements
   */
  bool areElemIDsIdentical(const std::string & id_name1, const std::string & id_name2) const
  {
    return _id_mesh->areElemIDsIdentical(id_name1, id_name2);
  }

  /**
   * Return all the unique element IDs for an element integer with its index on the entire domain
   */
  std::set<dof_id_type> getAllElemIDs(unsigned int elem_id_index) const
  {
    return _id_mesh->getAllElemIDs(elem_id_index);
  }

  /**
   * Return all the unique element IDs for an extra element integer with its index on a set of
   * subdomains
   */
  std::set<dof_id_type> getElemIDsOnBlocks(unsigned int elem_id_index,
                                           const std::set<SubdomainID> & blks) const
  {
    return _id_mesh->getElemIDsOnBlocks(elem_id_index, blks);
  }

  /**
   * Get an element integer for an element
   */
  dof_id_type getElementID(const Elem * elem, unsigned int elem_id_index) const
  {
    if (elem_id_index == elem->n_extra_integers())
      return elem->subdomain_id();

    return elem->get_extra_integer(elem_id_index);
  }

private:
  /// Reference to the object's input parameters
  const InputParameters & _obj_parameters;

  /// References to the mesh and displaced mesh (currently in the ActionWarehouse)
  std::shared_ptr<MooseMesh> & _id_mesh;

  /// Name of the object using this interface
  const std::string & _ei_name;
};
