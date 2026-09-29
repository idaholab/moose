//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"

class MaterialWarehouse;

/**
 * Interface class to return the size of material properties that do not have a fixed size
 */
class VariableSizeMaterialPropertiesInterface
{
public:
  VariableSizeMaterialPropertiesInterface(const InputParameters & /*params*/) {}

  /// Return the size of the variable size vector material property that the material defines.
  /// Materials that declare such a property must override this; the default errors.
  virtual std::size_t getVectorPropertySize(const MaterialPropertyName & prop_name) const;

  /// Return the number of rows and columns of the variable size matrix material property that the
  /// material defines. Materials that declare such a property must override this; the default
  /// errors.
  virtual std::pair<std::size_t, std::size_t>
  getMatrixPropertySize(const MaterialPropertyName & prop_name) const;
};

namespace Moose
{
/**
 * Check during setup that an array material property used as a coefficient has one entry per
 * component of an array variable. Only properties declared by materials implementing
 * VariableSizeMaterialPropertiesInterface can be checked; others are skipped.
 * @param object The object using the coefficient, which reports the error
 * @param warehouse The warehouse holding the materials that may declare the property
 * @param blocks The subdomains on which the object uses the coefficient
 * @param param_name The parameter of the object holding the material property name
 * @param count The number of components of the array variable
 * @param is_matrix Whether the coefficient is a count x count matrix instead of a vector
 */
void checkArrayMaterialPropertySize(const MooseObject & object,
                                    const MaterialWarehouse & warehouse,
                                    const std::set<SubdomainID> & blocks,
                                    const std::string & param_name,
                                    std::size_t count,
                                    bool is_matrix);
}
