//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MaterialAuxBase.h"
#include "RankFourTensor.h"

/**
 * MaterialRankFourTensorAux is designed to take the data in the RankFourTensor material
 * property, for example stiffness, and output the value for the
 * supplied indices.
 */

class MaterialRankFourTensorAux : public MaterialAuxBase<RankFourTensor>
{
public:
  static InputParameters validParams();

  MaterialRankFourTensorAux(const InputParameters & parameters);

protected:
  virtual Real getRealValue() override;

  ///@{ tensor indices
  const unsigned int _i;
  const unsigned int _j;
  const unsigned int _k;
  const unsigned int _l;
  ///@}
};
