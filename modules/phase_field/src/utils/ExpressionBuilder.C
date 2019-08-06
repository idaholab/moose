//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExpressionBuilder.h"

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

std::string
ExpressionBuilder::EBSymbolNode::stringify() const
{
  return _symbol;
}

std::string
ExpressionBuilder::EBTempIDNode::stringify() const
{
  std::ostringstream s;
  s << '[' << _id << ']';
  return s.str();
}

std::string
ExpressionBuilder::EBUnaryFuncTermNode::stringify() const
{
  const char * name[] = {"sin", "cos", "tan", "abs", "log", "log2", "log10", "exp", "sinh", "cosh", "sqrt"};
  std::ostringstream s;
  s << name[_type] << '(' << *_subnode << ')';
  return s.str();
}

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

std::string
ExpressionBuilder::EBBinaryFuncTermNode::stringify() const
{
  const char * name[] = {"min", "max", "atan2", "hypot", "plog"};
  std::ostringstream s;
  s << name[_type] << '(' << *_left << ',' << *_right << ')';
  return s.str();
}

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

std::string
ExpressionBuilder::EBTernaryFuncTermNode::stringify() const
{
  const char * name[] = {"if"};
  std::ostringstream s;
  s << name[_type] << '(' << *_left << ',' << *_middle << ',' << *_right << ')';
  return s.str();
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
  //if (narg != _eval_arguments.size())
  //  mooseError("EBFunction is used wth a different number of arguments than it was defined with.");

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
UNARY_FUNC_IMPLEMENT(sqrt, SQRT)

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

ExpressionBuilder::EBTerm::EBTermVector
ExpressionBuilder::EBTerm::CreateEBTermVector(const std::vector<const char *> symbol)
{
  EBTermVector vec;
  for(std::vector<const char *>::const_iterator it = symbol.begin();
      it != symbol.end(); ++it)
      vec.push_back(*it);
  return vec;
}

ExpressionBuilder::EBMatrixFunction::EBMatrixFunction()
{
  _rowNum = 0;
  _colNum = 0;
  setSize(0, 0);
}

ExpressionBuilder::EBMatrixFunction::EBMatrixFunction(std::vector<std::vector <ExpressionBuilder::EBTerm> > FunctionMatrix)
{
  this->FunctionMatrix = FunctionMatrix;
  _rowNum = FunctionMatrix.size();
  _colNum = FunctionMatrix[0].size();
  checkSize();
}

ExpressionBuilder::EBMatrixFunction::EBMatrixFunction(unsigned int i, unsigned int j)
{
  _rowNum = i;
  _colNum = j;
  setSize(i,j);
}

ExpressionBuilder::EBMatrixFunction &
operator*(const ExpressionBuilder::EBMatrixFunction & lhs, const ExpressionBuilder::EBMatrixFunction & rhs)
{
  ExpressionBuilder::EBMatrixFunction * result = new ExpressionBuilder::EBMatrixFunction();
  result->checkMultSize(lhs,rhs);
  result->setSize(lhs.rowNum(),rhs.colNum());
  for(unsigned int i = 0; i < lhs.rowNum(); ++i)
    for(unsigned int j = 0; j < rhs.rowNum(); ++j)
      for(unsigned int k = 0; k < rhs.colNum(); ++ k)
        (*result)[i][k] = (*result)[i][k] + lhs[i][j] * rhs[j][k];
  return *result;
}

ExpressionBuilder::EBMatrixFunction &
operator*(const Real & lhs, const ExpressionBuilder::EBMatrixFunction & rhs)
{
  ExpressionBuilder::EBMatrixFunction * result = new ExpressionBuilder::EBMatrixFunction();
  result->setSize(rhs.rowNum(),rhs.colNum());
  for(unsigned int i = 0; i < rhs.rowNum(); ++i)
    for(unsigned int j = 0; j < rhs.colNum(); ++j)
      (*result)[i][j] = lhs * rhs[i][j];
  return *result;
}

ExpressionBuilder::EBMatrixFunction &
operator*(const ExpressionBuilder::EBMatrixFunction & lhs, const Real & rhs)
{
  return rhs * lhs;
}

ExpressionBuilder::EBMatrixFunction &
operator*(const ExpressionBuilder::EBTerm & lhs, const ExpressionBuilder::EBMatrixFunction & rhs)
{
  ExpressionBuilder::EBMatrixFunction * result = new ExpressionBuilder::EBMatrixFunction();
  result->setSize(rhs.rowNum(),rhs.colNum());
  for(unsigned int i = 0; i < rhs.rowNum(); ++i)
    for(unsigned int j = 0; j < rhs.colNum(); ++j)
      (*result)[i][j] = lhs * rhs[i][j];
  return *result;
}

ExpressionBuilder::EBMatrixFunction &
operator*(const ExpressionBuilder::EBMatrixFunction & lhs, const ExpressionBuilder::EBTerm & rhs)
{
  return rhs * lhs;
}

ExpressionBuilder::EBMatrixFunction &
operator/(const ExpressionBuilder::EBMatrixFunction & lhs, const ExpressionBuilder::EBTerm & rhs)
{
  ExpressionBuilder::EBMatrixFunction * result = new ExpressionBuilder::EBMatrixFunction();
  result->setSize(lhs.rowNum(),lhs.colNum());
  for(unsigned int i = 0; i < lhs.rowNum(); ++i)
    for(unsigned int j = 0; j < lhs.colNum(); ++j)
      (*result)[i][j] = lhs[i][j] / rhs;
  return *result;
}

ExpressionBuilder::EBMatrixFunction &
operator/(const ExpressionBuilder::EBMatrixFunction & lhs, const Real & rhs)
{
  ExpressionBuilder::EBMatrixFunction * result = new ExpressionBuilder::EBMatrixFunction();
  result->setSize(lhs.rowNum(),lhs.colNum());
  for(unsigned int i = 0; i < lhs.rowNum(); ++i)
    for(unsigned int j = 0; j < lhs.colNum(); ++j)
      (*result)[i][j] = (1/rhs) * lhs[i][j];
  return *result;
}

ExpressionBuilder::EBMatrixFunction &
operator+(const ExpressionBuilder::EBMatrixFunction & lhs, const ExpressionBuilder::EBMatrixFunction & rhs)
{
  ExpressionBuilder::EBMatrixFunction * result = new ExpressionBuilder::EBMatrixFunction();
  result->checkAddSize(rhs,lhs);
  result->setSize(rhs.rowNum(),rhs.colNum());
  for(unsigned int i = 0; i < rhs.rowNum(); ++i)
    for(unsigned int j = 0; j < rhs.colNum(); ++j)
      (*result)[i][j] = rhs[i][j] + lhs[i][j];
  return *result;
}

ExpressionBuilder::EBMatrixFunction &
operator-(const ExpressionBuilder::EBMatrixFunction & lhs, const ExpressionBuilder::EBMatrixFunction & rhs)
{
  return lhs + (-1 * rhs);
}

//operator ExpressionBuilder::EBQuaternionFunction()

ExpressionBuilder::EBMatrixFunction::operator std::vector<std::vector<std::string> >() const
{
  std::vector<std::vector<std::string> > sVec;
  for(unsigned int i = 0; i < _rowNum; ++i)
  {
    std::vector<std::string> tempVec;
    for(unsigned int j = 0; j < _colNum; ++j)
      tempVec.push_back((*this)[i][j]);
    sVec.push_back(tempVec);
  }
  return sVec;
}

ExpressionBuilder::EBMatrixFunction &
ExpressionBuilder::EBMatrixFunction::operator+=(const ExpressionBuilder::EBMatrixFunction & rhs)
{
  return *this + rhs;
}

ExpressionBuilder::EBMatrixFunction &
ExpressionBuilder::EBMatrixFunction::operator-()
{
  return (-1) * (*this);
}

std::vector <ExpressionBuilder::EBTerm> &
ExpressionBuilder::EBMatrixFunction::operator[](unsigned int i)
{
  if(i < 0 || i > _rowNum)
  {} //MooseError

  return FunctionMatrix[i];
}

const std::vector <ExpressionBuilder::EBTerm> &
ExpressionBuilder::EBMatrixFunction::operator[](unsigned int i) const
{
  if(i < 0 || i > _rowNum)
  {} //MooseError

  return FunctionMatrix[i];
}

ExpressionBuilder::EBMatrixFunction &
ExpressionBuilder::EBMatrixFunction::operator!() //Implemented as a transpose function
{
  for(unsigned int i = 0; i < _rowNum; ++i)
    for(unsigned int j = i + 1; j < _colNum; ++j)
    {
      EBTerm temp = (*this)[i][j];
      (*this)[i][j] = (*this)[j][i];
      (*this)[j][i] = temp;
    }
  return *this;
}

void
ExpressionBuilder::EBMatrixFunction::checkSize()
{
  for(unsigned int i = 0; i < _rowNum; ++i)
    if(FunctionMatrix[i].size() != _colNum)
    {} //MooseError
}

unsigned int
ExpressionBuilder::EBMatrixFunction::rowNum() const
{
  return _rowNum;
}

unsigned int
ExpressionBuilder::EBMatrixFunction::colNum() const
{
  return _colNum;
}

void
ExpressionBuilder::EBMatrixFunction::setSize(const unsigned int i, const unsigned int j)
{
  FunctionMatrix.resize(i);
  for(unsigned int k = 0; k < i; k++)
    FunctionMatrix.resize(j);
}

void
ExpressionBuilder::EBMatrixFunction::checkMultSize(const ExpressionBuilder::EBMatrixFunction & lhs, const ExpressionBuilder::EBMatrixFunction & rhs)
{
  if(lhs.colNum() != rhs.rowNum())
  {} //MooseError
}

void
ExpressionBuilder::EBMatrixFunction::checkAddSize(const ExpressionBuilder::EBMatrixFunction & lhs, const ExpressionBuilder::EBMatrixFunction & rhs)
{
  if(lhs.colNum() != rhs.colNum())
  {} //MooseError
  if(lhs.rowNum() != rhs.rowNum())
  {} //MooseError
}

ExpressionBuilder::EBVectorFunction::EBVectorFunction(std::vector<EBTerm> FunctionVector)
{
  checkSize(FunctionVector);
  this->FunctionVector = FunctionVector;
}

ExpressionBuilder::EBVectorFunction::EBVectorFunction(std::vector<std::string> FunctionNameVector)
{
  checkSize(FunctionNameVector);
  for(int i = 0; i < 3; ++i)
    this->FunctionVector[i] = EBTerm(FunctionNameVector[i].c_str());
}

ExpressionBuilder::EBVectorFunction::operator std::vector<std::string>() const
{
  std::vector<std::string> sVec;
  for(unsigned int i = 0; i < 3; ++i)
    sVec.push_back((*this)[i]);
  return sVec;
}

ExpressionBuilder::EBTerm &
operator*(const ExpressionBuilder::EBVectorFunction & lhs, const ExpressionBuilder::EBVectorFunction & rhs)
{
  ExpressionBuilder::EBTerm * result = new ExpressionBuilder::EBTerm;
  for(unsigned int i = 0; i < 3; ++i)
    *result = *result + lhs[i] * rhs[i];
  return *result;
}

ExpressionBuilder::EBVectorFunction &
operator*(const ExpressionBuilder::EBVectorFunction & lhs, const Real & rhs)
{
  ExpressionBuilder::EBVectorFunction * result = new ExpressionBuilder::EBVectorFunction;
  for(unsigned int i = 0; i < 3; ++i)
    (*result)[i] = rhs * lhs[i];
  return *result;
}

ExpressionBuilder::EBVectorFunction &
operator*(const Real & lhs, const ExpressionBuilder::EBVectorFunction & rhs)
{
  return rhs * lhs;
}

ExpressionBuilder::EBVectorFunction &
operator*(const ExpressionBuilder::EBTerm & lhs, const ExpressionBuilder::EBVectorFunction & rhs)
{
  ExpressionBuilder::EBVectorFunction * result = new ExpressionBuilder::EBVectorFunction;
  for(unsigned int i = 0; i < 3; ++i)
    (*result)[i] = rhs[i] * lhs;
  return *result;
}

ExpressionBuilder::EBVectorFunction &
operator*(const ExpressionBuilder::EBVectorFunction & lhs, const ExpressionBuilder::EBTerm & rhs)
{
  return rhs * lhs;
}

ExpressionBuilder::EBVectorFunction &
operator*(const ExpressionBuilder::EBVectorFunction & lhs, const ExpressionBuilder::EBMatrixFunction & rhs) // We assume the vector is 1 x 3 here
{
  if(rhs.rowNum() != 3)
  {} // MooseError
  ExpressionBuilder::EBVectorFunction * result = new ExpressionBuilder::EBVectorFunction;
  for(unsigned int i = 0; i < 3; ++i)
    for(unsigned int j =0; j < 3; ++j)
      (*result)[i] = (*result)[i] + lhs[j] * rhs[j][i];
  return *result;
}

ExpressionBuilder::EBVectorFunction &
operator*(const ExpressionBuilder::EBMatrixFunction & lhs, const ExpressionBuilder::EBVectorFunction & rhs) // We assume the vector is 3 x 1 here
{
  if(lhs.colNum() != 3)
  {} // MooseError
  ExpressionBuilder::EBVectorFunction * result = new ExpressionBuilder::EBVectorFunction;
  for(unsigned int i = 0; i < 3; ++i)
    for(unsigned int j =0; j < 3; ++j)
      (*result)[i] = (*result)[i] + lhs[i][j] * rhs[j];
  return *result;
}

ExpressionBuilder::EBVectorFunction &
operator/(const ExpressionBuilder::EBVectorFunction & lhs, const Real & rhs)
{
  ExpressionBuilder::EBVectorFunction * result = new ExpressionBuilder::EBVectorFunction;
  for(unsigned int i = 0; i < 3; ++i)
  {
    (*result)[i] = (1/rhs) * lhs[i];
  }
  return *result;
}

ExpressionBuilder::EBVectorFunction &
operator/(const ExpressionBuilder::EBVectorFunction & lhs, const ExpressionBuilder::EBTerm & rhs)
{
  ExpressionBuilder::EBVectorFunction * result = new ExpressionBuilder::EBVectorFunction;
  for(unsigned int i = 0; i < 3; ++i)
    (*result)[i] = lhs[i] / rhs;
  return (*result);
}

ExpressionBuilder::EBVectorFunction &
operator+(const ExpressionBuilder::EBVectorFunction & lhs, const ExpressionBuilder::EBVectorFunction & rhs)
{
  ExpressionBuilder::EBVectorFunction * result = new ExpressionBuilder::EBVectorFunction;
  for(unsigned int i = 0; i < 3; ++i)
    (*result)[i] = lhs[i] + rhs[i];
  return *result;
}

ExpressionBuilder::EBVectorFunction &
operator-(const ExpressionBuilder::EBVectorFunction & lhs, const ExpressionBuilder::EBVectorFunction & rhs)
{
  return lhs + ((-1) * rhs);
}

ExpressionBuilder::EBVectorFunction &
ExpressionBuilder::EBVectorFunction::operator+=(const ExpressionBuilder::EBVectorFunction & rhs)
{
  return rhs + *this;
}

ExpressionBuilder::EBVectorFunction &
ExpressionBuilder::EBVectorFunction::operator-()
{
  return (-1) * (*this);
}

ExpressionBuilder::EBTerm &
ExpressionBuilder::EBVectorFunction::operator[](unsigned int i)
{
  return FunctionVector[i];
}

const ExpressionBuilder::EBTerm &
ExpressionBuilder::EBVectorFunction::operator[](unsigned int i) const
{
  return FunctionVector[i];
}

ExpressionBuilder::EBVectorFunction &
ExpressionBuilder::EBVectorFunction::cross(const ExpressionBuilder::EBVectorFunction & lhs, const ExpressionBuilder::EBVectorFunction & rhs) // This is defined as the cross product
{
  ExpressionBuilder::EBVectorFunction * result = new ExpressionBuilder::EBVectorFunction;
  (*result)[0] = lhs[1] * rhs[2] - lhs[2] * rhs[1];
  (*result)[1] = lhs[0] * rhs[2] - lhs[2] * rhs[0];
  (*result)[2] = lhs[0] * rhs[1] - lhs[1] * rhs[0];
  return *result;
}

ExpressionBuilder::EBTerm
ExpressionBuilder::EBVectorFunction::norm(const ExpressionBuilder::EBVectorFunction & rhs)
{
  return sqrt(rhs * rhs);
}

void
ExpressionBuilder::EBVectorFunction::checkSize(std::vector<EBTerm> FunctionVector)
{
  if(FunctionVector.size() != 3)
  {} // MooseError
}

void
ExpressionBuilder::EBVectorFunction::checkSize(std::vector<std::string> FunctionVector)
{
  if(FunctionVector.size() != 3)
  {} // MooseError
}

ExpressionBuilder::EBQuaternionFunction::EBQuaternionFunction(std::vector<ExpressionBuilder::EBTerm> FunctionQuat)
{
  this->FunctionQuat = FunctionQuat;
}

ExpressionBuilder::EBQuaternionFunction &
operator*(const ExpressionBuilder::EBQuaternionFunction & lhs, const ExpressionBuilder::EBQuaternionFunction & rhs)
{
  ExpressionBuilder::EBQuaternionFunction * result = new ExpressionBuilder::EBQuaternionFunction;
  (*result)[0] = lhs[0] * rhs[0] - lhs[1] * rhs[1] - lhs[2] * rhs[2] - lhs[3] * rhs[3];
  (*result)[1] = lhs[0] * rhs[1] + lhs[1] * rhs[0] + lhs[2] * rhs[3] - lhs[3] * rhs[2];
  (*result)[2] = lhs[0] * rhs[2] - lhs[1] * rhs[3] + lhs[2] * rhs[0] + lhs[3] * rhs[1];
  (*result)[3] = lhs[0] * rhs[3] + lhs[1] * rhs[2] - lhs[2] * rhs[1] + lhs[3] * rhs[0];
  return *result;
}

ExpressionBuilder::EBQuaternionFunction &
operator*(const ExpressionBuilder::EBQuaternionFunction & lhs, const Real & rhs)
{
  ExpressionBuilder::EBQuaternionFunction * result = new ExpressionBuilder::EBQuaternionFunction;
  for(unsigned int i = 0; i < 4; ++i)
  {
    (*result)[i] = lhs[i] * rhs;
  }
  return (*result);
}

ExpressionBuilder::EBQuaternionFunction &
operator*(const Real & lhs, const ExpressionBuilder::EBQuaternionFunction & rhs)
{
  return rhs * lhs;
}

ExpressionBuilder::EBQuaternionFunction &
operator*(const ExpressionBuilder::EBQuaternionFunction & lhs, const ExpressionBuilder::EBTerm & rhs)
{
  ExpressionBuilder::EBQuaternionFunction * result = new ExpressionBuilder::EBQuaternionFunction;
  for(unsigned int i = 0; i < 4; ++i)
  {
    (*result)[i] = lhs[i] * rhs;
  }
  return (*result);
}

ExpressionBuilder::EBQuaternionFunction &
operator*(const ExpressionBuilder::EBTerm & lhs, const ExpressionBuilder::EBQuaternionFunction & rhs)
{
  return rhs * lhs;
}

ExpressionBuilder::EBQuaternionFunction &
operator/(const ExpressionBuilder::EBQuaternionFunction & lhs, const Real & rhs)
{
  return (1/rhs) * lhs;
}

ExpressionBuilder::EBQuaternionFunction &
operator/(const ExpressionBuilder::EBQuaternionFunction & lhs, const ExpressionBuilder::EBTerm & rhs)
{
  return (1/rhs) * lhs;
}

ExpressionBuilder::EBQuaternionFunction &
operator+(const ExpressionBuilder::EBQuaternionFunction & lhs, const ExpressionBuilder::EBQuaternionFunction & rhs)
{
  ExpressionBuilder::EBQuaternionFunction * result = new ExpressionBuilder::EBQuaternionFunction;
  for(unsigned int i = 0; i < 4; ++i)
  {
    (*result)[i] = lhs[i] + rhs[i];
  }
  return (*result);
}

ExpressionBuilder::EBQuaternionFunction &
operator-(const ExpressionBuilder::EBQuaternionFunction & lhs, const ExpressionBuilder::EBQuaternionFunction & rhs)
{
  return lhs + ((-1) * rhs);
}

ExpressionBuilder::EBQuaternionFunction &
ExpressionBuilder::EBQuaternionFunction::operator-(const ExpressionBuilder::EBQuaternionFunction & rhs)
{
  return (-1) * rhs;
}

ExpressionBuilder::EBTerm &
ExpressionBuilder::EBQuaternionFunction::operator[](unsigned int i)
{
  return FunctionQuat[i];
}

const ExpressionBuilder::EBTerm &
ExpressionBuilder::EBQuaternionFunction::operator[](unsigned int i) const
{
  return FunctionQuat[i];
}

ExpressionBuilder::EBTerm
ExpressionBuilder::EBQuaternionFunction::norm(const ExpressionBuilder::EBQuaternionFunction & rhs)
{
  ExpressionBuilder::EBTerm temp;
  temp = rhs[0] * rhs[0];
  for(unsigned int i = 1; i < 4; ++i)
  {
    temp += rhs[i] * rhs[i];
  }
  return sqrt(temp);
}

void
ExpressionBuilder::EBQuaternionFunction::checkSize(std::vector<EBTerm> FunctionQuat)
{
  if(FunctionQuat.size() != 4)
  {} //MooseError
}
