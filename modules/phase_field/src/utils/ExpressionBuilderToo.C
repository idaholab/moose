//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExpressionBuilderToo.h"
#include "EBSimpleTrees.h"

std::vector<std::pair<ExpressionBuilderToo::EBTermNode *, ExpressionBuilderToo::EBTermNode *>>
    ExpressionBuilderToo::EBTerm::_prep_simplification_rules = getPrepRules();
std::vector<std::pair<ExpressionBuilderToo::EBTermNode *, ExpressionBuilderToo::EBTermNode *>>
    ExpressionBuilderToo::EBTerm::_simplification_rules = getRules();

/******************************************
 * The Beginning of the Node implementations
 * EBTermNode
 *******************************************/
bool
ExpressionBuilderToo::EBTermNode::compareRule(EBTermNode * rule,
                                              std::vector<EBTermNode *> & stars,
                                              std::vector<unsigned int> & current_index,
                                              std::vector<unsigned int> changed_indeces,
                                              bool checkNNaryLeaf)
{
  std::stack<EBTermNode *> self_stack;
  self_stack.emplace(this);
  std::stack<EBTermNode *> rule_stack;
  rule_stack.emplace(rule);
  std::vector<EBTermNode *> star_vec(stars);
  std::stack<Comparer *> permutations;
  bool isHead = true;
  bool next_permutation = false;
  while (rule_stack.size() > 0)
  {
    if (checkNNaryLeaf && rule_stack.size() != self_stack.size())
      return false;
    // Get the children of the top of the stack
    std::vector<EBTermNode *> self_children = self_stack.top()->getChildren();
    std::vector<EBTermNode *> rule_children = rule_stack.top()->getChildren();
    int permuteNumber = 0;
    // Check the nodes to make sure they match
    // If not, move to the next permutation
    if (!rule_stack.top()->compare(self_stack.top(), star_vec, isHead, permuteNumber) ||
        next_permutation)
    {
      if (permutations.size() != 0)
      {
        self_stack = permutations.top()->_self_stack;
        rule_stack = permutations.top()->_rule_stack;
        star_vec = permutations.top()->_stars;
        current_index = permutations.top()->_current_index;
        permutations.pop();
        next_permutation = false;
        continue;
      }
      else
        return false;
    }
    EBStarNode * checkLeaf = dynamic_cast<EBStarNode *>(rule_stack.top());
    EBRestNode * checkLeafTwo = dynamic_cast<EBRestNode *>(rule_stack.top());
    // We have 3 possibilities here.
    // The first is if we are at a leaf node.
    // The second is if we are at a commutative node.
    // The third is everything else.
    if (checkLeaf != NULL || checkLeafTwo != NULL)
    {
      rule_stack.pop();
      self_stack.pop();
    }
    else if (self_stack.top()->isCommutative())
    {
      EBNNaryOpTermNode * checkSelfNNary = dynamic_cast<EBNNaryOpTermNode *>(self_stack.top());
      EBNNaryOpTermNode * checkRuleNNary = dynamic_cast<EBNNaryOpTermNode *>(rule_stack.top());
      std::vector<std::vector<EBTermNode *>> permutedChildren(0);
      std::vector<std::vector<unsigned int>> nnary_index(0);
      if (checkSelfNNary != NULL && isHead && checkRuleNNary == NULL && !checkNNaryLeaf)
        permutedChildren = permuteChildren(self_children, nnary_index, 2, changed_indeces);
      else if (checkSelfNNary != NULL && isHead && checkRuleNNary != NULL && !checkNNaryLeaf)
        permutedChildren =
            permuteChildren(self_children, nnary_index, permuteNumber, changed_indeces);
      else
      {
        if (self_children.size() != rule_children.size())
        {
          if (checkRuleNNary->hasRest())
          {
            rule_stack.pop();
            for (unsigned int i = 0; i < rule_children.size(); ++i)
              rule_stack.emplace(rule_children[i]);
            self_stack.pop();
            self_stack.emplace(self_children[0]);
            self_stack.emplace(new EBNNaryOpTermNode(
                std::vector<EBTermNode *>(self_children.begin() + 1, self_children.end()),
                checkSelfNNary->_type));
            continue;
          }
          next_permutation = true;
          continue;
        }
        permutedChildren = permuteChildren(self_children, changed_indeces);
      }
      rule_stack.pop();
      for (unsigned int i = 0; i < rule_children.size(); ++i)
        rule_stack.emplace(rule_children[i]);

      for (unsigned int i = 0; i < permutedChildren.size(); ++i)
      {
        std::stack<EBTermNode *> temp_self_stack(self_stack);
        temp_self_stack.pop();
        for (unsigned int j = 0; j < permutedChildren[i].size(); ++j)
          temp_self_stack.emplace(permutedChildren[i][j]);
        if (nnary_index.size() > 0)
          current_index = nnary_index[i];
        Comparer * new_comparer =
            new Comparer(temp_self_stack, rule_stack, star_vec, current_index);

        permutations.emplace(new_comparer);
      }
      self_stack = permutations.top()->_self_stack;
      current_index = permutations.top()->_current_index;
      permutations.pop();
    }
    else
    {
      self_stack.pop();
      for (unsigned int i = 0; i < self_children.size(); ++i)
      {
        self_stack.emplace(self_children[i]);
      }

      rule_stack.pop();
      for (unsigned int i = 0; i < rule_children.size(); ++i)
        rule_stack.emplace(rule_children[i]);
    }
    isHead = false;
  }
  stars = star_vec;
  return true;
}

ExpressionBuilderToo::EBTermNode *
ExpressionBuilderToo::EBTermNode::simplify(std::pair<EBTermNode *, EBTermNode *> rule,
                                           bool & changed)
{
  std::vector<EBTermNode *> stars(27, NULL);
  std::vector<unsigned int> current_index(0);
  if (compareRule(rule.first->clone(), stars, current_index))
  {
    changed = true;
    EBTermNode * replacement = rule.second->clone();
    return replacement->replaceRule(stars);
  }
  return this;
}

