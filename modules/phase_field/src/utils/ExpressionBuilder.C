#include "ExpressionBuilder.h"

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
  // THIS leaks!
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
  unsigned int narg = arguments.size();

  if (narg == eval_arguments.size())
    // TODO: this substitution is dangerously wrong! h(a,b,c) = a+2*b+3*c; h(b,c,c) -> c+2*c+3*c !!!
    for (unsigned i = 0; i < narg; ++i)
      result.substitute(arguments[i], eval_arguments[i]);
  // else ERROR;

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
ExpressionBuilder::EBBinaryTermNode::substitute(const std::string & find_str, EBTermNode * replace)
{
  unsigned int success = 0;

  if (left->stringify() == find_str)
  {
    delete left;
    left = replace->clone();
    success++;
  }
  else
    success += left->substitute(find_str, replace);

  if (right->stringify() == find_str)
  {
    delete right;
    right = replace->clone();
    success++;
  }
  else
    success += right->substitute(find_str, replace);

  return success;
}

unsigned int
ExpressionBuilder::EBUnaryTermNode::substitute(const std::string & find_str, EBTermNode * replace)
{
  if (subnode->stringify() == find_str)
  {
    delete subnode;
    subnode = replace->clone();
    return 1;
  }
  else
    return subnode->substitute(find_str, replace);
}

unsigned int
ExpressionBuilder::EBTerm::substitute(const EBTerm & find, const EBTerm & replace)
{
  std::string find_str = find.root->stringify();

  if (root->stringify() == find_str)
  {
    delete root;
    root = replace.root->clone();
    return 1;
  }
  else
    return root->substitute(find_str, replace.root);
}
