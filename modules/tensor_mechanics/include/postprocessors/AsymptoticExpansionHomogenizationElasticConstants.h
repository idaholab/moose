//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementIntegralPostprocessor.h"

// Forward Declarations

/**
 * This postprocessor computes homogenized elastic constants
 */
class AsymptoticExpansionHomogenizationElasticConstants : public ElementIntegralPostprocessor
{
public:
  static InputParameters validParams();

  AsymptoticExpansionHomogenizationElasticConstants(const InputParameters & parameters);

  virtual void initialize();
  virtual void execute();
  virtual Real getValue();
  virtual void finalize();
  virtual void threadJoin(const UserObject & y);

protected:
  virtual Real computeQpIntegral();

private:
  /// Base name of the material system
  const std::string _base_name;

  const std::array<std::array<const VariableGradient *, 3>, 6> _grad;

  const MaterialProperty<RankFourTensor> & _elasticity_tensor;
  const unsigned int _column, _row;
  const std::array<unsigned int, 6> _ik_index;
  const std::array<unsigned int, 6> _jl_index;
  const unsigned _i, _j, _k, _l;
  Real _volume;
  Real _integral_value;
};
