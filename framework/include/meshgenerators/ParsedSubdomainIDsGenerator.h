//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ParsedSubdomainGeneratorBase.h"

/**
 * MeshGenerator to use a parsed expression to determine the subdomain ids of included elements.
 */
class ParsedSubdomainIDsGenerator : public ParsedSubdomainGeneratorBase
{
public:
  static InputParameters validParams();

  ParsedSubdomainIDsGenerator(const InputParameters & parameters);

protected:
  /// function expression
  const std::string _function;

  /**
   * Assign the subdomain id to the element based on the parsed expression
   * @param elem The element to assign the subdomain id to
   */
  void assignElemSubdomainID(Elem * elem) override;
};
