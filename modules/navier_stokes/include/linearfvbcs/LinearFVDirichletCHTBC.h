//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVCHTBCInterface.h"
#include "LinearFVAdvectionDiffusionFunctorDirichletBC.h"

/**
 * Conjugate heat transfer BC for Dirichlet boundary condition-based
 * coupling. Differs from regular BCs due to the need of error checking.
 */
class LinearFVDirichletCHTBC : public LinearFVAdvectionDiffusionFunctorDirichletBC,
                               public LinearFVCHTBCInterface
{
public:
  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   */
  LinearFVDirichletCHTBC(const InputParameters & parameters);

  static InputParameters validParams();
};
