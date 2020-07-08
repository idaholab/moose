/*************************************************/
/*           DO NOT MODIFY THIS HEADER           */
/*                                               */
/*                     BISON                     */
/*                                               */
/*    (c) 2015 Battelle Energy Alliance, LLC     */
/*            ALL RIGHTS RESERVED                */
/*                                               */
/*   Prepared by Battelle Energy Alliance, LLC   */
/*     Under Contract No. DE-AC07-05ID14517      */
/*     With the U. S. Department of Energy       */
/*                                               */
/*     See COPYRIGHT for full restrictions       */
/*************************************************/

#pragma once

#include "Kernel.h"
#include "ADKernel.h"

template <bool is_ad>
class GenericKernel : public Kernel
{
public:
  static InputParameters validParams()
  {
    InputParameters params = Kernel::validParams();
    return params;
  };
  GenericKernel(const InputParameters & parameters) : Kernel(parameters){};
};
template <>
class GenericKernel<true> : public ADKernel
{
public:
  static InputParameters validParams()
  {
    InputParameters params = ADKernel::validParams();
    return params;
  };
  GenericKernel(const InputParameters & parameters) : ADKernel(parameters){};
};
