//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose includes
#include "Positions.h"
#include "FunctorInterface.h"
#include "BlockRestrictable.h"
#include "FunctionParserUtils.h"

/**
 * Positions of the centroids of all elements meeting the parsed expression criterion
 */
class ParsedDownSelectionPositions : public Positions,
                                     public NonADFunctorInterface,
                                     public BlockRestrictable,
                                     public FunctionParserUtils<false>
{
public:
  static InputParameters validParams();
  ParsedDownSelectionPositions(const InputParameters & parameters);
  virtual ~ParsedDownSelectionPositions() = default;

  void initialize() override;

private:
  /// function expression
  std::string _expression;

  /// coordinate and time variable names
  const std::vector<std::string> _xyzt;

  /// function parser object for the local value of the expression
  SymFunctionPtr _func_F;

  usingFunctionParserUtilsMembers(false);

  /// Functors to use in the parsed expression
  const std::vector<MooseFunctorName> & _functor_names;

  /// Number of functors
  const unsigned int _n_functors;

  /// Symbolic name to use for each functor
  const std::vector<std::string> _functor_symbols;

  /// Vector of pointers to functors
  std::vector<const Moose::Functor<Real> *> _functors;

  /// Vector of pointers to positions objects
  std::vector<const Positions *> _positions_ptrs;

  enum PositionSelection : short
  {
    Error = 0,
    Discard = 1,
    Keep = 2
  };
};
