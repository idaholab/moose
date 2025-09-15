//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "gtest/gtest.h"
#include "MooseTypes.h"

// Forward declarations
namespace moose
{
namespace automatic_weak_form
{
class StringExpressionParser;
class WeakFormGenerator;
class ExpressionEvaluator;
class ExpressionSimplifier;
}
}

/**
 * Test fixture for weak form derivation system tests
 */
class WeakFormDerivationTest : public ::testing::Test
{
protected:
  void SetUp() override;

  std::unique_ptr<moose::automatic_weak_form::StringExpressionParser> parser;
  std::unique_ptr<moose::automatic_weak_form::WeakFormGenerator> generator;
  std::unique_ptr<moose::automatic_weak_form::ExpressionEvaluator> evaluator;
  std::unique_ptr<moose::automatic_weak_form::ExpressionSimplifier> simplifier;
};