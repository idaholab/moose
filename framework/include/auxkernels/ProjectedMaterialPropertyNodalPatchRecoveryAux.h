//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalPatchRecoveryAuxBase.h"

/// Forward declare user object
class ProjectedStatefulMaterialNodalPatchRecoveryBase;

class ProjectedMaterialPropertyNodalPatchRecoveryAux : public NodalPatchRecoveryAuxBase
{
public:
  static InputParameters validParams();

  ProjectedMaterialPropertyNodalPatchRecoveryAux(const InputParameters & parameters);

protected:
  virtual Real nodalPatchRecovery() override;

private:
  /// User object holding the data needed for patch recovery
  const ProjectedStatefulMaterialNodalPatchRecoveryBase & _npr;

  /// Property component (index into a serialized representation of the property)
  const unsigned int _component;
};
