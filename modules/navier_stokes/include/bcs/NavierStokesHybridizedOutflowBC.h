//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HybridizedIntegratedBC.h"
#include "NavierStokesHybridizedInterface.h"

#include <vector>

template <typename>
class MooseVariableFE;
class MooseVariableScalar;
template <typename>
class MooseArray;
class Function;

class NavierStokesHybridizedOutflowBC : public HybridizedIntegratedBC,
                                        public NavierStokesHybridizedInterface
{
public:
  static InputParameters validParams();

  NavierStokesHybridizedOutflowBC(const InputParameters & parameters);

protected:
  virtual void assemble() override;

  /// transformed Jacobian weights on the face
  const MooseArray<Real> & _JxW_face;

  /// The face quadrature rule
  const QBase * const & _qrule_face;

  friend class NavierStokesHybridizedKernel;
  friend class NavierStokesHybridizedInterface;
};