std::vector<std::vector<ExpressionBuilderToo::EBTermNode *>>
ExpressionBuilderToo::EBTermNode::permuteChildren(std::vector<EBTermNode *> children,
                                                  std::vector<unsigned int> changed_indeces)
{
  std::vector<std::vector<EBTermNode *>> resulting;
  if (children.size() > 1)
    for (unsigned int i = 0; i < children.size(); ++i)
    {
      if (std::find(changed_indeces.begin(), changed_indeces.end(), i) != changed_indeces.end())
        continue;

      std::vector<EBTermNode *> to_recurse(children.begin(), children.begin() + i);
      to_recurse.insert(to_recurse.end(), children.begin() + i + 1, children.end());
      std::vector<std::vector<EBTermNode *>> next_recursion =
          permuteChildren(to_recurse, changed_indeces);
      for (unsigned int j = 0; j < next_recursion.size(); ++j)
      {
        std::vector<EBTermNode *> permutation(1, children[i]);
        permutation.insert(permutation.end(), next_recursion[j].begin(), next_recursion[j].end());
        resulting.emplace_back(permutation);
      }
    }
  else
    return std::vector<std::vector<EBTermNode *>>(1, children);
  return resulting;
}

std::vector<std::vector<ExpressionBuilderToo::EBTermNode *>>
ExpressionBuilderToo::EBTermNode::permuteChildren(
    std::vector<EBTermNode *> children,
    std::vector<std::vector<unsigned int>> & nnary_index,
    int max_depth,
    std::vector<unsigned int> changed_indeces,
    int depth,
    std::vector<unsigned int> current_index)
{
  std::vector<std::vector<EBTermNode *>> resulting;
  current_index.emplace_back(0);
  if (depth < max_depth - 1)
    for (unsigned int i = 0; i < children.size(); ++i)
    {
      if (std::find(changed_indeces.begin(), changed_indeces.end(), i) != changed_indeces.end())
        continue;
      current_index[depth] = i;
      for (unsigned int j = 0; j < current_index.size() - 1; ++j)
        if (current_index[j] <= current_index[depth])
          current_index[depth] += 1;
      std::vector<EBTermNode *> to_recurse(children.begin(), children.begin() + i);
      to_recurse.insert(to_recurse.end(), children.begin() + i + 1, children.end());
      std::vector<unsigned int> updated_changed_indeces = changed_indeces;
      for (unsigned int j = 0; j < updated_changed_indeces.size(); ++j)
        if (updated_changed_indeces[j] > i)
          updated_changed_indeces[j]--;
      std::vector<std::vector<EBTermNode *>> next_recursion = permuteChildren(
          to_recurse, nnary_index, max_depth, updated_changed_indeces, depth + 1, current_index);
      for (unsigned int j = 0; j < next_recursion.size(); ++j)
      {
        std::vector<EBTermNode *> permutation(1, children[i]);
        permutation.insert(permutation.end(), next_recursion[j].begin(), next_recursion[j].end());
        resulting.emplace_back(permutation);
      }
    }
  else
    for (unsigned int i = 0; i < children.size(); ++i)
    {
      if (std::find(changed_indeces.begin(), changed_indeces.end(), i) != changed_indeces.end())
        continue;
      current_index[depth] = i;
      for (unsigned int j = 0; j < current_index.size() - 1; ++j)
        if (current_index[j] <= current_index[depth])
          current_index[depth] += 1;
      nnary_index.emplace_back(current_index);
      resulting.emplace_back(std::vector<EBTermNode *>(1, children[i]));
    }
  return resulting;
}

ExpressionBuilderToo::EBTermNode *
ExpressionBuilderToo::EBTermNode::substitute(
    std::vector<std::pair<EBTermNode *, EBTermNode *>> subs)
{
  std::vector<EBTermNode *> empty_nodes(0);
  std::vector<unsigned int> empty_ints(0);
  for (unsigned int i = 0; i < subs.size(); ++i)
    if (this->compareRule(subs[i].first, empty_nodes, empty_ints))
      return subs[i].second->clone();
  return this;
}

/*******************************
 * EBSymbolNode
 ********************************/

std::string
ExpressionBuilderToo::EBSymbolNode::stringify() const
{
  return _symbol;
}

/*******************************
 * EBTempIDNode
 ********************************/

std::string
ExpressionBuilderToo::EBTempIDNode::stringify() const
{
  std::ostringstream s;
  s << '[' << _id << ']';
  return s.str();
}

/*******************************
 * EBUnaryTermNode
 ********************************/

ExpressionBuilderToo::EBTermNode *
ExpressionBuilderToo::EBUnaryTermNode::substitute(
    std::vector<std::pair<EBTermNode *, EBTermNode *>> subs)
{
  std::vector<EBTermNode *> empty_nodes(0);
  std::vector<unsigned int> empty_ints(0);
  for (unsigned int i = 0; i < subs.size(); ++i)
    if (this->compareRule(subs[i].first, empty_nodes, empty_ints))
      return subs[i].second->clone();
  _subnode = _subnode->substitute(subs);
  return this;
}

/*******************************
 * EBUnaryFuncTermNode
 ********************************/

std::string
ExpressionBuilderToo::EBUnaryFuncTermNode::stringify() const
{
  const char * name[] = {"sin", "cos", "tan", "abs", "log", "log2", "log10", "exp", "sinh", "cosh"};
  std::ostringstream s;
  s << name[_type] << '(' << *_subnode << ')';
  return s.str();
}

