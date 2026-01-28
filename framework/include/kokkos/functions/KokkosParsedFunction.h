//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosFunction.h"
#include "KokkosFunctionParser.h"

#include "MooseParsedFunctionBase.h"

class KokkosParsedFunction : public Moose::Kokkos::FunctionBase, public MooseParsedFunctionBase
{
public:
  static InputParameters validParams();

  KokkosParsedFunction(const InputParameters & parameters);
  KokkosParsedFunction(const KokkosParsedFunction & function);

  using Real3 = Moose::Kokkos::Real3;

  KOKKOS_FUNCTION Real value(Real t, Real3 p) const;

  /**
   * Create the parsed function
   */
  virtual void initialSetup() override;

protected:
  /**
   * Parsed function expression
   */
  const std::string _expression;
  /**
   * Parsed function builder
   */
  std::unique_ptr<Moose::Kokkos::RPNBuilder> _builder;
};
