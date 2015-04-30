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

#include <string>
#include <vector>

class DerivativeMaterialPropertyNameInterface
{
public:
  /**
   * Helper functions to generate the material property names for the
   * arbitrary derivatives.
   */
  const std::string propertyName(const std::string &base, const std::vector<std::string> &c) const;

  /**
   * Helper functions to generate the material property names for the
   * first derivatives.
   */
  const std::string propertyNameFirst(const std::string &base,
    const std::string &c1) const;

  /**
   * Helper functions to generate the material property names for the
   * second derivatives.
   */
  const std::string propertyNameSecond(const std::string &base,
    const std::string &c1, const std::string &c2) const;

  /**
   * Helper functions to generate the material property names for the
   * third derivatives.
   */
  const std::string propertyNameThird(const std::string &base,
    const std::string &c1, const std::string &c2, const std::string &c3) const;
};

#endif // DERIVATIVEMATERIALPROPERTYNAMEINTERFACE_H