Real
ExpressionBuilderToo::EBUnaryFuncTermNode::getRealValue()
{
  switch (_type)
  {
    case SIN:
      return sin(_subnode->getRealValue());
    case COS:
      return cos(_subnode->getRealValue());
    case TAN:
      return tan(_subnode->getRealValue());
    case ABS:
      return abs(_subnode->getRealValue());
    case LOG:
      return log(_subnode->getRealValue());
    case LOG2:
      return log2(_subnode->getRealValue());
    case LOG10:
      return log10(_subnode->getRealValue());
    case EXP:
      return exp(_subnode->getRealValue());
    case SINH:
      return sinh(_subnode->getRealValue());
    case COSH:
      return cosh(_subnode->getRealValue());
    default:
      mooseError("Undefined type");
  }
}

ExpressionBuilderToo::EBTermNode *
ExpressionBuilderToo::EBUnaryFuncTermNode::derivative(const std::string & derivative_comp)
{
  EBTermNode * deriv_node = NULL;
  EBTermNode * new_left;
  EBTermNode * new_right;
  EBTermNode * cos_left;
  EBTermNode * pow_left;
  EBTermNode * cond_left;
  EBTermNode * log_left;
  EBTermNode * mult_left;
  switch (_type)
  {
    case SIN:
      new_left = new EBUnaryFuncTermNode(_subnode->clone(), EBUnaryFuncTermNode::NodeType::COS);
      new_right = _subnode->derivative(derivative_comp);
      deriv_node = new EBBinaryOpTermNode(new_left, new_right, EBBinaryOpTermNode::NodeType::MUL);
      break;
    case COS:
      new_left = new EBUnaryFuncTermNode(_subnode->clone(), EBUnaryFuncTermNode::NodeType::COS);
      new_right = new EBUnaryOpTermNode(_subnode->derivative(derivative_comp),
                                        EBUnaryOpTermNode::NodeType::NEG);
      deriv_node = new EBBinaryOpTermNode(new_left, new_right, EBBinaryOpTermNode::NodeType::MUL);
      break;
    case TAN:
      cos_left = new EBUnaryFuncTermNode(_subnode->clone(), EBUnaryFuncTermNode::NodeType::COS);
      pow_left = new EBBinaryOpTermNode(
          cos_left, new EBNumberNode<Real>(2), EBBinaryOpTermNode::NodeType::POW);
      new_left = new EBBinaryOpTermNode(
          new EBNumberNode<Real>(1), pow_left, EBBinaryOpTermNode::NodeType::DIV);
      new_right = _subnode->derivative(derivative_comp);
      deriv_node = new EBBinaryOpTermNode(new_left, new_right, EBBinaryOpTermNode::NodeType::MUL);
      break;
    case ABS:
      cond_left = new EBBinaryOpTermNode(
          _subnode->clone(), new EBNumberNode<Real>(0), EBBinaryOpTermNode::NodeType::GREATER);
      new_left = new EBTernaryFuncTermNode(cond_left,
                                           new EBNumberNode<Real>(1),
                                           new EBNumberNode<Real>(-1),
                                           EBTernaryFuncTermNode::NodeType::CONDITIONAL);
      new_right = _subnode->derivative(derivative_comp);
      deriv_node = new EBBinaryOpTermNode(new_left, new_right, EBBinaryOpTermNode::NodeType::MUL);
      break;
    case LOG:
      new_left = new EBBinaryOpTermNode(
          new EBNumberNode<Real>(1), _subnode->clone(), EBBinaryOpTermNode::NodeType::DIV);
      new_right = _subnode->derivative(derivative_comp);
      deriv_node = new EBBinaryOpTermNode(new_left, new_right, EBBinaryOpTermNode::NodeType::MUL);
      break;
    case LOG2:
      log_left =
          new EBUnaryFuncTermNode(new EBNumberNode<Real>(2), EBUnaryFuncTermNode::NodeType::LOG);
      mult_left =
          new EBBinaryOpTermNode(_subnode->clone(), log_left, EBBinaryOpTermNode::NodeType::MUL);
      new_left = new EBBinaryOpTermNode(
          new EBNumberNode<Real>(1), mult_left, EBBinaryOpTermNode::NodeType::DIV);
      new_right = _subnode->derivative(derivative_comp);
      deriv_node = new EBBinaryOpTermNode(new_left, new_right, EBBinaryOpTermNode::NodeType::MUL);
      break;
    case LOG10:
      log_left =
          new EBUnaryFuncTermNode(new EBNumberNode<Real>(10), EBUnaryFuncTermNode::NodeType::LOG);
      mult_left =
          new EBBinaryOpTermNode(_subnode->clone(), log_left, EBBinaryOpTermNode::NodeType::MUL);
      new_left = new EBBinaryOpTermNode(
          new EBNumberNode<Real>(1), mult_left, EBBinaryOpTermNode::NodeType::DIV);
      new_right = _subnode->derivative(derivative_comp);
      deriv_node = new EBBinaryOpTermNode(new_left, new_right, EBBinaryOpTermNode::NodeType::MUL);
      break;
    case EXP:
      new_left = clone();
      new_right = _subnode->derivative(derivative_comp);
      deriv_node = new EBBinaryOpTermNode(new_left, new_right, EBBinaryOpTermNode::NodeType::MUL);
      break;
    case SINH:
      new_left = new EBUnaryFuncTermNode(_subnode->clone(), EBUnaryFuncTermNode::NodeType::COSH);
      new_right = _subnode->derivative(derivative_comp);
      deriv_node = new EBBinaryOpTermNode(new_left, new_right, EBBinaryOpTermNode::NodeType::MUL);
      break;
    case COSH:
      new_left = new EBUnaryFuncTermNode(_subnode->clone(), EBUnaryFuncTermNode::NodeType::SINH);
      new_right = _subnode->derivative(derivative_comp);
      deriv_node = new EBBinaryOpTermNode(new_left, new_right, EBBinaryOpTermNode::NodeType::MUL);
      break;
  }
  return deriv_node;
}

/*******************************
 * EBUnaryOpTermNode
 ********************************/

std::string
ExpressionBuilderToo::EBUnaryOpTermNode::stringify() const
{
  const char * name[] = {"-", "!"};
  std::ostringstream s;

  s << name[_type];

  if (_subnode->precedence() > precedence())
    s << '(' << *_subnode << ')';
  else
    s << *_subnode;

  return s.str();
}

