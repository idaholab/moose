#include "ExpressionEvaluator.h"
#include "MooseError.h"
#include "MooseValueTypes.h"

#include <cmath>

namespace moose
{
namespace automatic_weak_form
{

ExpressionEvaluator::ExpressionEvaluator(unsigned int dim)
  : _dim(dim), _position(Point()), _time(0.0)
{
}

void
ExpressionEvaluator::setFieldValue(unsigned int var_num, Real value)
{
  _field_values[var_num] = value;
}

void
ExpressionEvaluator::setFieldGradient(unsigned int var_num, const RealGradient & gradient)
{
  _field_gradients[var_num] = gradient;
}

void
ExpressionEvaluator::setFieldHessian(unsigned int var_num, const RealTensor & hessian)
{
  _field_hessians[var_num] = hessian;
}

void
ExpressionEvaluator::setTestFunction(unsigned int var_num, Real value)
{
  _test_values[var_num] = value;
}

void
ExpressionEvaluator::setTestGradient(unsigned int var_num, const RealGradient & gradient)
{
  _test_gradients[var_num] = gradient;
}

void
ExpressionEvaluator::setShapeFunction(unsigned int var_num, Real value)
{
  _shape_values[var_num] = value;
}

void
ExpressionEvaluator::setShapeGradient(unsigned int var_num, const RealGradient & gradient)
{
  _shape_gradients[var_num] = gradient;
}

void
ExpressionEvaluator::registerVariable(const std::string & name, unsigned int var_num)
{
  _var_name_to_num[name] = var_num;
  _var_num_to_name[var_num] = name;
}

void
ExpressionEvaluator::setPosition(const Point & pos)
{
  _position = pos;
}

void
ExpressionEvaluator::setTime(Real t)
{
  _time = t;
}

void
ExpressionEvaluator::setParameter(const std::string & name, Real value)
{
  _parameters[name] = value;
}

void
ExpressionEvaluator::setVectorParameter(const std::string & name, const RealVectorValue & value)
{
  _vector_parameters[name] = value;
}

void
ExpressionEvaluator::setTensorParameter(const std::string & name, const RankTwoTensor & value)
{
  _tensor_parameters[name] = value;
}

Real
ExpressionEvaluator::evaluate(const NodePtr & expr)
{
  MooseValue value = evaluateNode(expr);
  if (!value.isScalar())
    mooseError("ExpressionEvaluator::evaluate expected scalar result");
  return value.asScalar();
}

Real
ExpressionEvaluator::evaluateDerivative(const NodePtr & expr, const std::string & var_name)
{
  auto it = _var_name_to_num.find(var_name);
  if (it == _var_name_to_num.end())
    mooseError("Unknown variable in derivative evaluation: ", var_name);
  return evaluateDerivativeByVarNum(expr, it->second);
}

Real
ExpressionEvaluator::evaluateDerivativeByVarNum(const NodePtr & expr, unsigned int var_num)
{
  auto it = _field_values.find(var_num);
  if (it == _field_values.end())
    mooseError("No field value registered for variable number ", var_num);

  Real original = it->second;
  Real eps = std::max(1e-8, 1e-6 * std::abs(original));

  setFieldValue(var_num, original + eps);
  Real forward = evaluate(expr);

  setFieldValue(var_num, original - eps);
  Real backward = evaluate(expr);

  setFieldValue(var_num, original);

  return (forward - backward) / (2.0 * eps);
}

RealVectorValue
ExpressionEvaluator::evaluateVector(const NodePtr & expr)
{
  MooseValue value = evaluateNode(expr);
  if (!value.isVector())
    mooseError("ExpressionEvaluator::evaluateVector expected vector result");
  return value.asVector();
}

RankTwoTensor
ExpressionEvaluator::evaluateTensor(const NodePtr & expr)
{
  MooseValue value = evaluateNode(expr);
  if (!value.isTensor())
    mooseError("ExpressionEvaluator::evaluateTensor expected tensor result");
  return value.asTensor();
}

MooseValue
ExpressionEvaluator::evaluateNode(const NodePtr & expr)
{
  if (!expr)
    return MooseValue(0.0);
  return evaluateNode(expr.get());
}

MooseValue
ExpressionEvaluator::evaluateNode(const Node * node)
{
  switch (node->type())
  {
    case NodeType::Constant:
      return visitConstant(static_cast<const ConstantNode *>(node));
    case NodeType::Variable:
      return visitVariable(static_cast<const VariableNode *>(node));
    case NodeType::FieldVariable:
      return visitFieldVariable(static_cast<const FieldVariableNode *>(node));
    case NodeType::Negate:
    case NodeType::Gradient:
    case NodeType::Norm:
    case NodeType::Trace:
    case NodeType::Determinant:
    case NodeType::Inverse:
    case NodeType::Transpose:
    case NodeType::Symmetric:
    case NodeType::Skew:
    case NodeType::Deviatoric:
      return visitUnaryOp(static_cast<const UnaryOpNode *>(node));
    case NodeType::Add:
    case NodeType::Subtract:
    case NodeType::Multiply:
    case NodeType::Divide:
    case NodeType::Power:
    case NodeType::Dot:
    case NodeType::Outer:
    case NodeType::Contract:
      return visitBinaryOp(static_cast<const BinaryOpNode *>(node));
    case NodeType::Function:
      return visitFunction(static_cast<const FunctionNode *>(node));
    case NodeType::VectorAssembly:
      return visitVectorAssembly(static_cast<const VectorAssemblyNode *>(node));
    case NodeType::VectorComponent:
    case NodeType::TensorComponent:
      return visitComponent(static_cast<const ComponentNode *>(node));
    default:
      mooseError("Unsupported node type in ExpressionEvaluator");
  }
}

MooseValue
ExpressionEvaluator::visitConstant(const ConstantNode * node)
{
  return node->value();
}

unsigned int
ExpressionEvaluator::getVarNum(const std::string & name) const
{
  auto it = _var_name_to_num.find(name);
  if (it == _var_name_to_num.end())
    mooseError("Unknown variable name: ", name);
  return it->second;
}

MooseValue
ExpressionEvaluator::visitVariable(const VariableNode * node)
{
  unsigned int var_num = getVarNum(node->name());
  auto it = _field_values.find(var_num);
  if (it != _field_values.end())
    return MooseValue(it->second);

  auto param_it = _parameters.find(node->name());
  if (param_it != _parameters.end())
    return MooseValue(param_it->second);

  mooseError("No value registered for variable: ", node->name());
}

MooseValue
ExpressionEvaluator::visitFieldVariable(const FieldVariableNode * node)
{
  unsigned int var_num = getVarNum(node->name());
  auto it = _field_values.find(var_num);
  if (it != _field_values.end())
    return MooseValue(it->second);

  mooseError("No field value registered for variable: ", node->name());
}

MooseValue
ExpressionEvaluator::visitUnaryOp(const UnaryOpNode * node)
{
  switch (node->type())
  {
    case NodeType::Negate:
      return applyUnaryOp(node->type(), evaluateNode(node->operand()));
    case NodeType::Gradient:
    {
      const NodePtr & operand = node->operand();
      if (operand->type() != NodeType::Variable && operand->type() != NodeType::FieldVariable)
        mooseError("Gradient currently supports field variables only");
      std::string name = (operand->type() == NodeType::Variable)
                             ? static_cast<const VariableNode &>(*operand).name()
                             : static_cast<const FieldVariableNode &>(*operand).name();
      unsigned int var_num = getVarNum(name);
      auto it = _field_gradients.find(var_num);
      if (it == _field_gradients.end())
        mooseError("No gradient registered for variable: ", name);
      return MooseValue(it->second, _dim);
    }
    case NodeType::Norm:
    case NodeType::Trace:
    case NodeType::Determinant:
    case NodeType::Inverse:
    case NodeType::Transpose:
    case NodeType::Symmetric:
    case NodeType::Skew:
    case NodeType::Deviatoric:
      return applyUnaryOp(node->type(), evaluateNode(node->operand()));
    default:
      mooseError("Unsupported unary operation in ExpressionEvaluator");
  }
}

MooseValue
ExpressionEvaluator::visitBinaryOp(const BinaryOpNode * node)
{
  MooseValue left = evaluateNode(node->left().get());
  MooseValue right = evaluateNode(node->right().get());
  return applyBinaryOp(node->type(), left, right);
}

MooseValue
ExpressionEvaluator::visitFunction(const FunctionNode * node)
{
  std::vector<MooseValue> args;
  args.reserve(node->args().size());
  for (const auto & arg : node->args())
    args.push_back(evaluateNode(arg.get()));
  return applyFunction(node->name(), args);
}

MooseValue
ExpressionEvaluator::visitVectorAssembly(const VectorAssemblyNode * node)
{
  const auto & components = node->components();
  RealVectorValue vec(0.0, 0.0, 0.0);
  for (std::size_t i = 0; i < components.size() && i < 3; ++i)
  {
    MooseValue comp_val = evaluateNode(components[i].get());
    if (!comp_val.isScalar())
      mooseError("VectorAssembly components must be scalar");
    vec(i) = comp_val.asScalar();
  }
  return MooseValue(vec, _dim);
}

MooseValue
ExpressionEvaluator::visitComponent(const ComponentNode * node)
{
  MooseValue value = evaluateNode(node->expr().get());
  if (node->type() == NodeType::VectorComponent)
  {
    if (!value.isVector())
      mooseError("Vector component requested from non-vector value");
    RealVectorValue vec = value.asVector();
    if (node->component() >= 3)
      mooseError("Vector component index out of range");
    return MooseValue(vec(node->component()));
  }
  else
  {
    if (!value.isTensor())
      mooseError("Tensor component requested from non-tensor value");
    RankTwoTensor tensor = value.asTensor();
    if (node->i() >= 3 || node->j() >= 3)
      mooseError("Tensor component index out of range");
    return MooseValue(tensor(node->i(), node->j()));
  }
}

MooseValue
ExpressionEvaluator::applyUnaryOp(NodeType op, const MooseValue & operand)
{
  switch (op)
  {
    case NodeType::Negate:
      if (!operand.isScalar())
        mooseError("Unary negation currently supports scalar values only");
      return MooseValue(-operand.asScalar());
    case NodeType::Norm:
      return norm(operand);
    case NodeType::Trace:
      return trace(operand);
    case NodeType::Determinant:
      if (!operand.isTensor())
        mooseError("Determinant requires tensor operand");
      return MooseValue(operand.asTensor().det());
    case NodeType::Inverse:
      return inverse(operand);
    case NodeType::Transpose:
      if (!operand.isTensor())
        mooseError("Transpose requires tensor operand");
      return MooseValue(operand.asTensor().transpose(), _dim);
    case NodeType::Symmetric:
      return sym(operand);
    case NodeType::Skew:
      return skew(operand);
    case NodeType::Deviatoric:
      return dev(operand);
    default:
      mooseError("Unsupported unary op in applyUnaryOp");
  }
}

MooseValue
ExpressionEvaluator::applyBinaryOp(NodeType op, const MooseValue & left, const MooseValue & right)
{
  switch (op)
  {
    case NodeType::Add:
      return left + right;
    case NodeType::Subtract:
      return left - right;
    case NodeType::Multiply:
      return left * right;
    case NodeType::Divide:
      if (left.isScalar() && right.isScalar())
        return MooseValue(left.asScalar() / right.asScalar());
      if (left.isVector() && right.isScalar())
        return MooseValue(left.asVector() / right.asScalar(), left.getDimension());
      if (left.isTensor() && right.isScalar())
      {
        RankTwoTensor result = left.asTensor();
        result /= right.asScalar();
        return MooseValue(result, left.getDimension());
      }
      mooseError("Division currently supports scalar/vector/tensor divided by scalar");
    case NodeType::Power:
      if (!left.isScalar() || !right.isScalar())
        mooseError("Power operation requires scalar operands");
      return MooseValue(std::pow(left.asScalar(), right.asScalar()));
    case NodeType::Dot:
      return dot(left, right);
    case NodeType::Contract:
      return contract(left, right);
    case NodeType::Outer:
      return outer(left, right);
    default:
      mooseError("Unsupported binary op in applyBinaryOp");
  }
}

MooseValue
ExpressionEvaluator::applyFunction(const std::string & name, const std::vector<MooseValue> & args)
{
  if (name == "sin" || name == "cos" || name == "exp" || name == "log" || name == "sqrt")
  {
    if (args.size() != 1 || !args[0].isScalar())
      mooseError(name, " expects one scalar argument");
    Real x = args[0].asScalar();
    if (name == "sin")
      return MooseValue(std::sin(x));
    if (name == "cos")
      return MooseValue(std::cos(x));
    if (name == "exp")
      return MooseValue(std::exp(x));
    if (name == "log")
      return MooseValue(std::log(x));
    return MooseValue(std::sqrt(x));
  }

  if (name == "W")
  {
    if (args.size() != 1 || !args[0].isScalar())
      mooseError("W expects one scalar argument");
    Real c = args[0].asScalar();
    Real val = 0.25 * std::pow(c * c - 1.0, 2.0);
    return MooseValue(val);
  }

  if (name == "dW_dc")
  {
    if (args.size() != 1 || !args[0].isScalar())
      mooseError("dW_dc expects one scalar argument");
    Real c = args[0].asScalar();
    Real val = 4.0 * c * (c * c - 1.0);
    return MooseValue(val);
  }

  auto param_it = _parameters.find(name);
  if (param_it != _parameters.end() && args.empty())
    return MooseValue(param_it->second);

  auto vec_it = _vector_parameters.find(name);
  if (vec_it != _vector_parameters.end() && args.empty())
    return MooseValue(vec_it->second, _dim);

  auto tensor_it = _tensor_parameters.find(name);
  if (tensor_it != _tensor_parameters.end() && args.empty())
    return MooseValue(tensor_it->second, _dim);

  mooseError("Unsupported function in ExpressionEvaluator: ", name);
}

} // namespace automatic_weak_form
} // namespace moose
