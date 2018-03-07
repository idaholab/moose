//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTEBEAMEIGENSTRAINFROMAUXVAR_H
#define COMPUTEBEAMEIGENSTRAINFROMAUXVAR_H

#include "ComputeBeamEigenstrainBase.h"

class ComputeBeamEigenstrainFromAuxVar;

template <>
InputParameters validParams<ComputeBeamEigenstrainFromAuxVar>();

/**
 * ComputeBeamEigenstrainFromAuxVar computes an Eigenstrain from displacement and rotational
 * eigenstrain aux variables
 */
class ComputeBeamEigenstrainFromAuxVar : public ComputeBeamEigenstrainBase
{
public:
  ComputeBeamEigenstrainFromAuxVar(const InputParameters & parameters);

protected:
  virtual void computeQpEigenstrain() override;

  /// Number of displacement eigenstrain variables
  const unsigned int _ndisp;

  /// Number of rotational eigenstrain variables
  const unsigned int _nrot;

  /// Displacemenet eigenstrain variable values
  std::vector<const VariableValue *> _disp;

  /// Rotational eigenstrain variable values
  std::vector<const VariableValue *> _rot;
};

#endif // COMPUTEBEAMEIGENSTRAINFROMAUXVAR_H
