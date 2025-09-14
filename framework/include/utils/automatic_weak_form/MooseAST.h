#pragma once

#include "MooseValueTypes.h"
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <functional>

namespace moose
{
namespace automatic_weak_form
{

class Node;
using NodePtr = std::shared_ptr<Node>;

enum class NodeType
{
  Constant,
  Variable,
  FieldVariable,
  TestFunction,
  ShapeFunction,
  Add,
  Subtract,
  Multiply,
  Divide,
  Power,
  Negate,
  Gradient,
  Divergence,
  Laplacian,
  Curl,
  Dot,
  Cross,
  Contract,
  Outer,
  Norm,
  Normalize,
  Trace,
  Determinant,
  Inverse,
  Transpose,
  Symmetric,
  Skew,
  Deviatoric,
  Function,
  TensorComponent,
  VectorComponent,
  VectorAssembly
};

class Node : public std::enable_shared_from_this<Node>
{
public:
  Node(NodeType type, const Shape & shape) : _type(type), _shape(shape) {}
  virtual ~Node() = default;
  
  NodeType type() const { return _type; }
  const Shape & shape() const { return _shape; }
  
  virtual std::string toString() const = 0;
  virtual NodePtr clone() const = 0;
  virtual bool equals(const Node & other) const = 0;
  
  virtual std::vector<NodePtr> children() const { return {}; }
  
  bool isScalar() const { return std::holds_alternative<ScalarShape>(_shape); }
  bool isVector() const { return std::holds_alternative<VectorShape>(_shape); }
  bool isTensor() const { return std::holds_alternative<TensorShape>(_shape); }
  
protected:
  NodeType _type;
  Shape _shape;
};

class ConstantNode : public Node
{
public:
  ConstantNode(const MooseValue & value) 
    : Node(NodeType::Constant, value.shape), _value(value) {}
  
  const MooseValue & value() const { return _value; }
  
  std::string toString() const override
  {
    if (_value.isScalar())
      return std::to_string(_value.asScalar());
    else if (_value.isVector())
      return "[vector]";
    else if (_value.isTensor())
      return "[tensor]";
    else
      return "[higher-rank]";
  }
  
  NodePtr clone() const override
  {
    return std::make_shared<ConstantNode>(_value);
  }
  
  bool equals(const Node & other) const override
  {
    if (other.type() != NodeType::Constant)
      return false;
    return true;
  }
  
private:
  MooseValue _value;
};

class VariableNode : public Node
{
public:
  VariableNode(const std::string & name, const Shape & shape) 
    : Node(NodeType::Variable, shape), _name(name) {}
  
  const std::string & name() const { return _name; }
  
  std::string toString() const override { return _name; }
  
  NodePtr clone() const override
  {
    return std::make_shared<VariableNode>(_name, _shape);
  }
  
  bool equals(const Node & other) const override
  {
    if (other.type() != NodeType::Variable)
      return false;
    auto & var = static_cast<const VariableNode &>(other);
    return _name == var._name;
  }
  
private:
  std::string _name;
};

class FieldVariableNode : public Node
{
public:
  FieldVariableNode(const std::string & name, const Shape & shape, unsigned int qp = 0) 
    : Node(NodeType::FieldVariable, shape), _name(name), _qp(qp) {}
  
  const std::string & name() const { return _name; }
  unsigned int qp() const { return _qp; }
  
  std::string toString() const override 
  { 
    // For symbolic manipulation, return just the variable name
    // Runtime evaluation will add the MOOSE-specific prefixes
    return _name; 
  }
  
  std::string toRuntimeString() const
  {
    // For runtime evaluation in MOOSE context
    return "_" + _name + "[_qp]";
  }
  
  NodePtr clone() const override
  {
    return std::make_shared<FieldVariableNode>(_name, _shape, _qp);
  }
  
