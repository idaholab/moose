/************************************************************/
/*                DO NOT MODIFY THIS HEADER                 */
/*   TMAP8: Tritium Migration Analysis Program, Version 8   */
/*                                                          */
/*   Copyright 2021 - 2022 Battelle Energy Alliance, LLC    */
/*                   ALL RIGHTS RESERVED                    */
/************************************************************/

#pragma once

#include "LMKernel.h"

class LMTimeKernel : public LMKernel
{
public:
  LMTimeKernel(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  const ADVariableValue & _u_dot;
};
