#include "EvaluationTape.h"
#include "MooseAST.h"
#include "MooseError.h"
#include <cmath>
#include <limits>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>

namespace moose
{
namespace automatic_weak_form
{

namespace
{
constexpr std::size_t invalidIndex()
{
  return std::numeric_limits<std::size_t>::max();
}

TapeValue applyBinary(TapeOp op, const TapeValue & lhs, const TapeValue & rhs)
{
  return std::visit(
      [&](const auto & a, const auto & b) -> TapeValue {
        using L = std::decay_t<decltype(a)>;
        using R = std::decay_t<decltype(b)>;

        if constexpr (!std::is_same_v<L, R>)
          mooseError("Tape binary operation requires matching operand types");
        else if constexpr (std::is_same_v<L, TapeScalar>)
        {
          switch (op)
          {
            case TapeOp::Add:
              return a + b;
            case TapeOp::Subtract:
              return a - b;
            case TapeOp::Multiply:
              return a * b;
            case TapeOp::Divide:
              return a / b;
            case TapeOp::Power:
              return std::pow(a, b);
            default:
              mooseError("Unsupported scalar binary operation in tape evaluation");
          }
        }
        else
          mooseError("Tape binary operation not implemented for operand type");
      },
      lhs,
      rhs);
}

TapeValue applyUnary(TapeOp op, const TapeValue & value)
{
  return std::visit(
      [&](const auto & v) -> TapeValue {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, TapeScalar>)
        {
          switch (op)
          {
            case TapeOp::Negate:
              return -v;
            default:
              mooseError("Unsupported scalar unary operation in tape evaluation");
          }
        }
        else
          mooseError("Tape unary operation not implemented for operand type");
      },
      value);
}

class ScalarTapeBuilder
{
public:
  ScalarTapeBuilder(const std::string & primary_var,
                    const std::vector<std::string> & additional)
  {
    _allowed.insert(primary_var);
    for (const auto & name : additional)
      _allowed.insert(name);
  }

  TapeBuildResult build(const NodePtr & expr)
  {
    TapeBuildResult result;
    auto index = buildNode(expr);
    if (!index.has_value())
    {
      result.success = false;
      return result;
    }

    result.tape = _tape;
    result.inputs.assign(_inputs.begin(), _inputs.end());
    result.success = true;
    return result;
  }

private:
  std::optional<std::size_t> buildNode(const NodePtr & node)
  {
    if (!node)
      return std::nullopt;

    auto cache_it = _cache.find(node.get());
    if (cache_it != _cache.end())
      return cache_it->second;

    std::optional<std::size_t> index;

    switch (node->type())
    {
      case NodeType::Constant:
      {
        auto constant_node = std::static_pointer_cast<ConstantNode>(node);
        if (!constant_node->value().isScalar())
          return std::nullopt;
        TapeNode tape_node{TapeOp::LoadConstant};
        tape_node.payload = constant_node->value().asScalar();
        index = _tape.appendNode(tape_node);
        break;
      }

      case NodeType::Variable:
      case NodeType::FieldVariable:
      {
        std::string name;
        if (node->type() == NodeType::Variable)
          name = std::static_pointer_cast<VariableNode>(node)->name();
        else
          name = std::static_pointer_cast<FieldVariableNode>(node)->name();

        if (_allowed.count(name) == 0)
          return std::nullopt;

        _inputs.insert(name);

        TapeNode tape_node{TapeOp::LoadInput};
        tape_node.label = name;
        index = _tape.appendNode(tape_node);
        break;
      }

      case NodeType::Add:
      case NodeType::Subtract:
      case NodeType::Multiply:
      case NodeType::Divide:
      case NodeType::Power:
      {
        auto binary = std::static_pointer_cast<BinaryOpNode>(node);
        auto left_index = buildNode(binary->left());
        auto right_index = buildNode(binary->right());
        if (!left_index.has_value() || !right_index.has_value())
          return std::nullopt;

        TapeNode tape_node;
        tape_node.op = translateBinary(node->type());
        tape_node.lhs = *left_index;
        tape_node.rhs = *right_index;
        index = _tape.appendNode(tape_node);
        break;
      }

      case NodeType::Negate:
      {
        auto unary = std::static_pointer_cast<UnaryOpNode>(node);
        auto operand_index = buildNode(unary->operand());
        if (!operand_index.has_value())
          return std::nullopt;

        TapeNode tape_node{TapeOp::Negate};
        tape_node.lhs = *operand_index;
        index = _tape.appendNode(tape_node);
        break;
      }

      default:
        return std::nullopt;
    }

    if (index.has_value())
      _cache.emplace(node.get(), *index);
    return index;
  }

  TapeOp translateBinary(NodeType type) const
  {
    switch (type)
    {
      case NodeType::Add:
        return TapeOp::Add;
      case NodeType::Subtract:
        return TapeOp::Subtract;
      case NodeType::Multiply:
        return TapeOp::Multiply;
      case NodeType::Divide:
        return TapeOp::Divide;
      case NodeType::Power:
        return TapeOp::Power;
      default:
        mooseError("Unsupported binary node type in tape translation");
    }
  }

  EvaluationTape _tape;
  std::unordered_set<std::string> _allowed;
  std::unordered_set<std::string> _inputs;
  std::unordered_map<const Node *, std::size_t> _cache;
};

} // namespace

std::size_t
EvaluationTape::appendNode(const TapeNode & node)
{
  _nodes.push_back(node);
  return _nodes.size() - 1;
}

std::optional<TapeValue>
EvaluationTape::evaluate(const std::unordered_map<std::string, TapeValue> & inputs) const
{
  std::vector<TapeValue> workspace;
  workspace.reserve(_nodes.size());

  for (const auto & node : _nodes)
  {
    switch (node.op)
    {
      case TapeOp::LoadConstant:
        workspace.push_back(node.payload);
        break;

      case TapeOp::LoadInput:
      {
        auto it = inputs.find(node.label);
        if (it == inputs.end())
          return std::nullopt;
        workspace.push_back(it->second);
        break;
      }

      case TapeOp::Add:
      case TapeOp::Subtract:
      case TapeOp::Multiply:
      case TapeOp::Divide:
      case TapeOp::Power:
      {
        if (node.lhs == invalidIndex() || node.rhs == invalidIndex())
          return std::nullopt;
        const auto & lhs = workspace.at(node.lhs);
        const auto & rhs = workspace.at(node.rhs);
        workspace.push_back(applyBinary(node.op, lhs, rhs));
        break;
      }

      case TapeOp::Negate:
      {
        if (node.lhs == invalidIndex())
          return std::nullopt;
        const auto & val = workspace.at(node.lhs);
        workspace.push_back(applyUnary(node.op, val));
        break;
      }

      default:
        return std::nullopt;
    }
  }

  if (workspace.empty())
    return std::nullopt;

  return workspace.back();
}

TapeBuildResult
buildScalarTape(const NodePtr & expr,
                const std::string & differentiation_variable,
                const std::vector<std::string> & additional_variables)
{
  ScalarTapeBuilder builder(differentiation_variable, additional_variables);
  return builder.build(expr);
}

} // namespace automatic_weak_form
} // namespace moose