  bool equals(const Node & other) const override
  {
    if (other.type() != NodeType::FieldVariable)
      return false;
    auto & var = static_cast<const FieldVariableNode &>(other);
    return _name == var._name && _qp == var._qp;
  }
  
private:
  std::string _name;
  unsigned int _qp;
};

class TestFunctionNode : public Node
{
public:
  TestFunctionNode(const std::string & var_name, const Shape & shape, 
                   unsigned int component = 0, bool is_gradient = false)
    : Node(NodeType::TestFunction, shape), 
      _var_name(var_name), _component(component), _is_gradient(is_gradient) {}
  
  const std::string & varName() const { return _var_name; }
  unsigned int component() const { return _component; }
  bool isGradient() const { return _is_gradient; }
  
  std::string toString() const override 
  { 
    // For symbolic manipulation
    if (_is_gradient)
      return "grad_test_" + _var_name;
    else
      return "test_" + _var_name;
  }
  
  std::string toRuntimeString() const
  {
    // For runtime evaluation in MOOSE context
    if (_is_gradient)
      return "_grad_test[_i][_qp]";
    else
      return "_test[_i][_qp]";
  }
  
  NodePtr clone() const override
  {
    return std::make_shared<TestFunctionNode>(_var_name, _shape, _component, _is_gradient);
  }
  
  bool equals(const Node & other) const override
  {
    if (other.type() != NodeType::TestFunction)
      return false;
    auto & test = static_cast<const TestFunctionNode &>(other);
    return _var_name == test._var_name && _component == test._component && 
           _is_gradient == test._is_gradient;
  }
  
private:
  std::string _var_name;
  unsigned int _component;
  bool _is_gradient;
};

class ShapeFunctionNode : public Node
{
public:
  ShapeFunctionNode(const std::string & var_name, const Shape & shape, 
                    unsigned int component = 0, bool is_gradient = false)
    : Node(NodeType::ShapeFunction, shape), 
      _var_name(var_name), _component(component), _is_gradient(is_gradient) {}
  
  const std::string & varName() const { return _var_name; }
  unsigned int component() const { return _component; }
  bool isGradient() const { return _is_gradient; }
  
  std::string toString() const override 
  { 
    // For symbolic manipulation
    if (_is_gradient)
      return "grad_phi_" + _var_name;
    else
      return "phi_" + _var_name;
  }
  
  std::string toRuntimeString() const
  {
    // For runtime evaluation in MOOSE context
    if (_is_gradient)
      return "_grad_phi[_j][_qp]";
    else
      return "_phi[_j][_qp]";
  }
  
  NodePtr clone() const override
  {
    return std::make_shared<ShapeFunctionNode>(_var_name, _shape, _component, _is_gradient);
  }
  
  bool equals(const Node & other) const override
  {
    if (other.type() != NodeType::ShapeFunction)
      return false;
    auto & shape = static_cast<const ShapeFunctionNode &>(other);
    return _var_name == shape._var_name && _component == shape._component && 
           _is_gradient == shape._is_gradient;
  }
  
private:
  std::string _var_name;
  unsigned int _component;
  bool _is_gradient;
};

class UnaryOpNode : public Node
{
public:
  UnaryOpNode(NodeType type, const NodePtr & operand, const Shape & shape)
    : Node(type, shape), _operand(operand)
  {
    // Debug shape issues in 1D
    if (type == NodeType::Gradient && operand->type() == NodeType::Gradient)
    {
      std::string shape_str = "unknown";
      if (std::holds_alternative<VectorShape>(shape))
        shape_str = "VECTOR(dim=" + std::to_string(std::get<VectorShape>(shape).dim) + ")";
      else if (std::holds_alternative<TensorShape>(shape))
        shape_str = "TENSOR(dim=" + std::to_string(std::get<TensorShape>(shape).dim) + ")";
      mooseInfo("[DEBUG UnaryOpNode] Creating grad(grad(...)) with shape=", shape_str);
    }
  }
  
  const NodePtr & operand() const { return _operand; }