Real
ExpressionBuilderToo::EBUnaryOpTermNode::getRealValue()
{
  switch (_type)
  {
    case NEG:
      return -_subnode->getRealValue();
    case LOGICNOT:
      return !_subnode->getRealValue();
    default:
      mooseError("Unknown type");
  }
}

ExpressionBuilderToo::EBTermNode *
ExpressionBuilderToo::EBUnaryOpTermNode::derivative(const std::string & derivative_comp)
{
  EBTermNode * deriv_node = NULL;
  switch (_type)
  {
    case NEG:
      deriv_node = new EBUnaryOpTermNode(_subnode->derivative(derivative_comp),
                                         EBUnaryOpTermNode::NodeType::NEG);
      break;
    case LOGICNOT:
      mooseError("Cannot take derivative of logical operator.");
  }
  return deriv_node;
}

/*******************************
 * EBBinaryTermNode
 ********************************/

ExpressionBuilderToo::EBTermNode *
ExpressionBuilderToo::EBBinaryTermNode::substitute(
    std::vector<std::pair<EBTermNode *, EBTermNode *>> subs)
{
  std::vector<EBTermNode *> empty_nodes(0);
  std::vector<unsigned int> empty_ints(0);
  for (unsigned int i = 0; i < subs.size(); ++i)
    if (this->compareRule(subs[i].first, empty_nodes, empty_ints))
      return subs[i].second->clone();
  _left = _left->substitute(subs);
  _right = _right->substitute(subs);
  return this;
}

/*******************************
 * EBBinaryFuncTermNode
 ********************************/

std::string
ExpressionBuilderToo::EBBinaryFuncTermNode::stringify() const
{
  const char * name[] = {"min", "max", "atan2", "hypot", "plog"};
  std::ostringstream s;
  s << name[_type] << '(' << *_left << ',' << *_right << ')';
  return s.str();
}

bool
ExpressionBuilderToo::EBBinaryFuncTermNode::isCommutative() const
{
  switch (_type)
  {
    case MIN:
    case MAX:
    case HYPOT:
      return true;
    case ATAN2:
    case PLOG:
      return false;
  }
  mooseError("Unknown type.");
}

Real
ExpressionBuilderToo::EBBinaryFuncTermNode::getRealValue()
{
  switch (_type)
  {
    case MIN:
      return std::min(_left->getRealValue(), _right->getRealValue());
    case MAX:
      return std::min(_left->getRealValue(), _right->getRealValue());
    case ATAN2:
      mooseError("Not Ready Yet");
    case HYPOT:
      mooseError("Not Ready Yet");
    case PLOG:
      mooseError("Not Ready Yet");
    default:
      mooseError("Unknown type");
  }
}

ExpressionBuilderToo::EBTermNode *
ExpressionBuilderToo::EBBinaryFuncTermNode::derivative(const std::string &)
{
  mooseError("Derivative not yet defined for Binary Functions yet");
}

/*******************************
 * EBBinaryOpTermNode
 ********************************/

std::string
ExpressionBuilderToo::EBBinaryOpTermNode::stringify() const
{
  const char * name[] = {"+", "-", "*", "/", "%", "^", "<", ">", "<=", ">=", "=", "!="};
  std::ostringstream s;

  if (_left->precedence() > precedence())
    s << '(' << *_left << ')';
  else
    s << *_left;

  s << name[_type];

  // these operators are left associative at equal precedence
  // (this matters for -,/,&,^ but not for + and *)
  if (_right->precedence() > precedence() ||
      (_right->precedence() == precedence() &&
       (_type == SUB || _type == DIV || _type == MOD || _type == POW)))
    s << '(' << *_right << ')';
  else
    s << *_right;

  return s.str();
}

int
ExpressionBuilderToo::EBBinaryOpTermNode::precedence() const
{
  switch (_type)
  {
    case ADD:
    case SUB:
      return 6;
    case MUL:
    case DIV:
    case MOD:
      return 5;
    case POW:
      return 2;
    case LESS:
    case GREATER:
    case LESSEQ:
    case GREATEREQ:
      return 8;
    case EQ:
    case NOTEQ:
      return 9;
  }

  mooseError("Unknown type.");
}

bool
ExpressionBuilderToo::EBBinaryOpTermNode::isCommutative() const
{
  switch (_type)
  {
    case ADD:
    case MUL:
    case EQ:
    case NOTEQ:
      return true;
    case SUB:
    case DIV:
    case MOD:
    case POW:
    case LESS:
    case GREATER:
    case LESSEQ:
    case GREATEREQ:
      return false;
  }
  mooseError("Unknown type.");
}

bool
ExpressionBuilderToo::EBBinaryOpTermNode::compare(EBTermNode * compare_to,
                                                  std::vector<EBTermNode *> &,
                                                  bool isHead,
                                                  int &)
{
  EBBinaryOpTermNode * c_to = dynamic_cast<EBBinaryOpTermNode *>(compare_to);

  if (c_to != NULL && c_to->_type == _type)
    return true;
  EBNNaryOpTermNode * c_to_nnary = dynamic_cast<EBNNaryOpTermNode *>(compare_to);
  if (c_to_nnary != NULL &&
      ((c_to_nnary->_type == EBNNaryOpTermNode::NodeType::MUL && _type == MUL) ||
       (c_to_nnary->_type == EBNNaryOpTermNode::NodeType::ADD && _type == ADD)))
  {
    if (c_to_nnary->childrenSize() == 2)
      return true;
    if (isHead == true)
      return true;
  }
  return false;
}

