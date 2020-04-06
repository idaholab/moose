//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"
#include "MooseEnum.h"

/**
 * Action class to setup peridynamic generalized plane strain models
 */
class GeneralizedPlaneStrainActionPD : public Action
{
public:
  static InputParameters validParams();

  GeneralizedPlaneStrainActionPD(const InputParameters & params);

  virtual void act() override;

protected:
  ///@{ Displacement variables
  std::vector<VariableName> _displacements;
  const unsigned int _ndisp;
  ///@}

  /// Option to choose which peridynamic model to use for generalized plane strain formulation: ordinary state based or non-ordinary state based
  const MooseEnum _formulation;

  /// Scalar variable for out-of-plane strain variable
  VariableName _scalar_out_of_plane_strain;
};
