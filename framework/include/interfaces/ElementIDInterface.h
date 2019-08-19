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

class InputParameters;
class MooseObject;
class MooseMesh;

class ElementIDInterface
{
public:
  ElementIDInterface(const MooseObject * moose_object);
  virtual ~ElementIDInterface() {}

  /**
   * Gets index of an element integer with a parameter of the object derived from this interface
   * @param id_name Name of object parameter
   * @param comp Component number for vector of integer names
   * @return Index of the element integer
   */
  virtual unsigned int getElementIDIndex(const std::string & id_parameter_name,
                                         unsigned int comp = 0) const;

  /**
   * Gets an element integer with a parameter of the object derived from this interface
   * @param id_name Name of object parameter
   * @param comp Component number for vector of integer names
   * @return Integer for the current element
   */
  virtual const dof_id_type & getElementID(const std::string & id_parameter_name,
                                           unsigned int comp = 0) const;

private:
  /// Reference to the object's input parameters
  const InputParameters & _obj_parameters;

  /// References to the mesh and displaced mesh (currently in the ActionWarehouse)
  std::shared_ptr<MooseMesh> & _mesh;
};