ExpressionBuilderToo::EBTermNode *
ExpressionBuilderToo::EBBinaryOpTermNode::toNNary()
{
  // Possibly change this to add all commutative operators
  if (_type == MUL || _type == ADD)
  {
    std::vector<EBTermNode *> new_children(2);
    new_children[0] = _right;
    new_children[1] = _left;
    EBNNaryOpTermNode * new_node;
    if (_type == ADD)
      new_node = new EBNNaryOpTermNode(new_children, EBNNaryOpTermNode::NodeType::ADD);
    else
      new_node = new EBNNaryOpTermNode(new_children, EBNNaryOpTermNode::NodeType::MUL);
    return new_node->toNNary();
  }
  else
    return EBBinaryTermNode::toNNary();
  return this;
}

Real
ExpressionBuilderToo::EBBinaryOpTermNode::getRealValue()
{
  switch (_type)
  {
    case ADD:
      return _left->getRealValue() + _right->getRealValue();
    case SUB:
      return _left->getRealValue() - _right->getRealValue();
    case MUL:
      return _left->getRealValue() * _right->getRealValue();
    case DIV:
      return _left->getRealValue() / _right->getRealValue();
    case MOD:
      return (int)round(_left->getRealValue()) % (int)round(_right->getRealValue());
    case POW:
      return pow(_left->getRealValue(), _right->getRealValue());
    case LESS:
      if (_left->getRealValue() < _right->getRealValue())
        return 1;
      return 0;
    case GREATER:
      if (_left->getRealValue() > _right->getRealValue())
        return 1;
      return 0;
    case LESSEQ:
      if (_left->getRealValue() <= _right->getRealValue())
        return 1;
      return 0;
    case GREATEREQ:
      if (_left->getRealValue() >= _right->getRealValue())
        return 1;
      return 0;
    case EQ:
      if (abs(_left->getRealValue() - _right->getRealValue()) == 0)
        return 1;
      return 0;
    case NOTEQ:
      if (abs(_left->getRealValue() - _right->getRealValue()) != 0)
        return 1;
      return 0;
    default:
      mooseError("Unknown type");
  }
}

ExpressionBuilderToo::EBTermNode *
ExpressionBuilderToo::EBBinaryOpTermNode::derivative(const std::string & derivative_comp)
{
  EBTermNode * deriv_node = NULL;
  EBTermNode * left_node;
  EBTermNode * right_node;
  EBTermNode * pow_node;
  EBTermNode * mult_node;
  EBTermNode * new_pow;
  switch (_type)
  {
    case ADD:
    case SUB:
      deriv_node = new EBBinaryOpTermNode(
          _left->derivative(derivative_comp), _right->derivative(derivative_comp), _type);
      break;
    case MUL:
      left_node =
          new EBBinaryOpTermNode(_left->clone(), _right->derivative(derivative_comp), _type);
      right_node =
          new EBBinaryOpTermNode(_left->derivative(derivative_comp), _right->clone(), _type);
      deriv_node = new EBBinaryOpTermNode(left_node, right_node, EBBinaryOpTermNode::NodeType::ADD);
      break;
    case DIV:
      pow_node = new EBBinaryOpTermNode(
          _right->clone(), new EBNumberNode<Real>(-1), EBBinaryOpTermNode::NodeType::POW);
      mult_node =
          new EBBinaryOpTermNode(_left->clone(), pow_node, EBBinaryOpTermNode::NodeType::MUL);
      deriv_node = mult_node->derivative(derivative_comp);
      break;
    case POW:
      new_pow = new EBBinaryOpTermNode(
          _right->clone(), new EBNumberNode<Real>(1), EBBinaryOpTermNode::NodeType::SUB);
      pow_node = new EBBinaryOpTermNode(_left->clone(), new_pow, EBBinaryOpTermNode::NodeType::POW);
      deriv_node =
          new EBBinaryOpTermNode(_right->clone(), pow_node, EBBinaryOpTermNode::NodeType::MUL);
      break;
    case MOD:
    case LESS:
    case GREATER:
    case LESSEQ:
    case GREATEREQ:
    case EQ:
    case NOTEQ:
      mooseError("Derivative of logical and mod operators not defined");
      break;
  }
  return deriv_node;
}

/*******************************
 * EBTernaryTermNode
 ********************************/

ExpressionBuilderToo::EBTermNode *
ExpressionBuilderToo::EBTernaryTermNode::substitute(
    std::vector<std::pair<EBTermNode *, EBTermNode *>> subs)
{
  std::vector<EBTermNode *> empty_nodes(0);
  std::vector<unsigned int> empty_ints(0);
  for (unsigned int i = 0; i < subs.size(); ++i)
    if (this->compareRule(subs[i].first, empty_nodes, empty_ints))
      return subs[i].second->clone();
  _left = _left->substitute(subs);
  _middle = _middle->substitute(subs);
  _right = _right->substitute(subs);
  return this;
}

/*******************************
 * EBTernaryFuncTermNode
 ********************************/

std::string
ExpressionBuilderToo::EBTernaryFuncTermNode::stringify() const
{
  const char * name[] = {"if"};
  std::ostringstream s;
  s << name[_type] << '(' << *_left << ',' << *_middle << ',' << *_right << ')';
  return s.str();
}

Real
ExpressionBuilderToo::EBTernaryFuncTermNode::getRealValue()
{
  switch (_type)
  {
    case CONDITIONAL:
      if (_left->getRealValue())
        return _middle->getRealValue();
      return _right->getRealValue();
    default:
      mooseError("Unknown type");
  }
}

ExpressionBuilderToo::EBTermNode *
ExpressionBuilderToo::EBTernaryFuncTermNode::derivative(const std::string & derivative_comp)
{
  EBTermNode * deriv_node = NULL;
  switch (_type)
  {
    case CONDITIONAL:
      deriv_node = new EBTernaryFuncTermNode(_left->clone(),
                                             _middle->derivative(derivative_comp),
                                             _right->derivative(derivative_comp),
                                             _type);
      break;
  }
  return deriv_node;
}

/*******************************
 * EBNNaryOpTermNode
 ********************************/

