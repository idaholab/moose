//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExpressionBuilder.h"
#include "EBSimpleTrees.h"

std::vector<std::pair<ExpressionBuilder::EBTermNode *, ExpressionBuilder::EBTermNode *>>
    ExpressionBuilder::EBTerm::_prep_simplification_rules = PrepEBSimpleTrees();
std::vector<std::pair<ExpressionBuilder::EBTermNode *, ExpressionBuilder::EBTermNode *>>
    ExpressionBuilder::EBTerm::_simplification_rules = EBSimpleTrees();

/******************************************
 * The Beginning of the Node implementations
 * EBTermNode
 *******************************************/
bool
ExpressionBuilder::EBTermNode::compareRule(EBTermNode * rule,
                                           std::vector<EBTermNode *> & stars,
                                           std::vector<int> & current_index,
                                           bool leafCompare)
{
  std::stack<EBTermNode *> self_stack;
  self_stack.emplace(this);
  std::stack<EBTermNode *> rule_stack;
  rule_stack.emplace(rule);
  std::vector<EBTermNode *> star_vec(stars);
  std::stack<Comparer *> permutations;
  bool isHead = true;
  while (rule_stack.size() > 0)
  {
    // Get the children of the top of the stack
    std::vector<EBTermNode *> self_children = self_stack.top()->getChildren();
    std::vector<EBTermNode *> rule_children = rule_stack.top()->getChildren();
    // Check the nodes to make sure they match
    // If not, move to the next permutation
    if (!rule_stack.top()->compare(self_stack.top(), star_vec, isHead))
    {
      if (permutations.size() != 0)
      {
        self_stack = permutations.top()->_self_stack;
        rule_stack = permutations.top()->_rule_stack;
        star_vec = permutations.top()->_stars;
        current_index = permutations.top()->_current_index;
        permutations.pop();
        continue;
      }
      else
        return false;
    }
    EBStarNode * checkLeaf = dynamic_cast<EBStarNode *>(rule_stack.top());
    // We have 3 possibilities here.
    // The first is if we are at a leaf node.
    // The second is if we are at a commutative node.
    // The third is everything else.
    if (checkLeaf != NULL)
    {
      rule_stack.pop();
      self_stack.pop();
    }
    else if (self_stack.top()->isCommutative())
    {
      EBNNaryOpTermNode * checkNNary = dynamic_cast<EBNNaryOpTermNode *>(self_stack.top());
      std::vector<std::vector<EBTermNode *>> permutedChildren(0);
      std::vector<std::vector<int>> nnary_index(0);
      if (checkNNary != NULL && isHead)
        permutedChildren = permuteChildren(self_children, nnary_index);
      else
        permutedChildren = permuteChildren(self_children);
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
        self_stack.emplace(self_children[i]);

      rule_stack.pop();
      for (unsigned int i = 0; i < rule_children.size(); ++i)
        rule_stack.emplace(rule_children[i]);
    }

    isHead = false;
  }
  stars = star_vec;
  return true;
}

ExpressionBuilder::EBTermNode *
ExpressionBuilder::EBTermNode::simplify(std::pair<EBTermNode *, EBTermNode *> rule, bool & changed)
{
  std::vector<EBTermNode *> stars(26, NULL);
  std::vector<int> current_index(0);
  if (compareRule(rule.first->clone(), stars, current_index))
  {
    changed = true;
    EBTermNode * replacement = rule.second->clone();
    return replacement->replaceRule(stars);
  }
  return this;
}

