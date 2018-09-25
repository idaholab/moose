//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ALGEBRAICRMTESTER_H
#define ALGEBRAICRMTESTER_H

#include "GeneralUserObject.h"

// Forward Declarations
class AlgebraicRMTester;

template <>
InputParameters validParams<AlgebraicRMTester>();

/**
 * User object to calculate ghosted elements on a single processor or the union across all
 * processors.
 */
class AlgebraicRMTester : public GeneralUserObject
{
public:
  AlgebraicRMTester(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

  unsigned long getElementalValue(dof_id_type element_id) const;

protected:
  std::set<dof_id_type> _ghost_data;
  const dof_id_type _rank;
  const bool _show_evaluable;
};

#endif // ALGEBRAICRMTESTER_H
