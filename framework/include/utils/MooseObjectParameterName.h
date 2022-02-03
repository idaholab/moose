//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "MooseObjectName.h"

// STL includes
#include <string>

/**
 * A class for storing an input parameter name.
 *
 * This class is used by the Control logic system, allowing for multiple tags
 * to be applied to many different MooseObject parameters.
 *
 * This class simply adds a third field (parameter name) to the MooseObjectName class.
 */
class MooseObjectParameterName : public MooseObjectName
{
public:
  /**
   * Build an object given a raw parameter name (e.g., from an input file parameter)
   */
  MooseObjectParameterName(std::string name);

  /**
   * Build an object given a MooseObjectName and parameter name
   */
  MooseObjectParameterName(const MooseObjectName & obj_name, const std::string & param);

  /**
   * Build an object given a tag, object name, and parameter name
   */
  MooseObjectParameterName(const std::string & tag,
                           const std::string & name,
                           const std::string & param,
                           const std::string & separator = std::string("/"));

  MooseObjectParameterName(const MooseObjectParameterName & rhs);

  /**
   * Return the parameter name.
   */
  const std::string & parameter() const { return _parameter; }

  /**
   * Adds the parameter name to error checking.
   */
  virtual void check() final;

  ///@{
  /**
   * Comparison operators.
   *
   * Not that this class may be compared with MooseObjectName, this
   * feature is used by ControlInterface.
   *
   * The less than operator is required so this container can work in std::map.
   *
   * @see InputParameterWarehouse
   */
  bool operator==(const MooseObjectParameterName & rhs) const;
  bool operator==(const MooseObjectName & rhs) const;

  bool operator!=(const MooseObjectParameterName & rhs) const;
  bool operator!=(const MooseObjectName & rhs) const;

  bool operator<(const MooseObjectParameterName & rhs) const;
  ///@}

  // Allow printing with std:: cout
  friend std::ostream & operator<<(std::ostream & stream, const MooseObjectParameterName & obj);

protected:
  /// The name of the input parameter
  std::string _parameter;
};
