//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseVariableBase.h"
#include "Coupleable.h"

#define usingNeighborCoupleableMembers usingCoupleableMembers

/**
 * Enhances Coupleable interface to also couple the values from neighbor elements
 *
 */
class NeighborCoupleable : public Coupleable
{
public:
  /**
   * Constructing the object
   * @param parameters Parameters that come from constructing the object
   * @param nodal true if we need to couple with nodal values, otherwise false
   * @param is_fv Whether the \p MooseObject is a finite volume object
   */
  NeighborCoupleable(const MooseObject * moose_object,
                     bool nodal,
                     bool neighbor_nodal,
                     bool is_fv = false);

  // neighbor
  virtual const VariableValue & coupledNeighborValue(const std::string & var_name,
                                                     unsigned int comp = 0) const;

  std::vector<const VariableValue *> coupledNeighborValues(const std::string & var_name) const;

  /**
   * Get the coupled neighbor variable value for \p var_name with derivative information for
   * automatic differentiation objects
   */
  virtual const ADVariableValue & adCoupledNeighborValue(const std::string & var_name,
                                                         unsigned int comp = 0) const;

  /**
   * Retrieve the coupled neighbor variable value whether AD or not
   */
  template <bool is_ad>
  const auto & coupledGenericNeighborValue(const std::string & var_name,
                                           unsigned int comp = 0) const;

  /**
   * Retrieve the coupled neighbor variable gradient whether AD or not
   */
  template <bool is_ad>
  const auto & coupledGenericNeighborGradient(const std::string & var_name,
                                              unsigned int comp = 0) const;

  /**
   * Get the time derivative of the coupled neighbor variable value for \p var_name with derivative
   * information for automatic differentiation objects
   */
  virtual const ADVariableValue & adCoupledNeighborValueDot(const std::string & var_name,
                                                            unsigned int comp = 0) const;

  std::vector<const ADVariableValue *> adCoupledNeighborValues(const std::string & var_name) const;

  /**
   * Get the coupled neighbor vector variable value for \p var_name with derivative information for
   * automatic differentiation objects
   */
  virtual const ADVectorVariableValue & adCoupledVectorNeighborValue(const std::string & var_name,
                                                                     unsigned int comp = 0) const;
  virtual const VariableValue & coupledNeighborValueDot(const std::string & var_name,
                                                        unsigned int comp = 0) const;
  virtual const VariableValue & coupledNeighborValueDotDu(const std::string & var_name,
                                                          unsigned int comp = 0) const;
  virtual const VariableValue & coupledNeighborValueOld(const std::string & var_name,
                                                        unsigned int comp = 0) const;
  virtual const VariableValue & coupledNeighborValueOlder(const std::string & var_name,
                                                          unsigned int comp = 0) const;

  virtual const VariableGradient & coupledNeighborGradient(const std::string & var_name,
                                                           unsigned int comp = 0) const;
  virtual std::vector<const VariableGradient *>
  coupledNeighborGradients(const std::string & var_name) const;
  virtual const VariableGradient & coupledNeighborGradientOld(const std::string & var_name,
                                                              unsigned int comp = 0) const;
  virtual const VariableGradient & coupledNeighborGradientOlder(const std::string & var_name,
                                                                unsigned int comp = 0) const;

  /**
   * Get the coupled neighbor variable gradient for \p var_name with derivative information for
   * automatic differentiation objects
   */
  virtual const ADVariableGradient & adCoupledNeighborGradient(const std::string & var_name,
                                                               unsigned int comp = 0) const;

  virtual const VectorVariableGradient & coupledVectorNeighborGradient(const std::string & var_name,
                                                                       unsigned int comp = 0) const;
  virtual const VectorVariableGradient &
  coupledVectorNeighborGradientOld(const std::string & var_name, unsigned int comp = 0) const;
  virtual const VectorVariableGradient &
  coupledVectorNeighborGradientOlder(const std::string & var_name, unsigned int comp = 0) const;

  virtual const ArrayVariableValue & coupledArrayNeighborValue(const std::string & var_name,
                                                               unsigned int comp = 0) const;
  virtual const ArrayVariableGradient & coupledArrayNeighborGradient(const std::string & var_name,
                                                                     unsigned int comp = 0) const;
  virtual const ArrayVariableGradient &
  coupledArrayNeighborGradientOld(const std::string & var_name, unsigned int comp = 0) const;
  virtual const ArrayVariableGradient &
  coupledArrayNeighborGradientOlder(const std::string & var_name, unsigned int comp = 0) const;

  virtual const VariableSecond & coupledNeighborSecond(const std::string & var_name,
                                                       unsigned int i = 0) const;

  virtual const VariableValue & coupledNeighborDofValues(const std::string & var_name,
                                                         unsigned int comp = 0) const;
  virtual const VariableValue & coupledNeighborDofValuesOld(const std::string & var_name,
                                                            unsigned int comp = 0) const;
  virtual const VariableValue & coupledNeighborDofValuesOlder(const std::string & var_name,
                                                              unsigned int comp = 0) const;

protected:
  bool _neighbor_nodal;
};

template <bool is_ad>
const auto &
NeighborCoupleable::coupledGenericNeighborValue(const std::string & var_name,
                                                const unsigned int comp) const
{
  if constexpr (is_ad)
    return adCoupledNeighborValue(var_name, comp);
  else
    return coupledNeighborValue(var_name, comp);
}

template <bool is_ad>
const auto &
NeighborCoupleable::coupledGenericNeighborGradient(const std::string & var_name,
                                                   const unsigned int comp) const
{
  if constexpr (is_ad)
    return adCoupledNeighborGradient(var_name, comp);
  else
    return coupledNeighborGradient(var_name, comp);
}
