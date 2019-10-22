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
  const char * name[] = {
      "sin", "cos", "tan", "abs", "log", "log2", "log10", "exp", "sinh", "cosh", "sqrt"};
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

/******************************
 *
 *  EBMatrix and EBVector after this point
 *
 ********************************/

ExpressionBuilder::EBMatrix::EBMatrix()
{
  _rowNum = 0;
  _colNum = 0;
  setSize(0, 0);
}

ExpressionBuilder::EBMatrix::EBMatrix(
    std::vector<std::vector<ExpressionBuilder::EBTerm>> FunctionMatrix)
{
  this->FunctionMatrix = FunctionMatrix;
  _rowNum = FunctionMatrix.size();
  _colNum = FunctionMatrix[0].size();
  checkSize();
}

ExpressionBuilder::EBMatrix::EBMatrix(unsigned int i, unsigned int j) { setSize(i, j); }

ExpressionBuilder::EBMatrix::EBMatrix(const EBTerm r,
                                      const EBTerm s,
                                      const EBTerm t,
                                      const EBTerm u,
                                      const EBTerm v,
                                      const EBTerm w,
                                      const EBTerm x,
                                      const EBTerm y,
                                      const EBTerm z)
{
  setSize(3, 3);
  this->FunctionMatrix[0][0] = r;
  this->FunctionMatrix[0][1] = s;
  this->FunctionMatrix[0][2] = t;
  this->FunctionMatrix[1][0] = u;
  this->FunctionMatrix[1][1] = v;
  this->FunctionMatrix[1][2] = w;
  this->FunctionMatrix[2][0] = x;
  this->FunctionMatrix[2][1] = y;
  this->FunctionMatrix[2][2] = z;
}

ExpressionBuilder::EBMatrix::EBMatrix(const RealTensorValue & rhs)
{
  this->setSize(3, 3);
  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      (*this)[i][j] = EBTerm(rhs(i, j));
}

ExpressionBuilder::EBMatrix & operator*(const ExpressionBuilder::EBMatrix & lhs,
                                        const ExpressionBuilder::EBMatrix & rhs)
{
  ExpressionBuilder::EBMatrix * result = new ExpressionBuilder::EBMatrix();
  result->checkMultSize(lhs, rhs);
  result->setSize(lhs.rowNum(), rhs.colNum());
  for (unsigned int i = 0; i < lhs.rowNum(); ++i)
    for (unsigned int j = 0; j < rhs.rowNum(); ++j)
      for (unsigned int k = 0; k < rhs.colNum(); ++k)
        (*result)[i][k] = (*result)[i][k] + lhs[i][j] * rhs[j][k];
  return *result;
}

ExpressionBuilder::EBMatrix & operator*(const Real & lhs, const ExpressionBuilder::EBMatrix & rhs)
{
  ExpressionBuilder::EBMatrix * result = new ExpressionBuilder::EBMatrix();
  result->setSize(rhs.rowNum(), rhs.colNum());
  for (unsigned int i = 0; i < rhs.rowNum(); ++i)
    for (unsigned int j = 0; j < rhs.colNum(); ++j)
      (*result)[i][j] = lhs * rhs[i][j];
  return *result;
}

ExpressionBuilder::EBMatrix & operator*(const ExpressionBuilder::EBMatrix & lhs, const Real & rhs)
{
  return rhs * lhs;
}

ExpressionBuilder::EBMatrix & operator*(const ExpressionBuilder::EBTerm & lhs,
                                        const ExpressionBuilder::EBMatrix & rhs)
{
  ExpressionBuilder::EBMatrix * result = new ExpressionBuilder::EBMatrix();
  result->setSize(rhs.rowNum(), rhs.colNum());
  for (unsigned int i = 0; i < rhs.rowNum(); ++i)
    for (unsigned int j = 0; j < rhs.colNum(); ++j)
      (*result)[i][j] = lhs * rhs[i][j];
  return *result;
}

ExpressionBuilder::EBMatrix & operator*(const ExpressionBuilder::EBMatrix & lhs,
                                        const ExpressionBuilder::EBTerm & rhs)
{
  return rhs * lhs;
}

ExpressionBuilder::EBMatrix &
operator+(const ExpressionBuilder::EBMatrix & lhs, const ExpressionBuilder::EBMatrix & rhs)
{
  ExpressionBuilder::EBMatrix * result = new ExpressionBuilder::EBMatrix();
  result->checkAddSize(rhs, lhs);
  result->setSize(rhs.rowNum(), rhs.colNum());
  for (unsigned int i = 0; i < rhs.rowNum(); ++i)
    for (unsigned int j = 0; j < rhs.colNum(); ++j)
      (*result)[i][j] = rhs[i][j] + lhs[i][j];
  return *result;
}

ExpressionBuilder::EBMatrix &
operator-(const ExpressionBuilder::EBMatrix & lhs, const ExpressionBuilder::EBMatrix & rhs)
{
  return lhs + (-1 * rhs);
}

