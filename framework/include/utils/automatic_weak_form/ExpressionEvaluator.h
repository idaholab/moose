#pragma once

#include "MooseAST.h"
#include "MooseTypes.h"
#include <map>
#include <string>
#include <vector>

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
  
  // Set field values at current quadrature point (using variable number for efficiency)
  void setFieldValue(unsigned int var_num, Real value);
  void setFieldGradient(unsigned int var_num, const RealGradient & gradient);
  void setFieldHessian(unsigned int var_num, const RealTensor & hessian);
  
  // Set test function values
  void setTestFunction(unsigned int var_num, Real value);
  void setTestGradient(unsigned int var_num, const RealGradient & gradient);
  
  // Set shape function values (for Jacobian)
  void setShapeFunction(unsigned int var_num, Real value);
  void setShapeGradient(unsigned int var_num, const RealGradient & gradient);
  
  // Map variable names to numbers for parsing
  void registerVariable(const std::string & name, unsigned int var_num);
  
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
  Real evaluateDerivativeByVarNum(const NodePtr & expr, unsigned int var_num);
  
  // Evaluate to get vector or tensor results
  RealVectorValue evaluateVector(const NodePtr & expr);
  RankTwoTensor evaluateTensor(const NodePtr & expr);
  
private:
  unsigned int _dim;
  
  // Storage for field values (indexed by variable number for performance)
  std::map<unsigned int, Real> _field_values;
  std::map<unsigned int, RealGradient> _field_gradients;
  std::map<unsigned int, RealTensor> _field_hessians;
  
  // Storage for test functions
  std::map<unsigned int, Real> _test_values;
  std::map<unsigned int, RealGradient> _test_gradients;
  
  // Storage for shape functions
  std::map<unsigned int, Real> _shape_values;
  std::map<unsigned int, RealGradient> _shape_gradients;
  
  // Map variable names to numbers for expression parsing
  std::map<std::string, unsigned int> _var_name_to_num;
  std::map<unsigned int, std::string> _var_num_to_name;
  
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
  MooseValue evaluateNode(const NodePtr & expr);
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

  unsigned int getVarNum(const std::string & name) const;
};

}
}
