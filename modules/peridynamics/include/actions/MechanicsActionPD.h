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

class MechanicsActionPD;

template <>
InputParameters validParams<MechanicsActionPD>();

/**
 * Action class to setup peridynamic models for solid mechanics problems
 */
class MechanicsActionPD : public Action
{
public:
  MechanicsActionPD(const InputParameters & params);

  virtual void act() override;

protected:
  /**
   * Function to get the kernel name based on the value of member variables: _formulation and
   * _stabilization
   * @return Kernel name
   */
  virtual std::string getKernelName();

  /**
   * Function to get the input parameters for a given kernel name
   * @param name   the name of the kernel
   * @return Parameters for the corresponding kernel
   */
  virtual InputParameters getKernelParameters(std::string name);

  ///@{ Displacement variables
  std::vector<VariableName> _displacements;
  const unsigned int _ndisp;
  ///@}

  /// Option of which peridynamic model needs to be setup: bond based, ordinary state based or non-ordinary state based
  const MooseEnum _formulation;

  /// Option of stabilization scheme for correspondence material model (non-ordinary state based): force or self stabilized
  const MooseEnum _stabilization;

  /// Option to set whether the correspondence material model is finite strain or not
  const bool _finite_strain_formulation;

  ///@{ Residual debugging
  std::vector<AuxVariableName> _save_in;
  std::vector<AuxVariableName> _diag_save_in;
  ///@}
};
