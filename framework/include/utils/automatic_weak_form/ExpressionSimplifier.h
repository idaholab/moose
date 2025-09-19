#pragma once

#include "MooseAST.h"
#include <memory>
#include <cmath>

namespace moose
{
namespace automatic_weak_form
{

/**
 * Simplifies symbolic expressions to reduce complexity and improve numerical stability
 *
 * Floating-point comparisons use machine-epsilon based tolerances:
 * - Relative tolerance: 100 * machine epsilon for values away from zero
 * - Absolute tolerance: 100 * machine epsilon for values near zero
 * This ensures robust comparisons that adapt to the floating-point precision (float vs double)
 */
class ExpressionSimplifier
{
public:
  static NodePtr simplify(const NodePtr & expr);
  
private:
  static NodePtr simplifyNode(const NodePtr & node);
  static NodePtr simplifyBinaryOp(const BinaryOpNode * node);
  static NodePtr simplifyUnaryOp(const UnaryOpNode * node);
  static NodePtr simplifyVectorAssembly(const VectorAssemblyNode * node);
  static NodePtr simplifyAdd(const NodePtr & left, const NodePtr & right);
  static NodePtr simplifySubtract(const NodePtr & left, const NodePtr & right);
  static NodePtr simplifyMultiply(const NodePtr & left, const NodePtr & right);
  static NodePtr simplifyDivide(const NodePtr & left, const NodePtr & right);
  static NodePtr simplifyPower(const NodePtr & left, const NodePtr & right);
  static NodePtr simplifyNegate(const NodePtr & operand);
  
  static bool isZero(const NodePtr & node);
  static bool isOne(const NodePtr & node);
  static bool isConstant(const NodePtr & node, Real value);
  static Real getConstantValue(const NodePtr & node);
};

} // namespace automatic_weak_form
} // namespace moose
