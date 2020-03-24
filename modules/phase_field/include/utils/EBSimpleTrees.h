
#pragma once

#include "ExpressionBuilderToo.h"

struct SimplificationRules
{
  SimplificationRules(std::vector<std::vector<ExpressionBuilderToo::EBTerm>> rules)
  {
    for (auto rule : rules)
    {
      _simplification_trees.emplace_back(
          std::make_pair(rule[0].cloneRoot()->toNNary(), rule[1].cloneRoot()->toNNary()));
    }
  }
  operator std::vector<
      std::pair<ExpressionBuilderToo::EBTermNode *, ExpressionBuilderToo::EBTermNode *>>()
  {
    return _simplification_trees;
  }
  std::vector<std::pair<ExpressionBuilderToo::EBTermNode *, ExpressionBuilderToo::EBTermNode *>>
      _simplification_trees;
};

class EBSimpleTrees
{
public:
  EBSimpleTrees();
  EBSimpleTrees(bool);

  operator std::vector<
      std::pair<ExpressionBuilderToo::EBTermNode *, ExpressionBuilderToo::EBTermNode *>>()
  {
    return _simplification_trees;
  }

  std::vector<std::pair<ExpressionBuilderToo::EBTermNode *, ExpressionBuilderToo::EBTermNode *>>
      _simplification_trees;
};
