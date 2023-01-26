/********************************************************************/
/*                   DO NOT MODIFY THIS HEADER                      */
/*          Subchannel: Thermal Hydraulics Reactor Analysis         */
/*                                                                  */
/*              (c) 2022 Battelle Energy Alliance, LLC              */
/*                      ALL RIGHTS RESERVED                         */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*               With the U. S. Department of Energy                */
/*                                                                  */
/*               See COPYRIGHT for full restrictions                */
/********************************************************************/

#pragma once

#include "AuxKernel.h"

/**
 * Constant auxiliary value
 */
class PostprocessorConstantAux : public AuxKernel
{
public:
  static InputParameters validParams();

  PostprocessorConstantAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// Value provided by postprocessor
  const PostprocessorValue & _pvalue;
};