std::string
ExpressionBuilderToo::EBNNaryOpTermNode::stringify() const
{
  const char * name[] = {"*", "+"};
  std::ostringstream s;

  for (unsigned int i = 0; i < _children.size(); ++i)
  {
    if (_children[i]->precedence() > precedence())
      s << '(' << *_children[i] << ')';
    else
      s << *_children[i];
    s << name[_type];
  }

  std::string result = s.str();
  result.pop_back();

  return result;
}

int
ExpressionBuilderToo::EBNNaryOpTermNode::precedence() const
{
  switch (_type)
  {
    case ADD:
      return 6;
    case MUL:
      return 5;
  }

  mooseError("Unknown type.");
}

bool
ExpressionBuilderToo::EBNNaryOpTermNode::compare(EBTermNode * compare_to,
                                                 std::vector<EBTermNode *> &,
                                                 bool isHead,
                                                 int & depth)
{
  EBNNaryOpTermNode * c_to_nnary = dynamic_cast<EBNNaryOpTermNode *>(compare_to);
  EBBinaryOpTermNode * c_to_bin = dynamic_cast<EBBinaryOpTermNode *>(compare_to);
  if (c_to_nnary != NULL && c_to_nnary->_type == _type)
  {
    if (_children.size() < c_to_nnary->childrenSize() && isHead)
    {
      depth = _children.size();
      return true;
    }

    if (_children.size() == c_to_nnary->childrenSize())
    {
      depth = _children.size();
      return true;
    }

    if (_children.size() < c_to_nnary->childrenSize() && hasRest())
      return true;
  }

  if (c_to_bin != NULL && ((c_to_bin->_type == EBBinaryOpTermNode::NodeType::MUL && _type == MUL) ||
                           (c_to_bin->_type == EBBinaryOpTermNode::NodeType::ADD && _type == ADD)))
  {
    if (_children.size() == 2)
      return true;
  }
  return false;
}

ExpressionBuilderToo::EBTermNode *
ExpressionBuilderToo::EBNNaryOpTermNode::simplify(std::pair<EBTermNode *, EBTermNode *> rule,
                                                  bool & changed)
{
  for (unsigned int i = 0; i < _children.size(); ++i)
    _children[i] = _children[i]->simplify(rule, changed);
  std::vector<EBTermNode *> stars;
  std::vector<unsigned int> current_index;
  bool nested_changed = true;
  while (nested_changed)
  {
    nested_changed = false;
    stars = std::vector<EBTermNode *>(27, NULL);
    current_index = std::vector<unsigned int>(0);
    if (compareRule(rule.first->clone(), stars, current_index))
    {
      nested_changed = true;
      changed = true;
      EBTermNode * replacement = rule.second->clone();
      if (current_index.size() != 0)
      {
        std::sort(current_index.begin(), current_index.end());
        for (int i = current_index.size() - 1; i >= 0; --i)
          _children.erase(_children.begin() + current_index[i]);
        EBBinaryOpTermNode * replac_bin = dynamic_cast<EBBinaryOpTermNode *>(replacement);
        EBNNaryOpTermNode * replac_nnary = dynamic_cast<EBNNaryOpTermNode *>(replacement);

        if (replac_bin != NULL &&
            ((_type == MUL && replac_bin->_type == EBBinaryOpTermNode::NodeType::MUL) ||
             (_type == ADD && replac_bin->_type == EBBinaryOpTermNode::NodeType::ADD)))
        {
          _children.emplace_back(replac_bin->getRight()->replaceRule(stars));
          _children.emplace_back(replac_bin->getLeft()->replaceRule(stars));
        }
        else if (replac_nnary != NULL && replac_nnary->_type == _type)
        {
          std::vector<EBTermNode *> new_children = replac_nnary->getChildren();
          for (unsigned int i = 0; i < new_children.size(); ++i)
            _children.emplace_back(new_children[i]->replaceRule(stars));
        }
        else
          _children.emplace_back(replacement->replaceRule(stars));
      }
      else
      {
        _children = replacement->getChildren();
      }
    }
    if (_children.size() == 1)
      return _children[0];
  }
  return this;
}

ExpressionBuilderToo::EBTermNode *
ExpressionBuilderToo::EBNNaryOpTermNode::substitute(
    std::vector<std::pair<EBTermNode *, EBTermNode *>> subs)
{
  std::vector<EBTermNode *> empty_nodes(0);
  std::vector<unsigned int> current_index(0);
  std::vector<unsigned int> changed_indeces(0);

  for (unsigned int k = 0; k < subs.size(); ++k)
  {
    std::pair<EBTermNode *, EBTermNode *> rule = subs[k];
    bool changed = true;
    while (changed)
    {
      changed = false;
      if (compareRule(rule.first->clone(), empty_nodes, current_index, changed_indeces))
      {
        changed = true;
        EBTermNode * replacement = rule.second->clone();
        if (current_index.size() != 0)
        {
          EBBinaryOpTermNode * replac_bin = dynamic_cast<EBBinaryOpTermNode *>(replacement);
          EBNNaryOpTermNode * replac_nnary = dynamic_cast<EBNNaryOpTermNode *>(replacement);

          if (replac_bin != NULL &&
              ((_type == MUL && replac_bin->_type == EBBinaryOpTermNode::NodeType::MUL) ||
               (_type == ADD && replac_bin->_type == EBBinaryOpTermNode::NodeType::ADD)))
          {
            _children[current_index[0]] = replac_bin->getRight()->replaceRule(empty_nodes);
            _children[current_index[1]] = replac_bin->getLeft()->replaceRule(empty_nodes);
            changed_indeces.emplace_back(current_index[0]);
            changed_indeces.emplace_back(current_index[1]);
          }
          else if (replac_nnary != NULL)
          {
            std::sort(current_index.begin(), current_index.end());
            for (int i = current_index.size() - 1; i >= 0; --i)
            {
              _children.erase(_children.begin() + current_index[i]);
              for (unsigned int j = 0; j < changed_indeces.size(); ++j)
                if (changed_indeces[j] > current_index[i])
                  changed_indeces[j] -= 1;
            }
            std::vector<EBTermNode *> new_children = replac_nnary->getChildren();
            for (unsigned int i = 0; i < new_children.size(); ++i)
            {
              _children.emplace_back(new_children[i]);
              changed_indeces.emplace_back(_children.size() - 1);
            }
          }
          else
          {
            _children[current_index[0]] = replacement->replaceRule(empty_nodes);
            _children.erase(_children.begin() + current_index[1]);
            changed_indeces.emplace_back(current_index[0]);
            for (unsigned int i = 0; i < current_index.size(); ++i)
              if (changed_indeces[i] > current_index[1])
                changed_indeces[i] -= 1;
          }
        }
        else
        {
          _children = replacement->getChildren();
          return this;
        }
      }
      if (_children.size() == 1)
        return _children[0];
    }
  }

  for (unsigned int i = 0; i < _children.size(); ++i)
    if (std::find(changed_indeces.begin(), changed_indeces.end(), i) != changed_indeces.end())
      _children[i] = _children[i]->substitute(subs);
  return this;
}

