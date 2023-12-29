//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"

// Forward Declarations

class AsymptoticExpansionHomogenizationKernel : public Kernel
{
public:
  static InputParameters validParams();

  AsymptoticExpansionHomogenizationKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  /// Base name of the material system that this kernel applies to
  const std::string _base_name;

  const MaterialProperty<RankFourTensor> & _elasticity_tensor;

private:
  /// An integer corresponding to the direction this kernel acts in
  const unsigned int _component;
  const unsigned int _column;
  const std::array<unsigned int, 6> _k_index;
  const std::array<unsigned int, 6> _l_index;
  const unsigned int _k;
  const unsigned int _l;
};
