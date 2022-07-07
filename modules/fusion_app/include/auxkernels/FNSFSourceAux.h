//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

class FNSFSourceAux : public AuxKernel
{
public:
  static InputParameters validParams();

  FNSFSourceAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  std::pair<int, int>
  index_xi_depth(Real xi,
                 Real depth,
                 const std::vector<Real> & inner_xi_grid,
                 const std::vector<Real> & outer_xi_grid,
                 const std::vector<Real> & depth_grid);

  std::vector<Real> _inner_xi_grid;
  std::vector<Real> _outer_xi_grid;
  std::vector<Real> _depth_grid;
  std::vector<Real> _source;
};
