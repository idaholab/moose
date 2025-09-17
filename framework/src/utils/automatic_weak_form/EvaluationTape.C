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

TapeValue negateValue(const TapeValue & value)
{
  return std::visit(
      [](const auto & v) -> TapeValue {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, TapeScalar> ||
                      std::is_same_v<T, TapeVector> ||
                      std::is_same_v<T, TapeRank2> ||
                      std::is_same_v<T, TapeRank3> ||
                      std::is_same_v<T, TapeRank4>)
          return -v;
        else
          mooseError("Unsupported tape negate operand type");
      },
      value);
}

TapeValue addValues(const TapeValue & lhs, const TapeValue & rhs)
{
  return std::visit(
      [](const auto & a, const auto & b) -> TapeValue {
        using L = std::decay_t<decltype(a)>;
        using R = std::decay_t<decltype(b)>;
        if constexpr (std::is_same_v<L, R>)
          return a + b;
        else
          mooseError("Tape addition requires matching operand types");
      },
      lhs,
      rhs);
}

TapeValue subtractValues(const TapeValue & lhs, const TapeValue & rhs)
{
  return std::visit(
      [](const auto & a, const auto & b) -> TapeValue {
        using L = std::decay_t<decltype(a)>;
        using R = std::decay_t<decltype(b)>;
        if constexpr (std::is_same_v<L, R>)
          return a - b;
        else
          mooseError("Tape subtraction requires matching operand types");
      },
      lhs,
      rhs);
}

TapeValue multiplyValues(const TapeValue & lhs, const TapeValue & rhs)
{
  return std::visit(
      [](const auto & a, const auto & b) -> TapeValue {
        using L = std::decay_t<decltype(a)>;
        using R = std::decay_t<decltype(b)>;

        if constexpr (std::is_same_v<L, TapeScalar> && std::is_same_v<R, TapeScalar>)
          return a * b;
        else if constexpr (std::is_same_v<L, TapeScalar> && std::is_same_v<R, TapeVector>)
          return b * a;
        else if constexpr (std::is_same_v<L, TapeVector> && std::is_same_v<R, TapeScalar>)
          return a * b;
        else if constexpr (std::is_same_v<L, TapeScalar> && std::is_same_v<R, TapeRank2>)
        {
          TapeRank2 result = b;
          result *= a;
          return result;
        }
        else if constexpr (std::is_same_v<L, TapeRank2> && std::is_same_v<R, TapeScalar>)
        {
          TapeRank2 result = a;
          result *= b;
          return result;
        }
        else if constexpr (std::is_same_v<L, TapeScalar> && std::is_same_v<R, TapeRank3>)
        {
          TapeRank3 result = b;
          result *= a;
          return result;
        }
        else if constexpr (std::is_same_v<L, TapeRank3> && std::is_same_v<R, TapeScalar>)
        {
          TapeRank3 result = a;
          result *= b;
          return result;
        }
        else if constexpr (std::is_same_v<L, TapeScalar> && std::is_same_v<R, TapeRank4>)
        {
          TapeRank4 result = b;
          result *= a;
          return result;
        }
        else if constexpr (std::is_same_v<L, TapeRank4> && std::is_same_v<R, TapeScalar>)
        {
          TapeRank4 result = a;
          result *= b;
          return result;
        }
        else if constexpr (std::is_same_v<L, TapeRank2> && std::is_same_v<R, TapeVector>)
          return a * b;
        else if constexpr (std::is_same_v<L, TapeVector> && std::is_same_v<R, TapeRank2>)
          return b.transpose() * a; // transpose multiply to keep vector result
        else if constexpr (std::is_same_v<L, TapeRank2> && std::is_same_v<R, TapeRank2>)
          return a * b;
        else
          mooseError("Unsupported tape multiplication operand types");
      },
      lhs,
      rhs);
}

TapeValue divideValues(const TapeValue & lhs, const TapeValue & rhs)
{
  return std::visit(
      [](const auto & a, const auto & b) -> TapeValue {
        using L = std::decay_t<decltype(a)>;
        using R = std::decay_t<decltype(b)>;

        if constexpr (std::is_same_v<L, TapeScalar> && std::is_same_v<R, TapeScalar>)
          return a / b;
        else if constexpr (std::is_same_v<L, TapeVector> && std::is_same_v<R, TapeScalar>)
          return a / b;
        else if constexpr (std::is_same_v<L, TapeRank2> && std::is_same_v<R, TapeScalar>)
        {
          TapeRank2 result = a;
          result /= b;
          return result;
        }
        else if constexpr (std::is_same_v<L, TapeRank3> && std::is_same_v<R, TapeScalar>)
        {
          TapeRank3 result = a;
          result /= b;
          return result;
        }
        else if constexpr (std::is_same_v<L, TapeRank4> && std::is_same_v<R, TapeScalar>)
        {
          TapeRank4 result = a;
          result /= b;
          return result;
        }
        else
          mooseError("Unsupported tape division operand types");
      },
      lhs,
      rhs);
}

TapeValue powerValues(const TapeValue & lhs, const TapeValue & rhs)
{
  return std::visit(
      [](const auto & a, const auto & b) -> TapeValue {
        using L = std::decay_t<decltype(a)>;
        using R = std::decay_t<decltype(b)>;
        if constexpr (std::is_same_v<L, TapeScalar> && std::is_same_v<R, TapeScalar>)
          return std::pow(a, b);
        else
          mooseError("Tape power currently supports scalar operands only");
      },
      lhs,
      rhs);
}