  // Debug helper
  std::string shapeString() const
  {
    if (std::holds_alternative<VectorShape>(_shape))
      return "VECTOR(dim=" + std::to_string(std::get<VectorShape>(_shape).dim) + ")";
    else if (std::holds_alternative<TensorShape>(_shape))
      return "TENSOR(dim=" + std::to_string(std::get<TensorShape>(_shape).dim) + ")";
    else if (std::holds_alternative<ScalarShape>(_shape))
      return "SCALAR";
    else
      return "UNKNOWN";
  }
  
  std::vector<NodePtr> children() const override { return {_operand}; }
  
  std::string toString() const override
  {
    switch (_type)
    {
      case NodeType::Negate: return "-(" + _operand->toString() + ")";
      case NodeType::Gradient: return "grad(" + _operand->toString() + ")";
      case NodeType::Divergence: return "div(" + _operand->toString() + ")";
      case NodeType::Laplacian: return "laplacian(" + _operand->toString() + ")";
      case NodeType::Curl: return "curl(" + _operand->toString() + ")";
      case NodeType::Norm: return "norm(" + _operand->toString() + ")";
      case NodeType::Normalize: return "normalize(" + _operand->toString() + ")";
      case NodeType::Trace: return "trace(" + _operand->toString() + ")";
      case NodeType::Determinant: return "det(" + _operand->toString() + ")";
      case NodeType::Inverse: return "inv(" + _operand->toString() + ")";
      case NodeType::Transpose: return "transpose(" + _operand->toString() + ")";
      case NodeType::Symmetric: return "sym(" + _operand->toString() + ")";
      case NodeType::Skew: return "skew(" + _operand->toString() + ")";
      case NodeType::Deviatoric: return "dev(" + _operand->toString() + ")";
      default: return "unary_op(" + _operand->toString() + ")";
    }
  }
  
  NodePtr clone() const override
  {
    auto cloned = std::make_shared<UnaryOpNode>(_type, _operand->clone(), _shape);

    // Debug cloning issues
    if (_type == NodeType::Gradient && _operand->type() == NodeType::Gradient)
    {
      std::string orig_shape = "unknown";
      if (std::holds_alternative<VectorShape>(_shape))
        orig_shape = "VECTOR(dim=" + std::to_string(std::get<VectorShape>(_shape).dim) + ")";
      else if (std::holds_alternative<TensorShape>(_shape))
        orig_shape = "TENSOR(dim=" + std::to_string(std::get<TensorShape>(_shape).dim) + ")";

      std::string cloned_shape = "unknown";
      if (cloned->isVector())
      {
        auto shape = cloned->shape();
        if (std::holds_alternative<VectorShape>(shape))
          cloned_shape = "VECTOR(dim=" + std::to_string(std::get<VectorShape>(shape).dim) + ")";
      }
      else if (cloned->isTensor())
      {
        auto shape = cloned->shape();
        if (std::holds_alternative<TensorShape>(shape))
          cloned_shape = "TENSOR(dim=" + std::to_string(std::get<TensorShape>(shape).dim) + ")";
      }

      mooseInfo("[DEBUG clone] grad(grad(...)) original shape=", orig_shape, " cloned shape=", cloned_shape);
    }

    return cloned;
  }
  
  bool equals(const Node & other) const override
  {
    if (other.type() != _type)
      return false;
    auto & unary = static_cast<const UnaryOpNode &>(other);
    return _operand->equals(*unary._operand);
  }
  
protected:
  NodePtr _operand;
};

class BinaryOpNode : public Node
{
public:
  BinaryOpNode(NodeType type, const NodePtr & left, const NodePtr & right, const Shape & shape)
    : Node(type, shape), _left(left), _right(right) {}
  
  const NodePtr & left() const { return _left; }
  const NodePtr & right() const { return _right; }
  
  std::vector<NodePtr> children() const override { return {_left, _right}; }
  
