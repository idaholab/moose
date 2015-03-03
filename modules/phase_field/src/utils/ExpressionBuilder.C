/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ExpressionBuilder.h"

#include "MooseError.h"

ExpressionBuilder::EBTermList
operator, (const ExpressionBuilder::EBTerm & larg, const ExpressionBuilder::EBTerm & rarg)
{
  ExpressionBuilder::EBTermList list(2);
  list[0] = larg;
  list[1] = rarg;
  return list;
}

ExpressionBuilder::EBTermList
operator, (const ExpressionBuilder::EBTerm & larg, const ExpressionBuilder::EBTermList & rargs)
{
  ExpressionBuilder::EBTermList list(1);
  list[0] = larg;
  list.insert( list.end(), rargs.begin(), rargs.end() );
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
operator<< (std::ostream & os, const ExpressionBuilder::EBTerm & term)
{
  if (term.root != NULL)
    return os << *term.root;
  else
    return os << "[NULL]";
}


std::string
ExpressionBuilder::EBSymbolNode::stringify() const
{
  return symbol;
}

std::string
ExpressionBuilder::EBUnaryFuncTermNode::stringify() const
{
  const char * name[] = { "sin", "cos", "tan", "abs", "log", "log2", "log10", "exp", "sinh", "cosh" };
  std::ostringstream s;
  s << name[type] << '(' << *subnode << ')';
  return s.str();
}

std::string
ExpressionBuilder::EBUnaryOpTermNode::stringify() const
{
  const char * name[] = { "-", "!" };
  std::ostringstream s;

  s <<  name[type];

  if (subnode->precedence() > precedence())
    s << '(' << *subnode << ')';
  else
    s << *subnode;

  return s.str();
}

std::string
ExpressionBuilder::EBBinaryFuncTermNode::stringify() const
{
  const char * name[] = { "min", "max", "atan2", "hypot", "plog" };
  std::ostringstream s;
  s << name[type] << '(' << *left << ',' << *right << ')';
  return s.str();
}

std::string
ExpressionBuilder::EBBinaryOpTermNode::stringify() const
{
  const char * name[] = { "+", "-", "*", "/", "%", "^", "<", ">", "<=", ">=", "=", "!=" };
  std::ostringstream s;

  if (left->precedence() > precedence())
    s << '(' << *left << ')';
  else
    s << *left;

  s << name[type];

  // these operators are left associative at equal precedence
  // (this matters for -,/,&,^ but not for + and *)
  if (right->precedence() > precedence() || (
        right->precedence() == precedence() && (
          type == SUB || type == DIV || type == MOD || type == POW
        )))
    s << '(' << *right << ')';
  else
    s << *right;

  return s.str();
}

int
ExpressionBuilder::EBBinaryOpTermNode::precedence() const
{
  switch (type)
  {
    case ADD: case SUB:
      return 6;
    case MUL: case DIV: case MOD:
      return 5;
    case POW:
      return 2;
    case LESS: case GREATER: case LESSEQ: case GREATEREQ:
      return 8;
    case EQ: case NOTEQ:
      return 9;
  }

  mooseError("Unknown type.");
}

std::string
ExpressionBuilder::EBTernaryFuncTermNode::stringify() const
{
  const char * name[] = { "if" };
  std::ostringstream s;
  s << name[type] << '(' << *left << ',' << *middle << ',' << *right << ')';
  return s.str();
}

ExpressionBuilder::EBFunction &
ExpressionBuilder::EBFunction::operator() (const ExpressionBuilder::EBTerm & arg)
{
  this->eval_arguments.resize(1);
  this->eval_arguments[0] = arg;
  return *this;
}

ExpressionBuilder::EBFunction &
ExpressionBuilder::EBFunction::operator() (const ExpressionBuilder::EBTermList & args)
{
  this->eval_arguments = EBTermList(args);
  return *this;
}

ExpressionBuilder::EBFunction &
ExpressionBuilder::EBFunction::operator= (const ExpressionBuilder::EBTerm & term)
{
  this->arguments = this->eval_arguments;
  this->term = term;
  return *this;
}

ExpressionBuilder::EBFunction &
ExpressionBuilder::EBFunction::operator= (const ExpressionBuilder::EBFunction & func)
{
  this->arguments = this->eval_arguments;
  this->term = EBTerm(func);
  return *this;
}

ExpressionBuilder::EBFunction::operator ExpressionBuilder::EBTerm() const
{
  unsigned int narg = arguments.size();
  if (narg != eval_arguments.size())
    mooseError("EBFunction is used wth a different number of arguments than it was defined with.");

  // prepare a copy of the function term to perform the substitution on
  EBTerm result(term);

  // prepare a rule list for the substitutions
  EBSubstitutionRuleList rules;
  for (unsigned i = 0; i < narg; ++i)
    rules.push_back(new EBTermSubstitution(arguments[i], eval_arguments[i]));

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
  unsigned int narg = arguments.size();
  if (narg < 1)
    return "";

  //for (unsigned int i = 1; i < nargs; ++i) // check for symbol nodes here

  std::ostringstream s;
  s << arguments[0];
  for (unsigned int i = 1; i < narg; ++i)
    s << ',' << arguments[i];

  return s.str();
}

unsigned int
ExpressionBuilder::EBFunction::substitute(const EBSubstitutionRule & rule)
{
  return term.substitute(rule);
}

unsigned int
ExpressionBuilder::EBFunction::substitute(const EBSubstitutionRuleList & rules)
{
  return term.substitute(rules);
}


#define UNARY_FUNC_IMPLEMENT(op,OP) \
ExpressionBuilder::EBTerm op (const ExpressionBuilder::EBTerm & term) { \
  return ExpressionBuilder::EBTerm( \
    new ExpressionBuilder::EBUnaryFuncTermNode(term.root->clone(), ExpressionBuilder::EBUnaryFuncTermNode::OP) \
  ); \
}
UNARY_FUNC_IMPLEMENT(sin,SIN)
UNARY_FUNC_IMPLEMENT(cos,COS)
UNARY_FUNC_IMPLEMENT(tan,TAN)
UNARY_FUNC_IMPLEMENT(abs,ABS)
UNARY_FUNC_IMPLEMENT(log,LOG)
UNARY_FUNC_IMPLEMENT(log2,LOG2)
UNARY_FUNC_IMPLEMENT(log10,LOG10)
UNARY_FUNC_IMPLEMENT(exp,EXP)
UNARY_FUNC_IMPLEMENT(sinh,SINH)
UNARY_FUNC_IMPLEMENT(cosh,COSH)

#define BINARY_FUNC_IMPLEMENT(op,OP) \
ExpressionBuilder::EBTerm op (const ExpressionBuilder::EBTerm & left, const ExpressionBuilder::EBTerm & right) { \
  return ExpressionBuilder::EBTerm( \
    new ExpressionBuilder::EBBinaryFuncTermNode(left.root->clone(), right.root->clone(), ExpressionBuilder::EBBinaryFuncTermNode::OP) \
  ); \
}
BINARY_FUNC_IMPLEMENT(min,MIN)
BINARY_FUNC_IMPLEMENT(max,MAX)
BINARY_FUNC_IMPLEMENT(atan2,ATAN2)
BINARY_FUNC_IMPLEMENT(hypot,HYPOT)
BINARY_FUNC_IMPLEMENT(plog,PLOG)

// this is a function in ExpressionBuilder (pow) but an operator in FParser (^)
ExpressionBuilder::EBTerm pow(const ExpressionBuilder::EBTerm & left, const ExpressionBuilder::EBTerm & right) {
  return ExpressionBuilder::EBTerm(
    new ExpressionBuilder::EBBinaryOpTermNode(
      left.root->clone(), right.root->clone(), ExpressionBuilder::EBBinaryOpTermNode::POW
    )
  );
}

#define TERNARY_FUNC_IMPLEMENT(op,OP) \
ExpressionBuilder::EBTerm op (const ExpressionBuilder::EBTerm & left, const ExpressionBuilder::EBTerm & middle, const ExpressionBuilder::EBTerm & right) { \
  return ExpressionBuilder::EBTerm( \
    new ExpressionBuilder::EBTernaryFuncTermNode(left.root->clone(), middle.root->clone(), right.root->clone(), ExpressionBuilder::EBTernaryFuncTermNode::OP) \
  ); \
}
TERNARY_FUNC_IMPLEMENT(conditional,CONDITIONAL)

unsigned int
ExpressionBuilder::EBUnaryTermNode::substitute(const EBSubstitutionRuleList & rules)
{
  unsigned int nrule = rules.size();

  for (unsigned int i = 0; i < nrule; ++i)
  {
    EBTermNode * replace = rules[i]->apply(subnode);
    if (replace != NULL)
    {
      delete subnode;
      subnode = replace;
      return 1;
    }
  }

  return subnode->substitute(rules);
}

unsigned int
ExpressionBuilder::EBBinaryTermNode::substitute(const EBSubstitutionRuleList & rules)
{
  unsigned int nrule = rules.size();
  unsigned int success = 0;

  for (unsigned int i = 0; i < nrule; ++i)
  {
    EBTermNode * replace = rules[i]->apply(left);
    if (replace != NULL)
    {
      delete left;
      left = replace;
      success = 1;
      break;
    }
  }

  if (success == 0)
    success += left->substitute(rules);

  for (unsigned int i = 0; i < nrule; ++i)
  {
    EBTermNode * replace = rules[i]->apply(right);
    if (replace != NULL)
    {
      delete right;
      right = replace;
      return success + 1;
    }
  }

  return success + right->substitute(rules);
}

unsigned int
ExpressionBuilder::EBTernaryTermNode::substitute(const EBSubstitutionRuleList & rules)
{
  unsigned int nrule = rules.size();
  bool left_success   = false,
       middle_success = false,
       right_success  = false;
  EBTermNode * replace;

  for (unsigned int i = 0; i < nrule; ++i)
  {
    replace = rules[i]->apply(left);
    if (replace)
    {
      delete left;
      left = replace;
      left_success = true;
      break;
    }
  }

  for (unsigned int i = 0; i < nrule; ++i)
  {
    replace = rules[i]->apply(middle);
    if (replace)
    {
      delete middle;
      middle = replace;
      middle_success = true;
      break;
    }
  }

  for (unsigned int i = 0; i < nrule; ++i)
  {
    replace = rules[i]->apply(right);
    if (replace)
    {
      delete right;
      right = replace;
      right_success = true;
      break;
    }
  }

  if (!left_success)
    left_success = left->substitute(rules);
  if (!middle_success)
    middle_success = middle->substitute(rules);
  if (!right_success)
    right_success = right->substitute(rules);

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

  if (root == NULL)
    return 0;

  for (unsigned int i = 0; i < nrule; ++i)
  {
    EBTermNode * replace = rules[i]->apply(root);
    if (replace != NULL)
    {
      delete root;
      root = replace;
      return 1;
    }
  }

  return root->substitute(rules);
}

template<class Node_T>
ExpressionBuilder::EBTermNode * ExpressionBuilder::EBSubstitutionRuleTyped<Node_T>::apply(const ExpressionBuilder::EBTermNode * node) const
{
  const Node_T * match_node = dynamic_cast<const Node_T *>(node);
  if (match_node == NULL)
    return NULL;
  else
    return substitute(*match_node);
}

ExpressionBuilder::EBTermSubstitution::EBTermSubstitution(const EBTerm & _find, const EBTerm & _replace) :
    replace(_replace.getRoot()->clone())
{
  const EBSymbolNode * find_root = dynamic_cast<const EBSymbolNode *>(_find.getRoot());
  if (find_root == NULL)
    mooseError("Function arguments must be pure symbols.");
  find = find_root->stringify();
}

ExpressionBuilder::EBTermNode *
ExpressionBuilder::EBTermSubstitution::substitute(const EBSymbolNode & node) const
{
  if (node.stringify() == find)
    return replace->clone();
  else
    return NULL;
}

ExpressionBuilder::EBTermNode *
ExpressionBuilder::EBLogPlogSubstitution::substitute(const EBUnaryFuncTermNode & node) const
{
  if (node.type == EBUnaryFuncTermNode::LOG)
    return new EBBinaryFuncTermNode(node.getSubnode()->clone(), epsilon->clone(), EBBinaryFuncTermNode::PLOG);
  else
    return NULL;
}
