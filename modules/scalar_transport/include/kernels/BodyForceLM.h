/************************************************************/
/*                DO NOT MODIFY THIS HEADER                 */
/*   TMAP8: Tritium Migration Analysis Program, Version 8   */
/*                                                          */
/*   Copyright 2021 - 2022 Battelle Energy Alliance, LLC    */
/*                   ALL RIGHTS RESERVED                    */
/************************************************************/

#pragma once

#include "LMKernel.h"

class BodyForceLM : public LMKernel
{
public:
  BodyForceLM(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  ADReal precomputeQpResidual() override;

  /// Scale factor
  const Real & _scale;

  /// Optional function value
  const Function & _function;

  /// Optional Postprocessor value
  const PostprocessorValue & _postprocessor;
};