std::vector<std::vector<ExpressionBuilder::EBTermNode *>>
ExpressionBuilder::EBTermNode::permuteChildren(std::vector<EBTermNode *> children)
{
  std::vector<std::vector<EBTermNode *>> resulting;
  if (children.size() > 1)
    for (unsigned int i = 0; i < children.size(); ++i)
    {
      std::vector<EBTermNode *> to_recurse(children.begin(), children.begin() + i);
      to_recurse.insert(to_recurse.end(), children.begin() + i + 1, children.end());
      std::vector<std::vector<EBTermNode *>> next_recursion = permuteChildren(to_recurse);
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

std::vector<std::vector<ExpressionBuilder::EBTermNode *>>
ExpressionBuilder::EBTermNode::permuteChildren(std::vector<EBTermNode *> children,
                                               std::vector<std::vector<int>> & nnary_index)
{

  std::vector<std::vector<ExpressionBuilder::EBTermNode *>> resulting;
  for (unsigned int i = 0; i < children.size(); ++i)
    for (unsigned int j = 0; j < children.size(); ++j)
    {
      std::vector<EBTermNode *> permutation(2);
      std::vector<int> current_index(2);
      if (i != j)
      {
        permutation[0] = children[i];
        permutation[1] = children[j];
        current_index[0] = i;
        current_index[1] = j;
        resulting.emplace_back(permutation);
        nnary_index.emplace_back(current_index);
      }
    }
  return resulting;
}

/*******************************
 * EBSymbolNode
 ********************************/

std::string
ExpressionBuilder::EBSymbolNode::stringify() const
{
  return _symbol;
}

/*******************************
 * EBTempIDNode
 ********************************/

std::string
ExpressionBuilder::EBTempIDNode::stringify() const
{
  std::ostringstream s;
  s << '[' << _id << ']';
  return s.str();
}

/*******************************
 * EBUnaryFuncTermNode
 ********************************/

std::string
ExpressionBuilder::EBUnaryFuncTermNode::stringify() const
{
  const char * name[] = {"sin", "cos", "tan", "abs", "log", "log2", "log10", "exp", "sinh", "cosh"};
  std::ostringstream s;
  s << name[_type] << '(' << *_subnode << ')';
  return s.str();
}

/*******************************
 * EBUnaryOpTermNode
 ********************************/

std::string
ExpressionBuilder::EBUnaryOpTermNode::stringify() const
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

/*******************************
 * EBBinaryFuncTermNode
 ********************************/

std::string
ExpressionBuilder::EBBinaryFuncTermNode::stringify() const
{
  const char * name[] = {"min", "max", "atan2", "hypot", "plog"};
  std::ostringstream s;
  s << name[_type] << '(' << *_left << ',' << *_right << ')';
  return s.str();
}

bool
ExpressionBuilder::EBBinaryFuncTermNode::isCommutative() const
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

/*******************************
 * EBBinaryOpTermNode
 ********************************/

std::string
ExpressionBuilder::EBBinaryOpTermNode::stringify() const
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
ExpressionBuilder::EBBinaryOpTermNode::precedence() const
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
ExpressionBuilder::EBBinaryOpTermNode::isCommutative() const
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
ExpressionBuilder::EBBinaryOpTermNode::compare(EBTermNode * compare_to,
                                               std::vector<EBTermNode *> &,
                                               bool isHead)
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

ExpressionBuilder::EBTermNode *
ExpressionBuilder::EBBinaryOpTermNode::toNNary()
{
  // Possibly change this to add all commutative operators
  if (_type == MUL || _type == ADD)
  {
    _left = _left->toNNary();
    _right = _right->toNNary();
    std::vector<EBTermNode *> new_children;
    EBNNaryOpTermNode * _left_nnary = dynamic_cast<EBNNaryOpTermNode *>(_left);
    if (_left_nnary != NULL)
    {
      std::vector<EBTermNode *> _left_children = _left_nnary->getChildren();
      new_children.insert(new_children.end(), _left_children.begin(), _left_children.end());
    }
    else
      new_children.emplace_back(_left);
    EBNNaryOpTermNode * _right_nnary = dynamic_cast<EBNNaryOpTermNode *>(_right);
    if (_right_nnary != NULL)
    {
      std::vector<EBTermNode *> _right_children = _right_nnary->getChildren();
      new_children.insert(new_children.end(), _right_children.begin(), _right_children.end());
    }
    else
      new_children.emplace_back(_right);
    if (_type == MUL)
      return new EBNNaryOpTermNode(new_children, EBNNaryOpTermNode::NodeType::MUL);
    if (_type == ADD)
      return new EBNNaryOpTermNode(new_children, EBNNaryOpTermNode::NodeType::ADD);
  }
  else
    return EBBinaryTermNode::toNNary();
  return this;
}

/*******************************
 * EBTernaryFuncTermNode
 ********************************/

std::string
ExpressionBuilder::EBTernaryFuncTermNode::stringify() const
{
  const char * name[] = {"if"};
  std::ostringstream s;
  s << name[_type] << '(' << *_left << ',' << *_middle << ',' << *_right << ')';
  return s.str();
}

/*******************************
 * EBNNaryOpTermNode
 ********************************/

std::string
ExpressionBuilder::EBNNaryOpTermNode::stringify() const
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
ExpressionBuilder::EBNNaryOpTermNode::precedence() const
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

ExpressionBuilder::EBTermNode *
ExpressionBuilder::EBNNaryOpTermNode::simplify(std::pair<EBTermNode *, EBTermNode *> rule,
                                               bool & changed)
{
  for (unsigned int i = 0; i < _children.size(); ++i)
    _children[i] = _children[i]->simplify(rule, changed);
  std::vector<EBTermNode *> stars(26, NULL);
  std::vector<int> current_index(0);
  if (compareRule(rule.first->clone(), stars, current_index))
  {
    changed = true;
    EBTermNode * replacement = rule.second->clone();
    EBBinaryOpTermNode * replac_bin = dynamic_cast<EBBinaryOpTermNode *>(replacement);
    EBNNaryOpTermNode * replac_nnary = dynamic_cast<EBNNaryOpTermNode *>(replacement);
    if (replac_bin != NULL &&
        ((_type == MUL && replac_bin->_type == EBBinaryOpTermNode::NodeType::MUL) ||
         (_type == ADD && replac_bin->_type == EBBinaryOpTermNode::NodeType::ADD)))
    {
      _children[current_index[0]] = replac_bin->getRight()->replaceRule(stars);
      _children[current_index[1]] = replac_bin->getLeft()->replaceRule(stars);
    }
    else if (replac_nnary != NULL)
    {
      if (current_index[0] < current_index[1])
      {
        _children.erase(_children.begin() + current_index[1]);
        _children.erase(_children.begin() + current_index[0]);
      }
      else
      {
        _children.erase(_children.begin() + current_index[1]);
        _children.erase(_children.begin() + current_index[0]);
      }
      std::vector<EBTermNode *> new_children = replac_nnary->getChildren();
      for (unsigned int i = 0; i < new_children.size(); ++i)
        _children.emplace_back(new_children[i]);
    }
    else
    {
      _children[current_index[0]] = replacement->replaceRule(stars);
      _children.erase(_children.begin() + current_index[1]);
    }
  }
  if (_children.size() == 1)
  {
    return _children[0];
  }
  return this;
}

/*******************************
 * End of Node implementations
 * EBTerm & Related Functionality
 ********************************/

void
ExpressionBuilder::EBTerm::simplify()
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

  _root = _root->toNNary();

  for (auto rule : _simplification_rules)
  {
    changed = true;
    while (changed == true)
    {
      changed = false;
      _root = _root->simplify(rule, changed);
      std::cout << _root->stringify() << std::endl;
    }
  }
}

ExpressionBuilder::EBTermList
operator, (const ExpressionBuilder::EBTerm & larg, const ExpressionBuilder::EBTerm & rarg)
{
  return {larg, rarg};
}

ExpressionBuilder::EBTermList
operator, (const ExpressionBuilder::EBTerm & larg, const ExpressionBuilder::EBTermList & rargs)
{
  ExpressionBuilder::EBTermList list = {larg};
  list.insert(list.end(), rargs.begin(), rargs.end());
  return list;
}

ExpressionBuilder::EBTermList
operator, (const ExpressionBuilder::EBTermList & largs, const ExpressionBuilder::EBTerm & rarg)
{
  ExpressionBuilder::EBTermList list = largs;
  list.push_back(rarg);
  return list;
}

std::ostream &
operator<<(std::ostream & os, const ExpressionBuilder::EBTerm & term)
{
  if (term._root != NULL)
    return os << *term._root;
  else
    return os << "[NULL]";
}

/*******************************
 * EBFunction
 ********************************/

ExpressionBuilder::EBFunction &
ExpressionBuilder::EBFunction::operator()(const ExpressionBuilder::EBTerm & arg)
{
  this->_eval_arguments = {arg};
  return *this;
}

ExpressionBuilder::EBFunction &
ExpressionBuilder::EBFunction::operator()(const ExpressionBuilder::EBTermList & args)
{
  this->_eval_arguments = EBTermList(args);
  return *this;
}

ExpressionBuilder::EBFunction &
ExpressionBuilder::EBFunction::operator=(const ExpressionBuilder::EBTerm & term)
{
  this->_arguments = this->_eval_arguments;
  this->_term = term;
  return *this;
}

ExpressionBuilder::EBFunction &
ExpressionBuilder::EBFunction::operator=(const ExpressionBuilder::EBFunction & func)
{
  this->_arguments = this->_eval_arguments;
  this->_term = EBTerm(func);
  return *this;
}

ExpressionBuilder::EBFunction::operator ExpressionBuilder::EBTerm() const
{
  unsigned int narg = _arguments.size();
  if (narg != _eval_arguments.size())
    mooseError("EBFunction is used wth a different number of arguments than it was defined with.");

  // prepare a copy of the function term to perform the substitution on
  EBTerm result(_term);

  // prepare a rule list for the substitutions
  EBSubstitutionRuleList rules;
  for (unsigned i = 0; i < narg; ++i)
    rules.push_back(new EBTermSubstitution(_arguments[i], _eval_arguments[i]));

  // perform substitution
  result.substitute(rules);

  // discard rule set
  for (unsigned i = 0; i < narg; ++i)
    delete rules[i];

  return result;
}

ExpressionBuilder::EBFunction::operator std::string() const
{
  EBTerm eval;
  eval = *this; // typecast EBFunction -> EBTerm performs a parameter substitution
  std::ostringstream s;
  s << eval;
  return s.str();
}

std::string
ExpressionBuilder::EBFunction::args()
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

unsigned int
ExpressionBuilder::EBFunction::substitute(const EBSubstitutionRule & rule)
{
  return _term.substitute(rule);
}

unsigned int
ExpressionBuilder::EBFunction::substitute(const EBSubstitutionRuleList & rules)
{
  return _term.substitute(rules);
}

#define UNARY_FUNC_IMPLEMENT(op, OP)                                                               \
  ExpressionBuilder::EBTerm op(const ExpressionBuilder::EBTerm & term)                             \
  {                                                                                                \
    mooseAssert(term._root != NULL, "Empty term provided as argument of function " #op "()");      \
    return ExpressionBuilder::EBTerm(new ExpressionBuilder::EBUnaryFuncTermNode(                   \
        term.cloneRoot(), ExpressionBuilder::EBUnaryFuncTermNode::OP));                            \
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
  ExpressionBuilder::EBTerm op(const ExpressionBuilder::EBTerm & left,                             \
                               const ExpressionBuilder::EBTerm & right)                            \
  {                                                                                                \
    mooseAssert(left._root != NULL,                                                                \
                "Empty term provided as first argument of function " #op "()");                    \
    mooseAssert(right._root != NULL,                                                               \
                "Empty term provided as second argument of function " #op "()");                   \
    return ExpressionBuilder::EBTerm(new ExpressionBuilder::EBBinaryFuncTermNode(                  \
        left.cloneRoot(), right.cloneRoot(), ExpressionBuilder::EBBinaryFuncTermNode::OP));        \
  }
BINARY_FUNC_IMPLEMENT(min, MIN)
BINARY_FUNC_IMPLEMENT(max, MAX)
BINARY_FUNC_IMPLEMENT(atan2, ATAN2)
BINARY_FUNC_IMPLEMENT(hypot, HYPOT)
BINARY_FUNC_IMPLEMENT(plog, PLOG)

// this is a function in ExpressionBuilder (pow) but an operator in FParser (^)
ExpressionBuilder::EBTerm
pow(const ExpressionBuilder::EBTerm & left, const ExpressionBuilder::EBTerm & right)
{
  mooseAssert(left._root != NULL, "Empty term for base of pow()");
  mooseAssert(right._root != NULL, "Empty term for exponent of pow()");
  return ExpressionBuilder::EBTerm(new ExpressionBuilder::EBBinaryOpTermNode(
      left.cloneRoot(), right.cloneRoot(), ExpressionBuilder::EBBinaryOpTermNode::POW));
}

#define TERNARY_FUNC_IMPLEMENT(op, OP)                                                             \
  ExpressionBuilder::EBTerm op(const ExpressionBuilder::EBTerm & left,                             \
                               const ExpressionBuilder::EBTerm & middle,                           \
                               const ExpressionBuilder::EBTerm & right)                            \
  {                                                                                                \
    mooseAssert(left._root != NULL,                                                                \
                "Empty term provided as first argument of the ternary function " #op "()");        \
    mooseAssert(middle._root != NULL,                                                              \
                "Empty term provided as second argument of the ternary function " #op "()");       \
    mooseAssert(right._root != NULL,                                                               \
                "Empty term provided as third argument of the ternary function " #op "()");        \
    return ExpressionBuilder::EBTerm(new ExpressionBuilder::EBTernaryFuncTermNode(                 \
        left.cloneRoot(),                                                                          \
        middle.cloneRoot(),                                                                        \
        right.cloneRoot(),                                                                         \
        ExpressionBuilder::EBTernaryFuncTermNode::OP));                                            \
  }
TERNARY_FUNC_IMPLEMENT(conditional, CONDITIONAL)

/*******************************
 * Substitution Rules
 ********************************/

unsigned int
ExpressionBuilder::EBUnaryTermNode::substitute(const EBSubstitutionRuleList & rules)
{
  unsigned int nrule = rules.size();

  for (unsigned int i = 0; i < nrule; ++i)
  {
    EBTermNode * replace = rules[i]->apply(_subnode);
    if (replace != NULL)
    {
      delete _subnode;
      _subnode = replace;
      return 1;
    }
  }

  return _subnode->substitute(rules);
}

unsigned int
ExpressionBuilder::EBBinaryTermNode::substitute(const EBSubstitutionRuleList & rules)
{
  unsigned int nrule = rules.size();
  unsigned int success = 0;

  for (unsigned int i = 0; i < nrule; ++i)
  {
    EBTermNode * replace = rules[i]->apply(_left);
    if (replace != NULL)
    {
      delete _left;
      _left = replace;
      success = 1;
      break;
    }
  }

  if (success == 0)
    success += _left->substitute(rules);

  for (unsigned int i = 0; i < nrule; ++i)
  {
    EBTermNode * replace = rules[i]->apply(_right);
    if (replace != NULL)
    {
      delete _right;
      _right = replace;
      return success + 1;
    }
  }

  return success + _right->substitute(rules);
}

unsigned int
ExpressionBuilder::EBTernaryTermNode::substitute(const EBSubstitutionRuleList & rules)
{
  unsigned int nrule = rules.size();
  bool left_success = false, middle_success = false, right_success = false;
  EBTermNode * replace;

  for (unsigned int i = 0; i < nrule; ++i)
  {
    replace = rules[i]->apply(_left);
    if (replace)
    {
      delete _left;
      _left = replace;
      left_success = true;
      break;
    }
  }

  for (unsigned int i = 0; i < nrule; ++i)
  {
    replace = rules[i]->apply(_middle);
    if (replace)
    {
      delete _middle;
      _middle = replace;
      middle_success = true;
      break;
    }
  }

  for (unsigned int i = 0; i < nrule; ++i)
  {
    replace = rules[i]->apply(_right);
    if (replace)
    {
      delete _right;
      _right = replace;
      right_success = true;
      break;
    }
  }

  if (!left_success)
    left_success = _left->substitute(rules);
  if (!middle_success)
    middle_success = _middle->substitute(rules);
  if (!right_success)
    right_success = _right->substitute(rules);

  return left_success + middle_success + right_success;
}

unsigned int
ExpressionBuilder::EBTerm::substitute(const EBSubstitutionRule & rule)
{
  EBSubstitutionRuleList rules(1);
  rules[0] = &rule;
  return substitute(rules);
}

unsigned int
ExpressionBuilder::EBTerm::substitute(const EBSubstitutionRuleList & rules)
{
  unsigned int nrule = rules.size();

  if (_root == NULL)
    return 0;

  for (unsigned int i = 0; i < nrule; ++i)
  {
    EBTermNode * replace = rules[i]->apply(_root);
    if (replace != NULL)
    {
      delete _root;
      _root = replace;
      return 1;
    }
  }

  return _root->substitute(rules);
}

ExpressionBuilder::EBTermSubstitution::EBTermSubstitution(const EBTerm & find,
                                                          const EBTerm & replace)
{
  // the expression we want to substitute (has to be a symbol node)
  const EBSymbolNode * find_root = dynamic_cast<const EBSymbolNode *>(find.getRoot());
  if (find_root == NULL)
    mooseError("Function arguments must be pure symbols.");
  _find = find_root->stringify();

  // the term we want to substitute with
  if (replace.getRoot() != NULL)
    _replace = replace.cloneRoot();
  else
    mooseError("Trying to substitute in an empty term for ", _find);
}

ExpressionBuilder::EBTermNode *
ExpressionBuilder::EBTermSubstitution::substitute(const EBSymbolNode & node) const
{
  if (node.stringify() == _find)
    return _replace->clone();
  else
    return NULL;
}

ExpressionBuilder::EBTermNode *
ExpressionBuilder::EBLogPlogSubstitution::substitute(const EBUnaryFuncTermNode & node) const
{
  if (node._type == EBUnaryFuncTermNode::LOG)
    return new EBBinaryFuncTermNode(
        node.getSubnode()->clone(), _epsilon->clone(), EBBinaryFuncTermNode::PLOG);
  else
    return NULL;
}
