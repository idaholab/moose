#pragma once

#include "MooseAST.h"
#include "MooseValueTypes.h"
#include <string>
#include <vector>
#include <variant>
#include <optional>
#include <array>
#include <unordered_map>

namespace moose
{
namespace automatic_weak_form
{

using TapeScalar = Real;
using TapeVector = RealVectorValue;
using TapeRank2 = RankTwoTensor;
using TapeRank3 = RankThreeTensor;
using TapeRank4 = RankFourTensor;

using TapeValue = std::variant<TapeScalar, TapeVector, TapeRank2, TapeRank3, TapeRank4>;

enum class TapeOp
{
  LoadConstant,
  LoadInput,
  Add,
  Subtract,
  Multiply,
  Divide,
  Negate,
  Power
};

struct TapeNode
{
  TapeOp op;
  std::size_t lhs = static_cast<std::size_t>(-1);
  std::size_t rhs = static_cast<std::size_t>(-1);
  std::string label;
  TapeValue payload;
};

class EvaluationTape
{
public:
  std::size_t appendNode(const TapeNode & node);
  const TapeNode & node(std::size_t index) const { return _nodes.at(index); }
  std::size_t size() const { return _nodes.size(); }
  void clear() { _nodes.clear(); }

  std::optional<TapeValue>
  evaluate(const std::unordered_map<std::string, TapeValue> & inputs) const;

private:
  std::vector<TapeNode> _nodes;
};

struct TapeBuildResult
{
  EvaluationTape tape;
  std::vector<std::string> inputs;
  bool success = false;
};

TapeBuildResult buildScalarTape(const NodePtr & expr,
                                const std::string & differentiation_variable,
                                const std::vector<std::string> & additional_variables = {});

} // namespace automatic_weak_form
} // namespace moose
