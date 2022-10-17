/************************************************************/
/*                DO NOT MODIFY THIS HEADER                 */
/*   TMAP8: Tritium Migration Analysis Program, Version 8   */
/*                                                          */
/*   Copyright 2021 - 2022 Battelle Energy Alliance, LLC    */
/*                   ALL RIGHTS RESERVED                    */
/************************************************************/

#pragma once

#include "ADKernelValue.h"

class LMKernel : public ADKernelValue
{
public:
  LMKernel(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  void computeResidual() override;
  void computeResidualsForJacobian() override;

  MooseVariable & _lm_var;
  const ADVariableValue & _lm;
  const VariableTestValue & _lm_test;
  const Real _lm_sign;

private:
  const std::vector<dof_id_type> & dofIndices() const override { return _all_dof_indices; }

  /// The union of the primary var dof indices as well as the LM dof indices
  std::vector<dof_id_type> _all_dof_indices;
};