ExpressionBuilderToo::EBTermNode *
ExpressionBuilderToo::EBNNaryOpTermNode::toNNary()
{
  unsigned int i = 0;
  while (i < _children.size())
  {
    EBTermNode * new_child = _children[i]->toNNary();
    EBNNaryOpTermNode * new_nnary = dynamic_cast<EBNNaryOpTermNode *>(new_child);
    if (new_nnary != NULL && _type == new_nnary->_type)
    {
      std::vector<EBTermNode *> new_children = new_nnary->getChildren();
      _children[i] = new_children[0];
      _children.insert(_children.end(), new_children.begin() + 1, new_children.end());
    }
    else
    {
      _children[i] = new_child;
      i++;
    }
  }
  return this;
}

ExpressionBuilderToo::EBTermNode *
ExpressionBuilderToo::EBNNaryOpTermNode::constantFolding()
{
  Real new_num;
  switch (_type)
  {
    case ADD:
      new_num = 0;
      break;
    case MUL:
      new_num = 1;
      break;
    default:
      mooseError("Unknown type");
  }
  std::vector<int> changed_indeces;
  for (unsigned int i = 0; i < _children.size(); ++i)
  {
    if (_children[i]->isConstant())
    {
      switch (_type)
      {
        case ADD:
          new_num += _children[i]->getRealValue();
          break;
        case MUL:
          new_num *= _children[i]->getRealValue();
          break;
        default:
          mooseError("Unknown type");
      }
      changed_indeces.emplace_back(i);
    }
  }
  if (changed_indeces.size() == _children.size())
    return new EBNumberNode<Real>(new_num);
  unsigned int vec_size = changed_indeces.size();
  if (vec_size > 0)
    for (unsigned int i = 0; i < vec_size; ++i)
      _children.erase(_children.begin() + changed_indeces[vec_size - i - 1]);
  if (changed_indeces.size() != 0)
    _children.emplace_back(new EBNumberNode<Real>(new_num));

  for (unsigned int i = 0; i < _children.size(); ++i)
    _children[i] = _children[i]->constantFolding();
  return this;
}

Real
ExpressionBuilderToo::EBNNaryOpTermNode::getRealValue()
{
  Real new_num;
  switch (_type)
  {
    case ADD:
      new_num = 0;
      break;
    case MUL:
      new_num = 1;
      break;
    default:
      mooseError("Unknown type");
  }
  for (unsigned int i = 0; i < _children.size(); ++i)
  {
    switch (_type)
    {
      case ADD:
        new_num += _children[i]->getRealValue();
        break;
      case MUL:
        new_num *= _children[i]->getRealValue();
        break;
      default:
        mooseError("Unknown type");
    }
  }
  return new_num;
}

ExpressionBuilderToo::EBTermNode *
ExpressionBuilderToo::EBNNaryOpTermNode::derivative(const std::string & derivative_comp)
{
  EBTermNode * deriv_node = NULL;
  std::vector<EBTermNode *> new_children(_children.size());
  switch (_type)
  {
    case ADD:
      for (unsigned int i = 0; i < _children.size(); ++i)
        new_children[i] = _children[i]->derivative(derivative_comp);
      deriv_node = new EBNNaryOpTermNode(new_children, ADD);
      break;
    case MUL:
      std::vector<EBTermNode *> sub_children(_children.size());
      for (unsigned int i = 0; i < new_children.size(); ++i)
      {
        for (unsigned int j = 0; j < sub_children.size(); ++j)
          if (i == j)
            sub_children[j] = _children[j]->derivative(derivative_comp);
          else
            sub_children[j] = _children[j];
        new_children[i] = new EBNNaryOpTermNode(sub_children, MUL);
      }
      deriv_node = new EBNNaryOpTermNode(new_children, ADD);
      break;
  }
  return deriv_node;
}

/*******************************
 * End of Node implementations
 * EBTerm & Related Functionality
 ********************************/

void
ExpressionBuilderToo::EBTerm::simplify()
{
  bool changed;
  for (auto rule : _prep_simplification_rules)
  {
    changed = true;
    while (changed == true)
    {
      changed = false;
      _root = _root->simplify(rule, changed);
    }
  }

  bool outer = true;
  while (outer)
  {
    outer = false;
    _root = _root->toNNary();
    _root = _root->constantFolding();

    for (auto rule : _simplification_rules)
    {
      changed = true;
      while (changed == true)
      {
        changed = false;
        _root = _root->simplify(rule, changed);
        if (changed)
          outer = true;
        _root = _root->toNNary();
      }
    }
  }
}

void
ExpressionBuilderToo::EBTerm::substitute(std::vector<std::vector<EBTerm>> subs)
{
  _root = _root->substitute(SimplificationRules(subs));
}

