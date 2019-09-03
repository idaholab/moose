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
   */
  NeighborCoupleable(const MooseObject * moose_object, bool nodal, bool neighbor_nodal);

  // neighbor
  virtual const VariableValue & coupledNeighborValue(const std::string & var_name,
                                                     unsigned int comp = 0);
  virtual const VariableValue & coupledNeighborValueDot(const std::string & var_name,
                                                        unsigned int comp = 0);
  virtual const VariableValue & coupledNeighborValueDotResidual(const std::string & var_name,
                                                                unsigned int comp = 0);
  virtual const VariableValue & coupledNeighborValueDotDu(const std::string & var_name,
                                                          unsigned int comp = 0);
  virtual const VariableValue & coupledNeighborValueOld(const std::string & var_name,
                                                        unsigned int comp = 0);
  virtual const VariableValue & coupledNeighborValueOlder(const std::string & var_name,
                                                          unsigned int comp = 0);

  virtual const VariableGradient & coupledNeighborGradient(const std::string & var_name,
                                                           unsigned int comp = 0);
  virtual const VariableGradient & coupledNeighborGradientOld(const std::string & var_name,
                                                              unsigned int comp = 0);
  virtual const VariableGradient & coupledNeighborGradientOlder(const std::string & var_name,
                                                                unsigned int comp = 0);
  virtual const VectorVariableGradient & coupledVectorNeighborGradient(const std::string & var_name,
                                                                       unsigned int comp = 0);
  virtual const VectorVariableGradient &
  coupledVectorNeighborGradientOld(const std::string & var_name, unsigned int comp = 0);
  virtual const VectorVariableGradient &
  coupledVectorNeighborGradientOlder(const std::string & var_name, unsigned int comp = 0);

  virtual const ArrayVariableValue & coupledArrayNeighborValue(const std::string & var_name,
                                                               unsigned int comp = 0);
  virtual const ArrayVariableGradient & coupledArrayNeighborGradient(const std::string & var_name,
                                                                     unsigned int comp = 0);
  virtual const ArrayVariableGradient &
  coupledArrayNeighborGradientOld(const std::string & var_name, unsigned int comp = 0);
  virtual const ArrayVariableGradient &
  coupledArrayNeighborGradientOlder(const std::string & var_name, unsigned int comp = 0);

  virtual const VariableSecond & coupledNeighborSecond(const std::string & var_name,
                                                       unsigned int i = 0);

  virtual const VariableValue & coupledNeighborDofValues(const std::string & var_name,
                                                         unsigned int comp = 0);
  virtual const VariableValue & coupledNeighborDofValuesOld(const std::string & var_name,
                                                            unsigned int comp = 0);
  virtual const VariableValue & coupledNeighborDofValuesOlder(const std::string & var_name,
                                                              unsigned int comp = 0);

protected:
  bool _neighbor_nodal;
};