TapeValue dotValues(const TapeValue & lhs, const TapeValue & rhs)
{
  return std::visit(
      [](const auto & a, const auto & b) -> TapeValue {
        using L = std::decay_t<decltype(a)>;
        using R = std::decay_t<decltype(b)>;

        if constexpr (std::is_same_v<L, TapeVector> && std::is_same_v<R, TapeVector>)
          return a * b;
        else
          mooseError("Tape dot product requires two vectors");
      },
      lhs,
      rhs);
}

TapeValue outerValues(const TapeValue & lhs, const TapeValue & rhs)
{
  return std::visit(
      [](const auto & a, const auto & b) -> TapeValue {
        using L = std::decay_t<decltype(a)>;
        using R = std::decay_t<decltype(b)>;

        if constexpr (std::is_same_v<L, TapeVector> && std::is_same_v<R, TapeVector>)
        {
          RankTwoTensor result;
          for (unsigned int i = 0; i < 3; ++i)
            for (unsigned int j = 0; j < 3; ++j)
              result(i, j) = a(i) * b(j);
          return result;
        }
        else
          mooseError("Tape outer product requires two vectors");
      },
      lhs,
      rhs);
}

TapeValue contractValues(const TapeValue & lhs, const TapeValue & rhs)
{
  return std::visit(
      [](const auto & a, const auto & b) -> TapeValue {
        using L = std::decay_t<decltype(a)>;
        using R = std::decay_t<decltype(b)>;

        if constexpr (std::is_same_v<L, TapeRank2> && std::is_same_v<R, TapeRank2>)
          return a.doubleContraction(b);
        else
          mooseError("Tape contract requires two rank-two tensors");
      },
      lhs,
      rhs);
}

class ScalarTapeBuilder
{
public:
  ScalarTapeBuilder(const std::string & primary_var,
                    const std::vector<std::string> & additional)
  {
    _allowed.insert(primary_var);
    _allowed.insert(primary_var + "_grad");
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
        TapeNode tape_node{};
        tape_node.op = TapeOp::LoadConstant;
        tape_node.lhs = invalidIndex();
        tape_node.rhs = invalidIndex();
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

        TapeNode tape_node{};
        tape_node.op = TapeOp::LoadInput;
        tape_node.lhs = invalidIndex();
        tape_node.rhs = invalidIndex();
        tape_node.label = name;
        index = _tape.appendNode(tape_node);
        break;
      }

      case NodeType::Gradient:
      {
        auto unary = std::static_pointer_cast<UnaryOpNode>(node);
        auto operand = unary->operand();

        std::string base_name;
        if (operand->type() == NodeType::Variable)
          base_name = std::static_pointer_cast<VariableNode>(operand)->name();
        else if (operand->type() == NodeType::FieldVariable)
          base_name = std::static_pointer_cast<FieldVariableNode>(operand)->name();
        else
          return std::nullopt;

        std::string grad_name = base_name + "_grad";
        if (_allowed.count(grad_name) == 0)
          return std::nullopt;

        _inputs.insert(grad_name);

        TapeNode tape_node{};
        tape_node.op = TapeOp::LoadInput;
        tape_node.lhs = invalidIndex();
        tape_node.rhs = invalidIndex();
        tape_node.label = grad_name;
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

      case NodeType::Dot:
      case NodeType::Contract:
      case NodeType::Outer:
      {
        auto binary = std::static_pointer_cast<BinaryOpNode>(node);
        auto left_index = buildNode(binary->left());
        auto right_index = buildNode(binary->right());
        if (!left_index.has_value() || !right_index.has_value())
          return std::nullopt;

        TapeNode tape_node;
        tape_node.op = translateSpecialBinary(node->type());
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

        TapeNode tape_node{};
        tape_node.op = TapeOp::Negate;
        tape_node.lhs = *operand_index;
        tape_node.rhs = invalidIndex();
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

  TapeOp translateSpecialBinary(NodeType type) const
  {
    switch (type)
    {
      case NodeType::Dot:
        return TapeOp::Dot;
      case NodeType::Outer:
        return TapeOp::Outer;
      case NodeType::Contract:
        return TapeOp::Contract;
      default:
        mooseError("Unsupported special binary node type in tape translation");
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
      case TapeOp::Dot:
      case TapeOp::Outer:
      case TapeOp::Contract:
      {
        if (node.lhs == invalidIndex() || node.rhs == invalidIndex())
          return std::nullopt;
        const auto & lhs = workspace.at(node.lhs);
        const auto & rhs = workspace.at(node.rhs);

        switch (node.op)
        {
          case TapeOp::Add:
            workspace.push_back(addValues(lhs, rhs));
            break;
          case TapeOp::Subtract:
            workspace.push_back(subtractValues(lhs, rhs));
            break;
          case TapeOp::Multiply:
            workspace.push_back(multiplyValues(lhs, rhs));
            break;
          case TapeOp::Divide:
            workspace.push_back(divideValues(lhs, rhs));
            break;
          case TapeOp::Power:
            workspace.push_back(powerValues(lhs, rhs));
            break;
          case TapeOp::Dot:
            workspace.push_back(dotValues(lhs, rhs));
            break;
          case TapeOp::Outer:
            workspace.push_back(outerValues(lhs, rhs));
            break;
          case TapeOp::Contract:
            workspace.push_back(contractValues(lhs, rhs));
            break;
          default:
            return std::nullopt;
        }

        break;
      }

      case TapeOp::Negate:
      {
        if (node.lhs == invalidIndex())
          return std::nullopt;
        const auto & val = workspace.at(node.lhs);
        workspace.push_back(negateValue(val));
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