ExpressionBuilderToo::EBTermList operator,(const ExpressionBuilderToo::EBTerm & larg,
                                          const ExpressionBuilderToo::EBTerm & rarg)
{
  return {larg, rarg};
}

ExpressionBuilderToo::EBTermList operator,(const ExpressionBuilderToo::EBTerm & larg,
                                          const ExpressionBuilderToo::EBTermList & rargs)
{
  ExpressionBuilderToo::EBTermList list = {larg};
  list.insert(list.end(), rargs.begin(), rargs.end());
  return list;
}

ExpressionBuilderToo::EBTermList operator,(const ExpressionBuilderToo::EBTermList & largs,
                                          const ExpressionBuilderToo::EBTerm & rarg)
{
  ExpressionBuilderToo::EBTermList list = largs;
  list.push_back(rarg);
  return list;
}

ExpressionBuilderToo::EBTerm &
ExpressionBuilderToo::EBTerm::operator()(const EBTerm & term)
{
  _eval_arguments = EBTermList(1, term);
  return *this;
}

ExpressionBuilderToo::EBTerm &
ExpressionBuilderToo::EBTerm::operator()(EBTermList term_list)
{
  _eval_arguments = term_list;
  return *this;
}

std::string
ExpressionBuilderToo::EBTerm::args()
{
  unsigned int narg = _arguments.size();
  if (narg < 1)
    return "";

  std::ostringstream s;
  s << _arguments[0];
  for (unsigned int i = 1; i < narg; ++i)
    s << ',' << _arguments[i];

  return s.str();
}

std::ostream &
operator<<(std::ostream & os, const ExpressionBuilderToo::EBTerm & term)
{
  if (term._root != NULL)
    return os << *term._root;
  else
    return os << "[NULL]";
}

#define UNARY_FUNC_IMPLEMENT(op, OP)                                                               \
  ExpressionBuilderToo::EBTerm op(const ExpressionBuilderToo::EBTerm & term)                       \
  {                                                                                                \
    mooseAssert(term._root != NULL, "Empty term provided as argument of function " #op "()");      \
    return ExpressionBuilderToo::EBTerm(new ExpressionBuilderToo::EBUnaryFuncTermNode(             \
        term.cloneRoot(), ExpressionBuilderToo::EBUnaryFuncTermNode::OP));                         \
  }
UNARY_FUNC_IMPLEMENT(sin, SIN)
UNARY_FUNC_IMPLEMENT(cos, COS)
UNARY_FUNC_IMPLEMENT(tan, TAN)
UNARY_FUNC_IMPLEMENT(abs, ABS)
UNARY_FUNC_IMPLEMENT(log, LOG)
UNARY_FUNC_IMPLEMENT(log2, LOG2)
UNARY_FUNC_IMPLEMENT(log10, LOG10)
UNARY_FUNC_IMPLEMENT(exp, EXP)
UNARY_FUNC_IMPLEMENT(sinh, SINH)
UNARY_FUNC_IMPLEMENT(cosh, COSH)

#define BINARY_FUNC_IMPLEMENT(op, OP)                                                              \
  ExpressionBuilderToo::EBTerm op(const ExpressionBuilderToo::EBTerm & left,                       \
                                  const ExpressionBuilderToo::EBTerm & right)                      \
  {                                                                                                \
    mooseAssert(left._root != NULL,                                                                \
                "Empty term provided as first argument of function " #op "()");                    \
    mooseAssert(right._root != NULL,                                                               \
                "Empty term provided as second argument of function " #op "()");                   \
    return ExpressionBuilderToo::EBTerm(new ExpressionBuilderToo::EBBinaryFuncTermNode(            \
        left.cloneRoot(), right.cloneRoot(), ExpressionBuilderToo::EBBinaryFuncTermNode::OP));     \
  }
BINARY_FUNC_IMPLEMENT(min, MIN)
BINARY_FUNC_IMPLEMENT(max, MAX)
BINARY_FUNC_IMPLEMENT(atan2, ATAN2)
BINARY_FUNC_IMPLEMENT(hypot, HYPOT)
BINARY_FUNC_IMPLEMENT(plog, PLOG)

// this is a function in ExpressionBuilderToo (pow) but an operator in FParser (^)
ExpressionBuilderToo::EBTerm
pow(const ExpressionBuilderToo::EBTerm & left, const ExpressionBuilderToo::EBTerm & right)
{
  mooseAssert(left._root != NULL, "Empty term for base of pow()");
  mooseAssert(right._root != NULL, "Empty term for exponent of pow()");
  return ExpressionBuilderToo::EBTerm(new ExpressionBuilderToo::EBBinaryOpTermNode(
      left.cloneRoot(), right.cloneRoot(), ExpressionBuilderToo::EBBinaryOpTermNode::POW));
}

#define TERNARY_FUNC_IMPLEMENT(op, OP)                                                             \
  ExpressionBuilderToo::EBTerm op(const ExpressionBuilderToo::EBTerm & left,                       \
                                  const ExpressionBuilderToo::EBTerm & middle,                     \
                                  const ExpressionBuilderToo::EBTerm & right)                      \
  {                                                                                                \
    mooseAssert(left._root != NULL,                                                                \
                "Empty term provided as first argument of the ternary function " #op "()");        \
    mooseAssert(middle._root != NULL,                                                              \
                "Empty term provided as second argument of the ternary function " #op "()");       \
    mooseAssert(right._root != NULL,                                                               \
                "Empty term provided as third argument of the ternary function " #op "()");        \
    return ExpressionBuilderToo::EBTerm(new ExpressionBuilderToo::EBTernaryFuncTermNode(           \
        left.cloneRoot(),                                                                          \
        middle.cloneRoot(),                                                                        \
        right.cloneRoot(),                                                                         \
        ExpressionBuilderToo::EBTernaryFuncTermNode::OP));                                         \
  }
TERNARY_FUNC_IMPLEMENT(conditional, CONDITIONAL)

/*******************************
 * End of Term implementations
 * EBTensor
 ********************************/
