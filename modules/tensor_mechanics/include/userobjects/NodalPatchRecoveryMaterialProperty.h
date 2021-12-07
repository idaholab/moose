//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "NodalPatchRecoveryBase.h"

#include "IndexableProperty.h"

/**
 * Prepare patches for use in nodal patch recovery based on a material property.
 */
class NodalPatchRecoveryMaterialProperty : public NodalPatchRecoveryBase
{
public:
  static InputParameters validParams();

  NodalPatchRecoveryMaterialProperty(const InputParameters & parameters);

  virtual void initialSetup() override { _prop.check(); }

protected:
  virtual Real computeValue() override { return _prop[_qp]; }

  const IndexableProperty<NodalPatchRecoveryBase, false> _prop;
};
