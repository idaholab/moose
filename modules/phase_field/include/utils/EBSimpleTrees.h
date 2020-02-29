
#pragma once

#include "ExpressionBuilder.h"

struct SimplificationRules
{
  SimplificationRules(std::vector<std::vector<ExpressionBuilder::EBTerm>> rules)
  {
    for (auto rule : rules)
    {
      _simplification_trees.emplace_back(std::make_pair(rule[0].cloneRoot(), rule[1].cloneRoot()));
    }
  }
  operator std::vector<
      std::pair<ExpressionBuilder::EBTermNode *, ExpressionBuilder::EBTermNode *>>()
  {
    return _simplification_trees;
  }
  std::vector<std::pair<ExpressionBuilder::EBTermNode *, ExpressionBuilder::EBTermNode *>>
      _simplification_trees;
};

class PrepEBSimpleTrees
{
public:
  PrepEBSimpleTrees();

  operator std::vector<
      std::pair<ExpressionBuilder::EBTermNode *, ExpressionBuilder::EBTermNode *>>()
  {
    return _prep_simplification_trees;
  }

  std::vector<std::pair<ExpressionBuilder::EBTermNode *, ExpressionBuilder::EBTermNode *>>
      _prep_simplification_trees;
};

class EBSimpleTrees
{
public:
  EBSimpleTrees();

  operator std::vector<
      std::pair<ExpressionBuilder::EBTermNode *, ExpressionBuilder::EBTermNode *>>()
  {
    return _simplification_trees;
  }

  std::vector<std::pair<ExpressionBuilder::EBTermNode *, ExpressionBuilder::EBTermNode *>>
      _simplification_trees;
};
