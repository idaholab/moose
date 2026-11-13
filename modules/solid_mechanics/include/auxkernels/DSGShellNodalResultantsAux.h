//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

class DSGShellNodalResultants;

/**
 * DSGShellNodalResultantsAux writes one stress resultant of the DSG shell elements,
 * averaged at the nodes by a DSGShellNodalResultants user object, to a nodal
 * auxiliary variable.
 */
class DSGShellNodalResultantsAux : public AuxKernel
{
public:
  static InputParameters validParams();

  DSGShellNodalResultantsAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// Index of the resultant in the resultants vector, from the chosen name
  const unsigned int _resultant_index;

  /// The user object holding the nodal averages
  const DSGShellNodalResultants & _nodal_resultants;
};