ExpressionBuilder::EBMatrix::operator std::vector<std::vector<std::string>>() const
{
  std::vector<std::vector<std::string>> sVec;
  for (unsigned int i = 0; i < _rowNum; ++i)
  {
    std::vector<std::string> tempVec;
    for (unsigned int j = 0; j < _colNum; ++j)
      tempVec.push_back((*this)[i][j]);
    sVec.push_back(tempVec);
  }
  return sVec;
}

ExpressionBuilder::EBMatrix &
ExpressionBuilder::EBMatrix::operator+=(const ExpressionBuilder::EBMatrix & rhs)
{
  return *this + rhs;
}

ExpressionBuilder::EBMatrix &
ExpressionBuilder::EBMatrix::operator-()
{
  return (-1) * (*this);
}

std::vector<ExpressionBuilder::EBTerm> & ExpressionBuilder::EBMatrix::operator[](unsigned int i)
{
  if (i > _rowNum)
  {
  } // MooseError

  return FunctionMatrix[i];
}

const std::vector<ExpressionBuilder::EBTerm> & ExpressionBuilder::EBMatrix::
operator[](unsigned int i) const
{
  if (i > _rowNum)
  {
  } // MooseError

  return FunctionMatrix[i];
}

ExpressionBuilder::EBMatrix
ExpressionBuilder::EBMatrix::transpose()
{
  for (unsigned int i = 0; i < _rowNum; ++i)
    for (unsigned int j = i + 1; j < _colNum; ++j)
    {
      ExpressionBuilder::EBTerm temp = (*this)[i][j];
      (*this)[i][j] = (*this)[j][i];
      (*this)[j][i] = temp;
    }
  return *this;
}

void
ExpressionBuilder::EBMatrix::checkSize()
{
  for (unsigned int i = 0; i < _rowNum; ++i)
    if (FunctionMatrix[i].size() != _colNum)
    {
    } // MooseError
}

unsigned int
ExpressionBuilder::EBMatrix::rowNum() const
{
  return _rowNum;
}

unsigned int
ExpressionBuilder::EBMatrix::colNum() const
{
  return _colNum;
}

void
ExpressionBuilder::EBMatrix::setSize(const unsigned int i, const unsigned int j)
{
  _rowNum = i;
  _colNum = j;
  FunctionMatrix.resize(i);
  for (unsigned int k = 0; k < i; k++)
    for (unsigned int l = 0; l < j; ++l)
      FunctionMatrix[k].push_back(EBTerm(0));
}

void
ExpressionBuilder::EBMatrix::checkMultSize(const ExpressionBuilder::EBMatrix & lhs,
                                           const ExpressionBuilder::EBMatrix & rhs)
{
  if (lhs.colNum() != rhs.rowNum())
  {
  } // MooseError
}

void
ExpressionBuilder::EBMatrix::checkAddSize(const ExpressionBuilder::EBMatrix & lhs,
                                          const ExpressionBuilder::EBMatrix & rhs)
{
  if (lhs.colNum() != rhs.colNum())
  {
  } // MooseError
  if (lhs.rowNum() != rhs.rowNum())
  {
  } // MooseError
}

ExpressionBuilder::EBVector::EBVector()
{
  for (int i = 0; i < 3; ++i)
  {
    FunctionVector.push_back(EBTerm(0));
  }
}

ExpressionBuilder::EBVector::EBVector(std::vector<EBTerm> FunctionVector)
{
  checkSize(FunctionVector);
  this->FunctionVector = FunctionVector;
}

ExpressionBuilder::EBVector::EBVector(std::vector<std::string> FunctionNameVector)
{
  checkSize(FunctionNameVector);
  for (int i = 0; i < 3; ++i)
    this->FunctionVector.push_back(EBTerm(FunctionNameVector[i].c_str()));
}

ExpressionBuilder::EBVector &
ExpressionBuilder::EBVector::operator=(const RealVectorValue & rhs)
{
  EBVector * result = new EBVector;
  for (unsigned int i = 0; i < 3; ++i)
  {
    (*result)[i] = EBTerm(rhs(i));
  }
  return (*result);
}

ExpressionBuilder::EBVector::EBVectorVector
ExpressionBuilder::EBVector::CreateEBVectorVector(const std::string & var_name,
                                                  unsigned int _op_num)
{
  EBVectorVector vec;
  for (unsigned int i = 0; i < _op_num; ++i)
  {
    EBVector temp;
    EBTerm tempx((var_name + std::to_string(i) + "x").c_str());
    EBTerm tempy((var_name + std::to_string(i) + "y").c_str());
    EBTerm tempz((var_name + std::to_string(i) = "z").c_str());
    temp.push_back(tempx);
    temp.push_back(tempy);
    temp.push_back(tempz);
    vec.push_back(temp);
  }
  return vec;
}

ExpressionBuilder::EBVector::operator std::vector<std::string>() const
{
  std::vector<std::string> sVec;
  for (unsigned int i = 0; i < 3; ++i)
    sVec.push_back((*this)[i]);
  return sVec;
}

