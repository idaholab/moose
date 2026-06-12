//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "MFEMObject.h"
#include "MFEMBlockRestrictable.h"
#include "MFEMBoundaryRestrictable.h"
#include "CoefficientManager.h"

class MFEMFunctorMaterial : public MFEMObject,
                            public MFEMBlockRestrictable,
                            public MFEMBoundaryRestrictable
{
public:
  static InputParameters validParams();

  MFEMFunctorMaterial(const InputParameters & parameters);
  virtual ~MFEMFunctorMaterial();

protected:
  /// Group numerical literal values, which should each be enclosed in curly braces, into
  /// single entries of the output vector. Within a literal, entries are separated by
  /// whitespace; matrix literals may additionally separate rows with ';'.
  template <typename T>
  static std::vector<T> processLiterals(const std::vector<T> & input,
                                        const std::string & object_type);

  Moose::MFEM::CoefficientManager & _properties;
};

template <typename T>
std::vector<T>
MFEMFunctorMaterial::processLiterals(const std::vector<T> & input, const std::string & object_type)
{
  std::vector<T> result;
  bool in_literal = false;
  T literal;
  for (const auto & item : input)
  {
    if (in_literal)
    {
      if (item.front() == '{')
        ::mooseError("Nested numeric values are not permitted in ", object_type, " prop_values.");
      else if (item.back() == '}')
      {
        in_literal = false;
        literal += " " + item.substr(0, item.size() - 1);
        result.push_back(literal);
      }
      else
        literal += " " + item;
    }
    else if (item.front() == '{')
    {
      if (item.back() == '}')
        result.push_back(item.substr(1, item.size() - 2));
      else
      {
        in_literal = true;
        literal = item.substr(1);
      }
    }
    else
      result.push_back(item);
  }
  if (in_literal)
    ::mooseError(
        "No closing curly brace for value in ", object_type, " prop_values: '{", literal, "'");
  return result;
}

#endif
