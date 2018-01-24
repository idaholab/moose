//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef DERIVATIVEMATERIALPROPERTYNAMEINTERFACE_H
#define DERIVATIVEMATERIALPROPERTYNAMEINTERFACE_H

#include "MooseTypes.h"

class DerivativeMaterialPropertyNameInterface
{
public:
  /**
   * Helper functions to generate the material property names for the
   * arbitrary derivatives.
   */
  const MaterialPropertyName propertyName(const MaterialPropertyName & base,
                                          const std::vector<VariableName> & c) const;

  /**
   * Helper functions to generate the material property names for the
   * first derivatives.
   */
  const MaterialPropertyName propertyNameFirst(const MaterialPropertyName & base,
                                               const VariableName & c1) const;

  /**
   * Helper functions to generate the material property names for the
   * second derivatives.
   */
  const MaterialPropertyName propertyNameSecond(const MaterialPropertyName & base,
                                                const VariableName & c1,
                                                const VariableName & c2) const;

  /**
   * Helper functions to generate the material property names for the
   * third derivatives.
   */
  const MaterialPropertyName propertyNameThird(const MaterialPropertyName & base,
                                               const VariableName & c1,
                                               const VariableName & c2,
                                               const VariableName & c3) const;
};

#endif // DERIVATIVEMATERIALPROPERTYNAMEINTERFACE_H
