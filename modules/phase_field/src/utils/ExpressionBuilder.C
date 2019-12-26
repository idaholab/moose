//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExpressionBuilder.h"

ExpressionBuilder::ExpressionBuilder(const InputParameters & pars)
{
  std::set<std::string> coupled_vars = pars.getCoupledVariableParamNames();
  std::map<std::string, std::pair<std::string, std::string>> vec_coupled =
      pars.getAutoBuildVectors();
  std::map<std::string, std::pair<std::string, std::string>>::iterator finder;
  for (std::set<std::string>::const_iterator it = coupled_vars.begin(); it != coupled_vars.end();
       ++it)
  {
    _coup_vars[*it] = EBTermList(0);
    _grad_coup_vars[*it] = EBTermList(0);
    _second_coup_vars[*it] = EBTermList(0);
    if (vec_coupled.find(*it) != vec_coupled.end())
    {
      std::pair<std::string, std::string> variable = vec_coupled[*it];
      // std::string base_name = pars.get<std::string>(variable.first);
      std::string base_name = *it;
      for (unsigned int j = 0; j < pars.get<unsigned int>(variable.second); ++j)
      {
        std::string varname = base_name + std::to_string(j);
        _coup_vars[*it].push_back(EBTerm(varname.c_str()));
        _grad_coup_vars[*it].push_back(makeGradEB(varname));
        _second_coup_vars[*it].push_back(makeSecondEB(varname));
      }
    }
    else
    {
      _coup_vars[*it].push_back(EBTerm(*it->c_str()));
      _grad_coup_vars[*it].push_back(makeGradEB(*it));
      _second_coup_vars[*it].push_back(makeSecondEB(*it));
    }
  }
}

ExpressionBuilder::EBTerm
ExpressionBuilder::makeGradEB(const std::string & var_name)
{
  return EBTerm({var_name + "_x", var_name + "_y", var_name + "_z"}, {3});
}