  std::string toString() const override
  {
    switch (_type)
    {
      case NodeType::Add: return "(" + _left->toString() + " + " + _right->toString() + ")";
      case NodeType::Subtract: return "(" + _left->toString() + " - " + _right->toString() + ")";
      case NodeType::Multiply: return "(" + _left->toString() + " * " + _right->toString() + ")";
      case NodeType::Divide: return "(" + _left->toString() + " / " + _right->toString() + ")";
      case NodeType::Power: return "pow(" + _left->toString() + ", " + _right->toString() + ")";
      case NodeType::Dot: return "dot(" + _left->toString() + ", " + _right->toString() + ")";
      case NodeType::Cross: return "cross(" + _left->toString() + ", " + _right->toString() + ")";
      case NodeType::Contract: return "contract(" + _left->toString() + ", " + _right->toString() + ")";
      case NodeType::Outer: return "outer(" + _left->toString() + ", " + _right->toString() + ")";
      default: return "binary_op(" + _left->toString() + ", " + _right->toString() + ")";
    }
  }
  
  NodePtr clone() const override
  {
    return std::make_shared<BinaryOpNode>(_type, _left->clone(), _right->clone(), _shape);
  }
  
  bool equals(const Node & other) const override
  {
    if (other.type() != _type)
      return false;
    auto & binary = static_cast<const BinaryOpNode &>(other);
    return _left->equals(*binary._left) && _right->equals(*binary._right);
  }
  
protected:
  NodePtr _left;
  NodePtr _right;
};

class FunctionNode : public Node
{
public:
  FunctionNode(const std::string & name, const std::vector<NodePtr> & args, const Shape & shape)
    : Node(NodeType::Function, shape), _name(name), _args(args) {}
  
  const std::string & name() const { return _name; }
  const std::vector<NodePtr> & args() const { return _args; }
  
  std::vector<NodePtr> children() const override { return _args; }
  
  std::string toString() const override
  {
    std::string result = _name + "(";
    for (size_t i = 0; i < _args.size(); ++i)
    {
      if (i > 0)
        result += ", ";
      result += _args[i]->toString();
    }
    result += ")";
    return result;
  }
  
  NodePtr clone() const override
  {
    std::vector<NodePtr> cloned_args;
    for (const auto & arg : _args)
      cloned_args.push_back(arg->clone());
    return std::make_shared<FunctionNode>(_name, cloned_args, _shape);
  }
  
  bool equals(const Node & other) const override
  {
    if (other.type() != NodeType::Function)
      return false;
    auto & func = static_cast<const FunctionNode &>(other);
    if (_name != func._name || _args.size() != func._args.size())
      return false;
    for (size_t i = 0; i < _args.size(); ++i)
      if (!_args[i]->equals(*func._args[i]))
        return false;
    return true;
  }
  
private:
  std::string _name;
  std::vector<NodePtr> _args;
};

class VectorAssemblyNode : public Node
{
public:
  VectorAssemblyNode(const std::vector<NodePtr> & components, const Shape & shape)
    : Node(NodeType::VectorAssembly, shape), _components(components) {}
  
  const std::vector<NodePtr> & components() const { return _components; }
  
  std::vector<NodePtr> children() const override { return _components; }
  
  std::string toString() const override
  {
    std::string result = "vec(";
    for (size_t i = 0; i < _components.size(); ++i)
    {
      if (i > 0)
        result += ", ";
      result += _components[i]->toString();
    }
    result += ")";
    return result;
  }
  
  NodePtr clone() const override
  {
    std::vector<NodePtr> cloned_components;
    for (const auto & comp : _components)
      cloned_components.push_back(comp->clone());
    return std::make_shared<VectorAssemblyNode>(cloned_components, _shape);
  }
  
