#include "ExpressionBuilder.h"

#include "MooseError.h"

ExpressionBuilder::EBTermList
ExpressionBuilder::EBTerm::operator, (const ExpressionBuilder::EBTerm & rarg)
{
  EBTermList list(2);
  list[0] = *this;
  list[1] = rarg;
  return list;
}

ExpressionBuilder::EBTermList
ExpressionBuilder::EBTerm::operator, (const ExpressionBuilder::EBTermList & rargs)
{
  EBTermList list(1);
  list[1] = *this;
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
  return os << *(term.root);
};

std::string
ExpressionBuilder::EBSymbolNode::stringify() const
{
  return symbol;
};

std::string
ExpressionBuilder::EBUnaryFuncTermNode::stringify() const
{
  const char * ufunc[] = { "sin", "cos", "tan", "abs", "log", "log2", "log10", "exp", "sinh", "cosh" };
  std::ostringstream s;
  s << ufunc[type] << '(' << *subnode << ')';
  return s.str();
};

std::string
ExpressionBuilder::EBUnaryOpTermNode::stringify() const
{
  const char * uop[] = { "-", "!" };
  std::ostringstream s;

  s <<  uop[type];

  if (subnode->precedence() > precedence())
    s << '(' << *subnode << ')';
  else
    s << *subnode;

  return s.str();
};

std::string
ExpressionBuilder::EBBinaryFuncTermNode::stringify() const
{
  const char * bfunc[] = { "min", "max", "atan2", "hypot" };
  std::ostringstream s;
  s << bfunc[type] << '(' << *left << ',' << *right << ')';
  return s.str();
};

std::string
ExpressionBuilder::EBBinaryOpTermNode::stringify() const
{
  const char * bop[] = { "+", "-", "*", "/", "%", "^" };
  std::ostringstream s;

  if (left->precedence() > precedence())
    s << '(' << *left << ')';
  else
    s << *left;

  s << bop[type];

  if (right->precedence() > precedence())
    s << '(' << *right << ')';
  else
    s << *right;

  return s.str();
};

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
  }

  mooseError("Unknown type.");
}

ExpressionBuilder::EBFunction::EBFunction(const ExpressionBuilder::EBTerm & arg)
{
  this->eval_arguments.resize(1);
  this->eval_arguments[0] = arg;
}

ExpressionBuilder::EBFunction::EBFunction(const ExpressionBuilder::EBTermList & args)
{
  this->eval_arguments = args;
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
  this->term = EBTerm(term);
  return *this;
}

ExpressionBuilder::EBFunction::operator ExpressionBuilder::EBTerm() const
{
  EBTerm result(term);
  result.substitute(arguments, eval_arguments);
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

// this is a function in ExpressionBuilder (pow) but an operator in FParser (^)
ExpressionBuilder::EBTerm pow(const ExpressionBuilder::EBTerm & left, const ExpressionBuilder::EBTerm & right) {
  return ExpressionBuilder::EBTerm(
    new ExpressionBuilder::EBBinaryOpTermNode(
      left.root->clone(), right.root->clone(), ExpressionBuilder::EBBinaryOpTermNode::POW
    )
  );
}

unsigned int
ExpressionBuilder::EBBinaryTermNode::substitute(const std::vector<std::string> & find_str, EBTermNodeList replace)
{
  std::string left_str = left->stringify();
  std::string right_str = right->stringify();
  unsigned int nfind = find_str.size();
  unsigned int success = 0;

  for (unsigned int i = 0; i < nfind; ++i)
  {
    if (left_str == find_str[i])
    {
      delete left;
      left = replace[i]->clone();
      success = 1;
      break;
    }
  }

  if (success == 0)
    success += left->substitute(find_str, replace);

  for (unsigned int i = 0; i < nfind; ++i)
  {
    if (right_str == find_str[i])
    {
      delete right;
      right = replace[i]->clone();
      return success + 1;
    }
  }

  return success + right->substitute(find_str, replace);
}

unsigned int
ExpressionBuilder::EBUnaryTermNode::substitute(const std::vector<std::string> & find_str, EBTermNodeList replace)
{
  std::string subnode_str = subnode->stringify();
  unsigned int nfind = find_str.size();

  for (unsigned int i = 0; i < nfind; ++i)
  {
    if (subnode_str == find_str[i])
    {
      delete subnode;
      subnode = replace[i]->clone();
      return 1;
    }
  }

  return subnode->substitute(find_str, replace);
}

unsigned int
ExpressionBuilder::EBTerm::substitute(const EBTerm & find, const EBTerm & replace)
{
  EBTermList find_list(1), replace_list(1);
  find_list[0] = find;
  replace_list[0] = replace;
  return substitute(find_list, replace_list);
}

unsigned int
ExpressionBuilder::EBTerm::substitute(const EBTermList & find, const EBTermList & replace)
{
  unsigned int nfind = find.size();
  if (nfind != replace.size())
    mooseError("Find and replace EBTerm lists must have equal length.");

  std::string root_str = root->stringify();

  EBTermNodeList replace_terms(nfind);
  std::vector<std::string> find_str(nfind);

  for (unsigned int i = 0; i < nfind; ++i)
  {
    find_str[i] = find[i].root->stringify();
    replace_terms[i] = replace[i].root;

    if (root_str == find_str[i])
    {
      delete root;
      root = replace_terms[i]->clone();
      return 1;
    }
  }

  return root->substitute(find_str, replace_terms);
}
