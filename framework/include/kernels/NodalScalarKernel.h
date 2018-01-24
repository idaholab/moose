//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NODALSCALARKERNEL_H
#define NODALSCALARKERNEL_H

#include "ScalarKernel.h"
#include "Coupleable.h"
#include "MooseVariableDependencyInterface.h"

class NodalScalarKernel;

template <>
InputParameters validParams<NodalScalarKernel>();

/**
 *
 */
class NodalScalarKernel : public ScalarKernel,
                          public Coupleable,
                          public MooseVariableDependencyInterface
{
public:
  NodalScalarKernel(const InputParameters & parameters);

  virtual void reinit() override;
  virtual void computeOffDiagJacobian(unsigned int jvar) override;

protected:
  /// List of node IDs
  std::vector<dof_id_type> _node_ids;
};

#endif /* NODALSCALARKERNEL_H */