ExpressionBuilder::EBTerm
ExpressionBuilder::makeSecondEB(const std::string & var_name)
{
  return EBTerm({var_name + "_xx",
                 var_name + "_xy",
                 var_name + "_xz",
                 var_name + "_yx",
                 var_name + "_yy",
                 var_name + "_yz",
                 var_name + "_zx",
                 var_name + "_zy",
                 var_name + "_zz"},
                {3, 3});
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

ExpressionBuilder::EBTerm::operator std::vector<std::string>()
{
  if (_root != NULL)
  {
    std::vector<std::string> string_vector;
    string_vector = _root->fullStringify();
    return string_vector;
  }
  return std::vector<std::string>(1, "[NULL]");
}

std::vector<std::string>
ExpressionBuilder::EBTermNode::fullStringify() const
{
  std::vector<std::string> string_vector;
  getStringVector(string_vector, std::vector<unsigned int>(_shape.size(), 0));
  return string_vector;
}

void
ExpressionBuilder::EBTermNode::getStringVector(std::vector<std::string> & string_vector,
                                               std::vector<unsigned int> current_dim,
                                               unsigned int position) const
{
  for (unsigned int i = 0; i < _shape[position]; ++i)
  {
    current_dim[position] = i;
    if (position != _shape.size() - 1)
      getStringVector(string_vector, current_dim, position + 1);
    string_vector.push_back(stringify(current_dim));
  }
}

void
ExpressionBuilder::EBTerm::checkShape(const std::vector<unsigned int> component) const
{
  std::vector<unsigned int> shape = _root->getShape();
  if (component.size() != shape.size())
    mooseError("Incorrect size for accessing EBTerm");
  for (unsigned int i = 0; i < component.size(); ++i)
    if (component[i] >= shape[i])
      mooseError("Incorrect size for accessing EBTerm");
}

ExpressionBuilder::EBTerm
ExpressionBuilder::EBTerm::identity(unsigned int mat_size, int k)
{
  std::vector<Real> mat_vec(mat_size * mat_size);
  for (unsigned int i = 0; i < mat_size; ++i)
    for (unsigned int j = 0; j < mat_size; ++j)
      if (i == j + k)
        mat_vec[i * mat_size + j] = 1.;
      else
        mat_vec[i * mat_size + j] = 0.;
  return EBTerm(mat_vec, {mat_size, mat_size});
}

void
ExpressionBuilder::EBTermNode::transpose()
{
  if (getShape().size() == 2)
  {
    if (_isTransposed == false)
      _isTransposed = true;
    else
      _isTransposed = false;
    std::reverse(_shape.begin(), _shape.end());
  }
  else
    mooseError("Cannot transpose higher order matrices or scalars");
}

void
ExpressionBuilder::EBTermNode::transposeComponent(std::vector<unsigned int> & component) const
{
  // Implement current transpose rule here
  std::reverse(component.begin(), component.end());
}

std::string
ExpressionBuilder::EBSymbolNode::stringify(std::vector<unsigned int> component) const
{
  if (_isTransposed)
    transposeComponent(component);

  unsigned int position = 0;
  for (unsigned int i = 0; i < component.size(); ++i)
  {
    unsigned int multiplier = 1;
    for (unsigned int j = i + 1; j < component.size(); ++j)
      multiplier *= _shape[j];
    position += component[i] * multiplier;
  }

  return _symbol[position];
}

std::string
ExpressionBuilder::EBTempIDNode::stringify(std::vector<unsigned int> component) const
{
  std::ostringstream s;
  s << '[' << _id << ']';
  return s.str();
  component.clear(); // Suppresses unused parameter warning
}

std::string
ExpressionBuilder::EBUnaryFuncTermNode::stringify(std::vector<unsigned int> component) const
{
  if (_isTransposed)
    transposeComponent(component);

  const char * name[] = {"sin", "cos", "tan", "abs", "log", "log2", "log10", "exp", "sinh", "cosh"};
  std::ostringstream s;
  s << name[_type] << '(' << _subnode->stringify(component) << ')';
  return s.str();
}

std::vector<unsigned int>
ExpressionBuilder::EBUnaryFuncTermNode::setShape()
{
  std::vector<unsigned int> sub_shape = _subnode->setShape();
  if (sub_shape.size() != 1 || sub_shape[0] != 1)
    mooseError("Improper shape for unary node");
  _shape = sub_shape;
  return sub_shape;
}

std::string
ExpressionBuilder::EBUnaryOpTermNode::stringify(std::vector<unsigned int> component) const
{
  if (_isTransposed)
    transposeComponent(component);

  const char * name[] = {"-", "!"};
  std::ostringstream s;

  s << name[_type];

  if (_subnode->precedence() > precedence())
    s << '(' << _subnode->stringify(component) << ')';
  else
    s << _subnode->stringify(component);

  return s.str();
}

std::vector<unsigned int>
ExpressionBuilder::EBUnaryOpTermNode::setShape()
{
  std::vector<unsigned int> sub_shape = _subnode->setShape();
  if ((sub_shape.size() != 1 || sub_shape[0] != 1) && _type == LOGICNOT)
    mooseError("Improper shape for unary node with operator !=");
  _shape = sub_shape;
  return sub_shape;
}

std::string
ExpressionBuilder::EBBinaryFuncTermNode::stringify(std::vector<unsigned int> component) const
{
  if (_isTransposed)
    transposeComponent(component);

  const char * name[] = {"min", "max", "atan2", "hypot", "plog"};
  std::ostringstream s;
  s << name[_type] << '(' << _left->stringify(component) << ',' << _left->stringify(component)
    << ')';
  return s.str();
}

std::vector<unsigned int>
ExpressionBuilder::EBBinaryFuncTermNode::setShape()
{
  std::vector<unsigned int> left_shape = _left->setShape();
  std::vector<unsigned int> right_shape = _right->setShape();

  if (left_shape.size() != 1 || left_shape[0] != 1)
    mooseError("Improper shape for binary function node");
  if (right_shape.size() != 1 || right_shape[0] != 1)
    mooseError("Improper shape for binary function node");
  _shape = left_shape;
  return left_shape;
}

std::string
ExpressionBuilder::EBBinaryOpTermNode::stringify(std::vector<unsigned int> component) const
{
  if (_isTransposed)
    transposeComponent(component);

  const char * name[] = {"+", "-", "*", "/", "%", "^", "<", ">", "<=", ">=", "=", "!="};
  std::ostringstream s;

  if (_type == MUL)
    return multRule(component);

  if (_type == CROSS)
    return crossRule(component);

  std::vector<unsigned int> left_component = component;
  std::vector<unsigned int> right_component = component;

  if (_left->precedence() > precedence())
    s << '(' << _left->stringify(left_component) << ')';
  else
    s << _left->stringify(left_component);

  s << name[_type];

  // these operators are left associative at equal precedence
  // (this matters for -,/,&,^ but not for + and *)
  if (_right->precedence() > precedence() ||
      (_right->precedence() == precedence() && (_type == SUB || _type == DIV)))
    s << '(' << _right->stringify(right_component) << ')';
  else
    s << _right->stringify(right_component);

  return s.str();
}

std::vector<unsigned int>
ExpressionBuilder::EBBinaryOpTermNode::setShape()
{
  std::vector<unsigned int> left_shape = _left->setShape();
  std::vector<unsigned int> right_shape = _right->setShape();

  switch (_type)
  {
    case ADD:
    case SUB:
      if (left_shape != right_shape)
        mooseError("Improper shape for binary operator node");
      break;
    case MUL:
      if (left_shape.back() != right_shape[0])
      {
        if (left_shape == right_shape && left_shape.size() == 1)
        {
          left_shape = std::vector<unsigned int>(1, 1);
          break;
        }
        if (left_shape.size() == 1 && left_shape[0] == 1)
        {
          left_shape = right_shape;
          break;
        }
        if (right_shape.size() == 1 && right_shape[0] == 1)
          break;
        mooseError("Improper shape for binary operator node");
      }
      left_shape.pop_back();
      left_shape.insert(left_shape.end(), right_shape.begin() + 1, right_shape.end());
      break;
    case DIV:
      if (right_shape.size() != 1 || right_shape[0] != 1)
        mooseError("Improper shape for binary operator node");
      break;
    case MOD:
    case POW:
    case LESS:
    case GREATER:
    case LESSEQ:
    case GREATEREQ:
    case EQ:
    case NOTEQ:
      if (left_shape.size() != 1 || left_shape[0] != 1)
        mooseError("Improper shape for binary operator node");
      if (right_shape.size() != 1 || right_shape[0] != 1)
        mooseError("Improper shape for binary operator node");
      break;
    case CROSS:
      if (left_shape.size() != 1 || left_shape[0] != 3)
        mooseError("Improper shape for binary operator node");
      if (right_shape.size() != 1 || right_shape[0] != 3)
        mooseError("Improper shape for binary operator node");
  }
  _shape = left_shape;
  return left_shape;
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
    case CROSS:
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

std::string
ExpressionBuilder::EBBinaryOpTermNode::multRule(std::vector<unsigned int> component) const
{
  std::vector<unsigned int> left_dims = _left->getShape();
  std::vector<unsigned int> right_dims = _right->getShape();
  std::ostringstream s;

  if (left_dims == right_dims && left_dims.size() == 1)
  {
    std::vector<unsigned int> current_comp(1);
    s << '(';
    for (unsigned int i = 0; i < left_dims.size(); ++i)
    {
      current_comp[0] = i;
      s << _left->stringify(current_comp) << '*' << _right->stringify(current_comp) << '+';
    }
    std::string return_string = s.str();
    return_string.back() = ')';
    return return_string;
  }

  std::vector<unsigned int> zero_vector(1, 0);
  if (left_dims[0] == 1 && left_dims.size() == 1)
  {
    s << _left->stringify(zero_vector) << '*' << (_right->stringify(component));
    return s.str();
  }

  if (right_dims[0] == 1 && right_dims.size() == 1)
  {
    s << _left->stringify(component) << '*' << _right->stringify(zero_vector);
    return s.str();
  }

  std::vector<unsigned int> left_component(component.begin(), component.begin() + left_dims.size());
  std::vector<unsigned int> right_component(component.begin() + left_dims.size() - 2,
                                            component.end());
  s << '(';
  for (unsigned int i = 0; i < right_dims[0]; ++i)
  {
    left_component.back() = i;
    right_component[0] = i;
    s << _left->stringify(left_component) << '*' << _right->stringify(right_component) << '+';
  }
  std::string finished = s.str();
  finished.back() = ')';
  return finished;
}

std::string
ExpressionBuilder::EBBinaryOpTermNode::crossRule(std::vector<unsigned int> component) const
{
  std::vector<unsigned int> comp1(1, (component[0] + 1) % 3);
  std::vector<unsigned int> comp2(1, (component[0] + 2) % 3);
  std::ostringstream s;

  s << '(' << _left->stringify(comp1) << '*' << _right->stringify(comp2);
  s << '-' << _left->stringify(comp2) << '*' << _right->stringify(comp1) << ')';
  return s.str();
}

std::string
ExpressionBuilder::EBTernaryFuncTermNode::stringify(std::vector<unsigned int> component) const
{
  if (_isTransposed)
    transposeComponent(component);

  std::vector<unsigned int> zero_vector(1, 0);
  const char * name[] = {"if"};
  std::ostringstream s;
  s << name[_type] << '(' << _left->stringify(zero_vector) << ',' << _middle->stringify(component)
    << ',' << _right->stringify(component) << ')';
  return s.str();
}

std::vector<unsigned int>
ExpressionBuilder::EBTernaryFuncTermNode::setShape()
{
  std::vector<unsigned int> left_shape = _left->setShape();
  std::vector<unsigned int> right_shape = _right->setShape();
  std::vector<unsigned int> middle_shape = _middle->setShape();

  if (middle_shape.size() != 1 || middle_shape[0] != 1)
    mooseError("Improper shape for binary operator node");
  if (left_shape != right_shape)
    mooseError("Improper shape for binary operator node");
  _shape = left_shape;
  return left_shape;
}

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

ExpressionBuilder::EBFunction::operator std::vector<std::string>() const
{
  EBTerm eval;
  eval = *this; // typecast EBFunction -> EBTerm performs a parameter substitution
  return std::vector<std::string>(eval);
}

std::string
ExpressionBuilder::EBFunction::args()
{
  unsigned int narg = _arguments.size();
  if (narg < 1)
    return "";

  std::ostringstream s;
  for (unsigned int i = 0; i < narg; ++i)
    for (std::string & arg : std::vector<std::string>(_arguments[i]))
      s << arg << ",";
  std::string all_args = s.str();
  all_args.pop_back();
  return all_args;
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

ExpressionBuilder::EBTerm
cross(const ExpressionBuilder::EBTerm & left, const ExpressionBuilder::EBTerm & right)
{
  mooseAssert(left._root != NULL, "Empty term for left side of cross()");
  mooseAssert(right._root != NULL, "Empty term for right side of cross()");
  return ExpressionBuilder::EBTerm(new ExpressionBuilder::EBBinaryOpTermNode(
      left.cloneRoot(), right.cloneRoot(), ExpressionBuilder::EBBinaryOpTermNode::CROSS));
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
  _find = find_root->fullStringify();

  // the term we want to substitute with
  if (replace.getRoot() != NULL)
    _replace = replace.cloneRoot();
  else
    mooseError("Trying to substitute in an empty term");
}

ExpressionBuilder::EBTermNode *
ExpressionBuilder::EBTermSubstitution::substitute(const EBSymbolNode & node) const
{
  if (node.fullStringify() == _find)
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