  bool equals(const Node & other) const override
  {
    if (other.type() != _type)
      return false;
    auto & va = static_cast<const VectorAssemblyNode &>(other);
    if (_components.size() != va._components.size())
      return false;
    for (size_t i = 0; i < _components.size(); ++i)
      if (!_components[i]->equals(*va._components[i]))
        return false;
    return true;
  }
  
private:
  std::vector<NodePtr> _components;
};

class ComponentNode : public Node
{
public:
  ComponentNode(const NodePtr & expr, unsigned int component, const Shape & shape)
    : Node(NodeType::VectorComponent, shape), _expr(expr), _component(component) {}
  
  ComponentNode(const NodePtr & expr, unsigned int i, unsigned int j, const Shape & shape)
    : Node(NodeType::TensorComponent, shape), _expr(expr), _i(i), _j(j) {}
  
  const NodePtr & expr() const { return _expr; }
  unsigned int component() const { return _component; }
  unsigned int i() const { return _i; }
  unsigned int j() const { return _j; }
  
  std::vector<NodePtr> children() const override { return {_expr}; }
  
  std::string toString() const override
  {
    if (_type == NodeType::VectorComponent)
      return _expr->toString() + "[" + std::to_string(_component) + "]";
    else
      return _expr->toString() + "[" + std::to_string(_i) + "," + std::to_string(_j) + "]";
  }
  
  NodePtr clone() const override
  {
    if (_type == NodeType::VectorComponent)
      return std::make_shared<ComponentNode>(_expr->clone(), _component, _shape);
    else
      return std::make_shared<ComponentNode>(_expr->clone(), _i, _j, _shape);
  }
  
  bool equals(const Node & other) const override
  {
    if (other.type() != _type)
      return false;
    auto & comp = static_cast<const ComponentNode &>(other);
    if (_type == NodeType::VectorComponent)
      return _expr->equals(*comp._expr) && _component == comp._component;
    else
      return _expr->equals(*comp._expr) && _i == comp._i && _j == comp._j;
  }
  
private:
  NodePtr _expr;
  unsigned int _component = 0;
  unsigned int _i = 0, _j = 0;
};

class Expression
{
public:
  Expression() = default;
  explicit Expression(const NodePtr & root) : _root(root) {}
  
  const NodePtr & root() const { return _root; }
  void setRoot(const NodePtr & root) { _root = root; }
  
  bool empty() const { return !_root; }
  
  std::string toString() const
  {
    if (_root)
      return _root->toString();
    return "empty";
  }
  
