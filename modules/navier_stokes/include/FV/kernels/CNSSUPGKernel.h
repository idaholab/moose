//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernel.h"

class CNSSUPGKernel;

declareADValidParams(CNSSUPGKernel);

/**
 * Kernel representing the SUPG stabilization term for each conservation
 * equation, including both the time-dependent and spatial portions, with weak form
 * $\int \epsilon \textbf{A}_i\frac{\partial \vec{W}_i}{\partial x_i}\cdot\tau\vec{R}$.
 * The equation to which this stabilization is applied is indicated with
 * the `var_num` parameter, with indexing defined as 0 for the mass equation,
 * 1 to $n_{sd}$ for the momentum equations, and $n_{sd}+2$ for the energy
 * equation. In this indexin scheme, $n_{sd}$ is the number of spatial dimensions
 * solved in the problem.
 */
class CNSSUPGKernel : public ADKernel
{
public:
  CNSSUPGKernel(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /// dimension of the mesh
  const unsigned int _mesh_dimension;

  /// number of variables in equation system
  const unsigned int _n_vars;

  /// variable number in coupled system of equations
  const unsigned int & _var_num;

  /// porosity
  const VariableValue & _eps;

  /// inviscid flux Jacobian matrices $\textbf{A}_i$
  const ADMaterialProperty<std::vector<DenseMatrix<Real>>> & _A;

  /// matrix $\tau$ stabilization parameter
  const ADMaterialProperty<DenseMatrix<Real>> & _Tau;

  /// quasi-linear strong residual $R$
  const ADMaterialProperty<DenseVector<Real>> & _R;

  /// these are used for temporary calc caching/reusage.  They facilitate
  /// performance improvements.
  std::vector<ADReal> _res_tmp;
  std::array<ADReal, 3> _supg_mult;
  ADReal _tmp;
  ADReal _r;

};
