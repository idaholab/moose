#pragma once

#include "MooseAST.h"
#include "MooseTypes.h"
#include <map>
#include <string>

namespace moose
{
namespace automatic_weak_form
{

/**
 * Runtime evaluator for symbolic expressions
 * Converts AST nodes to numerical values at quadrature points
 */
class ExpressionEvaluator
{
public:
  ExpressionEvaluator(unsigned int dim = 3);
  
  // Set field values at current quadrature point
  void setFieldValue(const std::string & name, Real value);
  void setFieldGradient(const std::string & name, const RealGradient & gradient);
  void setFieldHessian(const std::string & name, const RealTensor & hessian);
  
  // Set test function values
  void setTestFunction(const std::string & name, Real value);
  void setTestGradient(const std::string & name, const RealGradient & gradient);
  
  // Set shape function values (for Jacobian)
  void setShapeFunction(const std::string & name, Real value);
  void setShapeGradient(const std::string & name, const RealGradient & gradient);
  
  // Set position and time
  void setPosition(const Point & pos);
  void setTime(Real t);
  
  // Set parameters
  void setParameter(const std::string & name, Real value);
  void setVectorParameter(const std::string & name, const RealVectorValue & value);
  void setTensorParameter(const std::string & name, const RankTwoTensor & value);
  
  // Evaluate an expression to get a scalar result
  Real evaluate(const NodePtr & expr);
  
  // Evaluate derivative of expression with respect to a variable (for AD)
  Real evaluateDerivative(const NodePtr & expr, const std::string & var_name);
  
  // Evaluate to get vector or tensor results
  RealVectorValue evaluateVector(const NodePtr & expr);
  RankTwoTensor evaluateTensor(const NodePtr & expr);
  
private:
  unsigned int _dim;
  
  // Storage for field values
  std::map<std::string, Real> _field_values;
  std::map<std::string, RealGradient> _field_gradients;
  std::map<std::string, RealTensor> _field_hessians;
  
  // Storage for test functions
  std::map<std::string, Real> _test_values;
  std::map<std::string, RealGradient> _test_gradients;
  
  // Storage for shape functions
  std::map<std::string, Real> _shape_values;
  std::map<std::string, RealGradient> _shape_gradients;
  
  // Parameters
  std::map<std::string, Real> _parameters;
  std::map<std::string, RealVectorValue> _vector_parameters;
  std::map<std::string, RankTwoTensor> _tensor_parameters;
  
  // Current position and time
  Point _position;
  Real _time;
  
  // Visitor for evaluation
  class EvaluationVisitor;
  
  // Internal evaluation methods
  MooseValue evaluateNode(const Node * node);
  MooseValue visitConstant(const ConstantNode * node);
  MooseValue visitVariable(const VariableNode * node);
  MooseValue visitFieldVariable(const FieldVariableNode * node);
  MooseValue visitUnaryOp(const UnaryOpNode * node);
  MooseValue visitBinaryOp(const BinaryOpNode * node);
  MooseValue visitFunction(const FunctionNode * node);
  MooseValue visitVectorAssembly(const VectorAssemblyNode * node);
  MooseValue visitComponent(const ComponentNode * node);
  
  // Helper functions for operations
  MooseValue applyUnaryOp(NodeType op, const MooseValue & operand);
  MooseValue applyBinaryOp(NodeType op, const MooseValue & left, const MooseValue & right);
  MooseValue applyFunction(const std::string & name, const std::vector<MooseValue> & args);
};

}
}