ExpressionBuilder::EBTerm & operator*(const ExpressionBuilder::EBVector & lhs,
                                      const ExpressionBuilder::EBVector & rhs)
{
  ExpressionBuilder::EBTerm * result = new ExpressionBuilder::EBTerm(0);
  for (unsigned int i = 0; i < 3; ++i)
    *result = *result + lhs[i] * rhs[i];
  return *result;
}

ExpressionBuilder::EBVector & operator*(const ExpressionBuilder::EBVector & lhs, const Real & rhs)
{
  ExpressionBuilder::EBVector * result = new ExpressionBuilder::EBVector;
  for (unsigned int i = 0; i < 3; ++i)
    (*result)[i] = rhs * lhs[i];
  return *result;
}

ExpressionBuilder::EBVector & operator*(const Real & lhs, const ExpressionBuilder::EBVector & rhs)
{
  return rhs * lhs;
}

ExpressionBuilder::EBVector & operator*(const ExpressionBuilder::EBTerm & lhs,
                                        const ExpressionBuilder::EBVector & rhs)
{
  ExpressionBuilder::EBVector * result = new ExpressionBuilder::EBVector;
  for (unsigned int i = 0; i < 3; ++i)
    (*result)[i] = rhs[i] * lhs;
  return *result;
}

ExpressionBuilder::EBVector & operator*(const ExpressionBuilder::EBVector & lhs,
                                        const ExpressionBuilder::EBTerm & rhs)
{
  return rhs * lhs;
}

ExpressionBuilder::EBVector &
operator*(const ExpressionBuilder::EBVector & lhs,
          const ExpressionBuilder::EBMatrix & rhs) // We assume the vector is 1 x 3 here
{
  if (rhs.rowNum() != 3)
  {
  } // MooseError
  ExpressionBuilder::EBVector * result = new ExpressionBuilder::EBVector;
  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      (*result)[i] = (*result)[i] + lhs[j] * rhs[j][i];
  return *result;
}

ExpressionBuilder::EBVector &
operator*(const ExpressionBuilder::EBMatrix & lhs,
          const ExpressionBuilder::EBVector & rhs) // We assume the vector is 3 x 1 here
{
  if (lhs.colNum() != 3)
  {
  } // MooseError
  ExpressionBuilder::EBVector * result = new ExpressionBuilder::EBVector;
  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      (*result)[i] = (*result)[i] + lhs[i][j] * rhs[j];
  return *result;
}

ExpressionBuilder::EBVector &
operator+(const ExpressionBuilder::EBVector & lhs, const ExpressionBuilder::EBVector & rhs)
{
  ExpressionBuilder::EBVector * result = new ExpressionBuilder::EBVector;
  for (unsigned int i = 0; i < 3; ++i)
    (*result)[i] = lhs[i] + rhs[i];
  return *result;
}

ExpressionBuilder::EBVector &
operator-(const ExpressionBuilder::EBVector & lhs, const ExpressionBuilder::EBVector & rhs)
{
  return lhs + ((-1) * rhs);
}

ExpressionBuilder::EBVector &
ExpressionBuilder::EBVector::operator+=(const ExpressionBuilder::EBVector & rhs)
{
  return rhs + *this;
}

ExpressionBuilder::EBVector &
ExpressionBuilder::EBVector::operator-()
{
  return (-1) * (*this);
}

ExpressionBuilder::EBTerm & ExpressionBuilder::EBVector::operator[](unsigned int i)
{
  return FunctionVector[i];
}

const ExpressionBuilder::EBTerm & ExpressionBuilder::EBVector::operator[](unsigned int i) const
{
  return FunctionVector[i];
}

ExpressionBuilder::EBVector
ExpressionBuilder::EBVector::cross(const ExpressionBuilder::EBVector & lhs,
                                   const ExpressionBuilder::EBVector & rhs)
{
  ExpressionBuilder::EBVector * result = new ExpressionBuilder::EBVector;
  (*result)[0] = lhs[1] * rhs[2] - lhs[2] * rhs[1];
  (*result)[1] = lhs[0] * rhs[2] - lhs[2] * rhs[0];
  (*result)[2] = lhs[0] * rhs[1] - lhs[1] * rhs[0];
  return *result;
}

ExpressionBuilder::EBTerm
ExpressionBuilder::EBVector::norm()
{
  return sqrt(*this * *this);
}

void
ExpressionBuilder::EBVector::push_back(EBTerm term)
{
  FunctionVector.push_back(term);
  if (FunctionVector.size() > 3)
  {
  } // MooseError
}

void
ExpressionBuilder::EBVector::checkSize(std::vector<EBTerm> FunctionVector)
{
  if (FunctionVector.size() != 3)
  {
  } // MooseError
}

void
ExpressionBuilder::EBVector::checkSize(std::vector<std::string> FunctionVector)
{
  if (FunctionVector.size() != 3)
  {
  } // MooseError
}