  Expression operator+(const Expression & other) const;
  Expression operator-(const Expression & other) const;
  Expression operator*(const Expression & other) const;
  Expression operator/(const Expression & other) const;
  Expression operator-() const;
  
private:
  NodePtr _root;
};

inline NodePtr constant(Real value)
{
  return std::make_shared<ConstantNode>(MooseValue(value));
}

inline NodePtr constant(const RealVectorValue & value, unsigned int dim)
{
  return std::make_shared<ConstantNode>(MooseValue(value, dim));
}

inline NodePtr constant(const RankTwoTensor & value, unsigned int dim)
{
  return std::make_shared<ConstantNode>(MooseValue(value, dim));
}

inline NodePtr variable(const std::string & name, const Shape & shape = ScalarShape{})
{
  return std::make_shared<VariableNode>(name, shape);
}

inline NodePtr fieldVariable(const std::string & name, const Shape & shape = ScalarShape{})
{
  return std::make_shared<FieldVariableNode>(name, shape);
}

inline NodePtr testFunction(const std::string & var_name, bool is_gradient = false)
{
  Shape shape = is_gradient ? Shape(VectorShape{3}) : Shape(ScalarShape{});
  return std::make_shared<TestFunctionNode>(var_name, shape, 0, is_gradient);
}

inline NodePtr shapeFunction(const std::string & var_name, bool is_gradient = false)
{
  Shape shape = is_gradient ? Shape(VectorShape{3}) : Shape(ScalarShape{});
  return std::make_shared<ShapeFunctionNode>(var_name, shape, 0, is_gradient);
}

inline NodePtr add(const NodePtr & a, const NodePtr & b)
{
  return std::make_shared<BinaryOpNode>(NodeType::Add, a, b, a->shape());
}

inline NodePtr subtract(const NodePtr & a, const NodePtr & b)
{
  return std::make_shared<BinaryOpNode>(NodeType::Subtract, a, b, a->shape());
}

inline NodePtr multiply(const NodePtr & a, const NodePtr & b)
{
  NodePtr result;
  if (a->isScalar())
    result = std::make_shared<BinaryOpNode>(NodeType::Multiply, a, b, b->shape());
  else if (b->isScalar())
    result = std::make_shared<BinaryOpNode>(NodeType::Multiply, a, b, a->shape());
  else if (a->isVector() && b->isVector())
    result = std::make_shared<BinaryOpNode>(NodeType::Multiply, a, b, ScalarShape{});
  else
    result = std::make_shared<BinaryOpNode>(NodeType::Multiply, a, b, a->shape());
  
  // Debug output for shape tracking
  std::string a_shape = "unknown";
  if (a->isScalar()) a_shape = "scalar";
  else if (a->isVector()) a_shape = "vector";
  else if (a->isTensor()) a_shape = "tensor";
  
  std::string b_shape = "unknown";
  if (b->isScalar()) b_shape = "scalar";
  else if (b->isVector()) b_shape = "vector";
  else if (b->isTensor()) b_shape = "tensor";
  
  std::string result_shape = "unknown";
  if (result->isScalar()) result_shape = "scalar";
  else if (result->isVector()) result_shape = "vector";
  else if (result->isTensor()) result_shape = "tensor";
  
  if (a_shape == "scalar" && b_shape == "tensor")
    mooseInfo("[DEBUG] multiply: ", a->toString(), " (", a_shape, ") * ", b->toString(), " (", b_shape, ") -> shape = ", result_shape);
  
  return result;
}

inline NodePtr divide(const NodePtr & a, const NodePtr & b)
{
  return std::make_shared<BinaryOpNode>(NodeType::Divide, a, b, a->shape());
}

inline NodePtr power(const NodePtr & a, const NodePtr & b)
{
  return std::make_shared<BinaryOpNode>(NodeType::Power, a, b, a->shape());
}

inline NodePtr negate(const NodePtr & a)
{
  return std::make_shared<UnaryOpNode>(NodeType::Negate, a, a->shape());
}

inline NodePtr grad(const NodePtr & a, unsigned int dim = 3)
{
  auto result_shape = gradientShape(a->shape(), dim);
  auto result = std::make_shared<UnaryOpNode>(NodeType::Gradient, a, result_shape.shape);
  
  // Debug output for nested gradients
  if (a->type() == NodeType::Gradient && dim == 1)
  {
    std::string input_shape_str = "unknown";
    if (a->isScalar()) input_shape_str = "scalar";
    else if (a->isVector()) input_shape_str = "vector";
    else if (a->isTensor()) input_shape_str = "tensor";
    
    std::string output_shape_str = "unknown";
    if (result->isScalar()) output_shape_str = "scalar";
    else if (result->isVector()) output_shape_str = "vector";
    else if (result->isTensor()) output_shape_str = "tensor";
    
    mooseInfo("[DEBUG] grad(grad(...)) in 1D: input=", a->toString(), " shape=", input_shape_str, 
              " -> output shape=", output_shape_str);
  }
  
  return result;
}

inline NodePtr div(const NodePtr & a)
{
  auto result_shape = divergenceShape(a->shape());
  return std::make_shared<UnaryOpNode>(NodeType::Divergence, a, result_shape);
}

inline NodePtr laplacian(const NodePtr & a)
{
  return std::make_shared<UnaryOpNode>(NodeType::Laplacian, a, a->shape());
}

inline NodePtr curl(const NodePtr & a, unsigned int dim = 3)
{
  if (dim == 2)
    return std::make_shared<UnaryOpNode>(NodeType::Curl, a, ScalarShape{});
  else
    return std::make_shared<UnaryOpNode>(NodeType::Curl, a, VectorShape{dim});
}

inline NodePtr dot(const NodePtr & a, const NodePtr & b)
{
  if (a->isVector() && b->isVector())
    return std::make_shared<BinaryOpNode>(NodeType::Dot, a, b, ScalarShape{});
  else if (a->isTensor() && b->isVector())
    return std::make_shared<BinaryOpNode>(NodeType::Dot, a, b, b->shape());
  else
    return std::make_shared<BinaryOpNode>(NodeType::Dot, a, b, ScalarShape{});
}

inline NodePtr cross(const NodePtr & a, const NodePtr & b)
{
  return std::make_shared<BinaryOpNode>(NodeType::Cross, a, b, a->shape());
}

inline NodePtr contract(const NodePtr & a, const NodePtr & b)
{
  return std::make_shared<BinaryOpNode>(NodeType::Contract, a, b, ScalarShape{});
}

inline NodePtr outer(const NodePtr & a, const NodePtr & b)
{
  if (a->isVector() && b->isVector())
  {
    auto dim = std::get<VectorShape>(a->shape()).dim;
    return std::make_shared<BinaryOpNode>(NodeType::Outer, a, b, TensorShape{dim});
  }
  return std::make_shared<BinaryOpNode>(NodeType::Outer, a, b, TensorShape{3});
}

inline NodePtr norm(const NodePtr & a)
{
  return std::make_shared<UnaryOpNode>(NodeType::Norm, a, ScalarShape{});
}

inline NodePtr normalize(const NodePtr & a)
{
  return std::make_shared<UnaryOpNode>(NodeType::Normalize, a, a->shape());
}

inline NodePtr trace(const NodePtr & a)
{
  return std::make_shared<UnaryOpNode>(NodeType::Trace, a, ScalarShape{});
}

inline NodePtr det(const NodePtr & a)
{
  return std::make_shared<UnaryOpNode>(NodeType::Determinant, a, ScalarShape{});
}

inline NodePtr inv(const NodePtr & a)
{
  return std::make_shared<UnaryOpNode>(NodeType::Inverse, a, a->shape());
}

inline NodePtr transpose(const NodePtr & a)
{
  return std::make_shared<UnaryOpNode>(NodeType::Transpose, a, a->shape());
}

inline NodePtr sym(const NodePtr & a)
{
  return std::make_shared<UnaryOpNode>(NodeType::Symmetric, a, a->shape());
}

inline NodePtr skew(const NodePtr & a)
{
  return std::make_shared<UnaryOpNode>(NodeType::Skew, a, a->shape());
}

inline NodePtr dev(const NodePtr & a)
{
  return std::make_shared<UnaryOpNode>(NodeType::Deviatoric, a, a->shape());
}

inline NodePtr vec2(const NodePtr & x, const NodePtr & y)
{
  std::vector<NodePtr> components = {x, y};
  return std::make_shared<VectorAssemblyNode>(components, VectorShape{2});
}

inline NodePtr vec3(const NodePtr & x, const NodePtr & y, const NodePtr & z)
{
  std::vector<NodePtr> components = {x, y, z};
  return std::make_shared<VectorAssemblyNode>(components, VectorShape{3});
}

inline NodePtr function(const std::string & name, const std::vector<NodePtr> & args, 
                        const Shape & shape = ScalarShape{})
{
  return std::make_shared<FunctionNode>(name, args, shape);
}

inline Expression Expression::operator+(const Expression & other) const
{
  return Expression(add(_root, other._root));
}

inline Expression Expression::operator-(const Expression & other) const
{
  return Expression(subtract(_root, other._root));
}

inline Expression Expression::operator*(const Expression & other) const
{
  return Expression(multiply(_root, other._root));
}

inline Expression Expression::operator/(const Expression & other) const
{
  return Expression(divide(_root, other._root));
}

inline Expression Expression::operator-() const
{
  return Expression(negate(_root));
}

}
}