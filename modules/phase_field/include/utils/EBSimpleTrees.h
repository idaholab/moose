#pragma once

#include "ExpressionBuilderToo.h"

struct SimplificationRules
{
  SimplificationRules(std::vector<std::vector<ExpressionBuilderToo::EBTerm>> rules)
  {
    for (auto rule : rules)
    {
      std::vector<ExpressionBuilderToo::EBTermNode *> conditions(rule.size() - 2);
      for (unsigned int i = 0; i < rule.size() - 2; ++i)
        conditions[i] = rule[i + 2].cloneRoot();
      _simplification_trees.emplace_back(
          std::make_pair(rule[0].cloneRoot()->toNNary(), rule[1].cloneRoot()->toNNary()));
      _conditions.emplace_back(conditions);
    }
  }
  operator std::vector<
      std::pair<ExpressionBuilderToo::EBTermNode *, ExpressionBuilderToo::EBTermNode *>>()
  {
    return _simplification_trees;
  }
  operator ExpressionBuilderToo::SimpleRules()
  {
    return ExpressionBuilderToo::SimpleRules(_simplification_trees, _conditions);
  }

  std::vector<std::pair<ExpressionBuilderToo::EBTermNode *, ExpressionBuilderToo::EBTermNode *>>
      _simplification_trees;
  std::vector<std::vector<ExpressionBuilderToo::EBTermNode *>> _conditions;
};

ExpressionBuilderToo::SimpleRules getPrepRules();

ExpressionBuilderToo::SimpleRules getRules();
