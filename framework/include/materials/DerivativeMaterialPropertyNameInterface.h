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
