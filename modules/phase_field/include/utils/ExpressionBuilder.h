/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef EXPRESSIONBUILDER_H
#define EXPRESSIONBUILDER_H

#include <vector>
#include <ostream>
#include <sstream>
#include <iomanip>

#include "libmesh/libmesh_common.h"

using namespace libMesh;

/**
 * ExpressionBuilder adds an interface to derived classes that enables
 * convenient construction of FParser expressions through operator overloading.
 * It exposes the new types EBTerm and EBFunction
 * Variables used in your expressions are of type EBTerm. The following declares
 * three variables that can be used in an expression:
 *
 * EBTerm c1("c1"), c2("c3"), phi("phi");
 *
 * Declare a function 'G' and define it. Note the double bracket syntax '(())'':
 *
 * EBFunction G;
 * G((c1, c2, c3)) = c1 + 2 * c2 + 3 * pow(c3, 2);
 *
 * Performing a substitution is as easy as:
 * EBFunction H;
 * H((c1, c2)) = G((c1, c2, 1-c1-c2))
 *
 * Use the ```<<``` io operator to output functions or terms. Or use explicit or
 * implicit casts from EBFunction to std::string``` to pass a function to the
 * FParser Parse method. FParser variables are built using the ```args()``` method.
 *
 * FunctionParserADBase<Real> GParser;
 * GParser.Parse(G, G.args);
 */
class ExpressionBuilder
{
public:
  ExpressionBuilder() {};

  // forward delcarations
  class EBTerm;
  class EBTermNode;
  class EBFunction;
  class EBSubstitutionRule;
  typedef std::vector<EBTerm> EBTermList;
  typedef std::vector<EBTermNode *> EBTermNodeList;
  typedef std::vector<const EBSubstitutionRule *> EBSubstitutionRuleList;

  // Base class for nodes in the expression tree
  class EBTermNode
  {
  public:
    virtual ~EBTermNode() {};
    virtual EBTermNode * clone() const = 0;

    virtual std::string stringify() const = 0;
    virtual unsigned int substitute(const EBSubstitutionRuleList & /*rule*/) { return 0; }
    virtual int precedence() const = 0;
    friend std::ostream& operator<< (std::ostream & os, const EBTermNode & node) { return os << node.stringify(); }
  };

  // Template class for leaf nodes holding numbers in the expression tree
  template<typename T>
  class EBNumberNode : public EBTermNode
  {
    T value;

  public:
    EBNumberNode(T _value) : value(_value) {};
    virtual EBNumberNode<T> * clone() const { return new EBNumberNode(value); }

    virtual std::string stringify() const;
    virtual int precedence() const { return 0; }
  };

  // Template class for leaf nodes holding symbols (i.e. variables) in the expression tree
  class EBSymbolNode : public EBTermNode
  {
    std::string symbol;

  public:
    EBSymbolNode(std::string _symbol) : symbol(_symbol) {};
    virtual EBSymbolNode * clone() const { return new EBSymbolNode(symbol); }

    virtual std::string stringify() const;
    virtual int precedence() const { return 0; }
  };

  // Base class for nodes with a single sub node (i.e. functions or operators taking one argument)
  class EBUnaryTermNode : public EBTermNode
  {
  public:
    EBUnaryTermNode(EBTermNode * _subnode) : subnode(_subnode) {};
    virtual ~EBUnaryTermNode() { delete subnode; };

    virtual unsigned int substitute(const EBSubstitutionRuleList & rule);
    const EBTermNode * getSubnode() const { return subnode; }

  protected:
    EBTermNode *subnode;
  };

  // Node representing a function with two arguments
  class EBUnaryFuncTermNode : public EBUnaryTermNode
  {
  public:
    enum NodeType { SIN, COS, TAN, ABS, LOG, LOG2, LOG10, EXP, SINH, COSH } type;

    EBUnaryFuncTermNode(EBTermNode * _subnode, NodeType _type) :
      EBUnaryTermNode(_subnode), type(_type) {};
    virtual EBUnaryFuncTermNode * clone() const {
      return new EBUnaryFuncTermNode(subnode->clone(), type);
    };

    virtual std::string stringify() const;
    virtual int precedence() const { return 2; }
  };

  // Node representing a unary operator
  class EBUnaryOpTermNode : public EBUnaryTermNode
  {
  public:
    enum NodeType { NEG, LOGICNOT } type;

    EBUnaryOpTermNode(EBTermNode * _subnode, NodeType _type) :
      EBUnaryTermNode(_subnode), type(_type) {};
    virtual EBUnaryOpTermNode * clone() const {
      return new EBUnaryOpTermNode(subnode->clone(), type);
    };

    virtual std::string stringify() const;
    virtual int precedence() const { return 3; }
  };

  // Base class for nodes with two sub nodes (i.e. functions or operators taking two arguments)
  class EBBinaryTermNode : public EBTermNode
  {
  public:
    EBBinaryTermNode(EBTermNode * _left, EBTermNode * _right) : left(_left), right(_right) {};
    virtual ~EBBinaryTermNode() { delete left; delete right; };

    virtual unsigned int substitute(const EBSubstitutionRuleList & rule);

  protected:
    EBTermNode *left, *right;
  };

  // Node representing a binary operator
  class EBBinaryOpTermNode : public EBBinaryTermNode
  {
  public:
    enum NodeType { ADD, SUB, MUL, DIV, MOD, POW, LESS, GREATER, LESSEQ, GREATEREQ, EQ, NOTEQ };

    EBBinaryOpTermNode(EBTermNode * _left, EBTermNode * _right, NodeType _type) :
      EBBinaryTermNode(_left, _right), type(_type) {};
    virtual EBBinaryOpTermNode * clone() const {
      return new EBBinaryOpTermNode(left->clone(), right->clone(), type);
    };

    virtual std::string stringify() const;
    virtual int precedence() const;

  protected:
    NodeType type;
  };

  // Node representing a function with two arguments
  class EBBinaryFuncTermNode : public EBBinaryTermNode
  {
  public:
    enum NodeType { MIN, MAX, ATAN2, HYPOT, PLOG } type;

    EBBinaryFuncTermNode(EBTermNode * _left, EBTermNode * _right, NodeType _type) :
      EBBinaryTermNode(_left, _right), type(_type) {};
    virtual EBBinaryFuncTermNode * clone() const {
      return new EBBinaryFuncTermNode(left->clone(), right->clone(), type);
    };

    virtual std::string stringify() const;
    virtual int precedence() const { return 2; }
  };

  // Base class for nodes with two sub nodes (i.e. functions or operators taking two arguments)
  class EBTernaryTermNode : public EBBinaryTermNode
  {
  public:
    EBTernaryTermNode(EBTermNode * _left, EBTermNode * _middle, EBTermNode * _right) : EBBinaryTermNode(_left, _right), middle(_middle) {};
    virtual ~EBTernaryTermNode() { delete middle; };

    virtual unsigned int substitute(const EBSubstitutionRuleList & rule);

  protected:
    EBTermNode *middle;
  };

  // Node representing a function with three arguments
  class EBTernaryFuncTermNode : public EBTernaryTermNode
  {
  public:
    enum NodeType { CONDITIONAL } type;

    EBTernaryFuncTermNode(EBTermNode * _left, EBTermNode * _middle, EBTermNode * _right, NodeType _type) :
      EBTernaryTermNode(_left, _middle, _right), type(_type) {};
    virtual EBTernaryFuncTermNode * clone() const {
      return new EBTernaryFuncTermNode(left->clone(), middle->clone(), right->clone(), type);
    };

    virtual std::string stringify() const;
    virtual int precedence() const { return 2; }
  };


  // substitution rule functor base class to perform flexible term substitutions
  class EBSubstitutionRule {
  public:
    virtual EBTermNode * apply(const EBTermNode *) const = 0;
    virtual ~EBSubstitutionRule() {}
  };

  template<class Node_T>
  class EBSubstitutionRuleTyped : public EBSubstitutionRule {
  public:
    virtual EBTermNode * apply(const EBTermNode *) const;
  protected:
    // on successful substitution this returns a new node to replace toe old one, otherwise it returns NULL
    virtual EBTermNode * substitute(const Node_T &) const = 0;
  };

  class EBTermSubstitution : public EBSubstitutionRuleTyped<EBSymbolNode> {
  public:
    EBTermSubstitution(const EBTerm & _find, const EBTerm & _replace);
    virtual ~EBTermSubstitution() { delete replace; }
  protected:
    virtual EBTermNode * substitute(const EBSymbolNode &) const;
    std::string find;
    EBTermNode * replace;
  };

  class EBLogPlogSubstitution : public EBSubstitutionRuleTyped<EBUnaryFuncTermNode> {
  public:
    EBLogPlogSubstitution(const EBTerm & _epsilon) : epsilon(_epsilon.getRoot()->clone()) {}
    virtual ~EBLogPlogSubstitution() { delete epsilon; }
  protected:
    virtual EBTermNode * substitute(const EBUnaryFuncTermNode &) const;
    EBTermNode * epsilon;
  };


  // User facing host object for an expression tree
  class EBTerm
  {
  public:
    EBTerm() : root(NULL) {};
    EBTerm(const EBTerm & term) : root(term.root==NULL ? NULL : term.root->clone()) {};
    ~EBTerm() { delete root; };

    // construct a term from a node
    EBTerm(EBTermNode * _root) : root(_root) {};

    // construct from number or string
    EBTerm(int number) : root(new EBNumberNode<int>(number)) {}
    EBTerm(Real number) : root(new EBNumberNode<Real>(number)) {}
    EBTerm(const char *symbol) : root(new EBSymbolNode(symbol)) {}

    // concatenate terms to form a parameter list with (()) syntax (those need to be out-of-class!)
    friend EBTermList operator, (const ExpressionBuilder::EBTerm & larg, const ExpressionBuilder::EBTerm & rarg);
    friend EBTermList operator, (const ExpressionBuilder::EBTerm & larg, const ExpressionBuilder::EBTermList & rargs);
    friend EBTermList operator, (const ExpressionBuilder::EBTermList & largs, const ExpressionBuilder::EBTerm & rarg);

    // dump term as FParser expression
    friend std::ostream & operator<< (std::ostream & os, const EBTerm & term);
    // cast into a string
    operator std::string() const { return root->stringify(); }

    // assign a term
    EBTerm & operator= (const EBTerm & term) { delete root; root = term.root==NULL ? NULL : term.root->clone(); return *this; }

    // perform a substitution (returns substituton count)
    unsigned int substitute(const EBSubstitutionRule & rule);
    unsigned int substitute(const EBSubstitutionRuleList & rules);

    const EBTermNode * getRoot() const { return root; }

  protected:
    EBTermNode *root;

  public:
    /**
     * Unary operators
     */
    #define UNARY_OP_IMPLEMENT(op,OP) \
    EBTerm operator op () { \
      return EBTerm(new EBUnaryOpTermNode(root->clone(), EBUnaryOpTermNode::OP)); \
    }
    UNARY_OP_IMPLEMENT(-,NEG)
    UNARY_OP_IMPLEMENT(!,LOGICNOT)

    /**
     * Unary functions
     */
    friend EBTerm sin(const EBTerm &);
    friend EBTerm cos(const EBTerm &);
    friend EBTerm tan(const EBTerm &);
    friend EBTerm abs(const EBTerm &);
    friend EBTerm log(const EBTerm &);
    friend EBTerm log2(const EBTerm &);
    friend EBTerm log10(const EBTerm &);
    friend EBTerm exp(const EBTerm &);
    friend EBTerm sinh(const EBTerm &);
    friend EBTerm cosh(const EBTerm &);

    /**
     * Binary operators (including number,term operations)
     */
    #define BINARY_OP_IMPLEMENT(op,OP) \
    EBTerm operator op (const EBTerm & term) { \
      return EBTerm(new EBBinaryOpTermNode(root->clone(), term.root->clone(), EBBinaryOpTermNode::OP)); \
    } \
    friend EBTerm operator op (int left, const EBTerm & right) { \
      return EBTerm(new EBBinaryOpTermNode(new EBNumberNode<int>(left), right.root->clone(), EBBinaryOpTermNode::OP)); \
    } \
    friend EBTerm operator op (Real left, const EBTerm & right) { \
      return EBTerm(new EBBinaryOpTermNode(new EBNumberNode<Real>(left), right.root->clone(), EBBinaryOpTermNode::OP)); \
    } \
    friend EBTerm operator op (const EBFunction & left, const EBTerm & right) { \
      return EBTerm(new EBBinaryOpTermNode(EBTerm(left).root->clone(), right.root->clone(), EBBinaryOpTermNode::OP)); \
    } \
    friend EBTerm operator op (const EBFunction & left, const EBFunction & right); \
    friend EBTerm operator op (int left, const EBFunction & right); \
    friend EBTerm operator op (Real left, const EBFunction & right);
    BINARY_OP_IMPLEMENT(+,ADD)
    BINARY_OP_IMPLEMENT(-,SUB)
    BINARY_OP_IMPLEMENT(*,MUL)
    BINARY_OP_IMPLEMENT(/,DIV)
    BINARY_OP_IMPLEMENT(%,MOD)
    BINARY_OP_IMPLEMENT(<,LESS)
    BINARY_OP_IMPLEMENT(>,GREATER)
    BINARY_OP_IMPLEMENT(<=,LESSEQ)
    BINARY_OP_IMPLEMENT(>=,GREATEREQ)
    BINARY_OP_IMPLEMENT(==,EQ)
    BINARY_OP_IMPLEMENT(!=,NOTEQ)

    /**
     * Binary functions
     */
    friend EBTerm min(const EBTerm &, const EBTerm &);
    friend EBTerm max(const EBTerm &, const EBTerm &);
    friend EBTerm pow(const EBTerm &, const EBTerm &);
    template<typename T> friend EBTerm pow(const EBTerm &, T exponent);
    friend EBTerm atan2(const EBTerm &, const EBTerm &);
    friend EBTerm hypot(const EBTerm &, const EBTerm &);
    friend EBTerm plog(const EBTerm &, const EBTerm &);

    /**
     * Ternary functions
     */
    friend EBTerm conditional(const EBTerm &, const EBTerm &, const EBTerm &);
  };

  // User facing host object for a function. This combines a term with an argument list.
  class EBFunction
  {
  public:
    EBFunction() {};

    // set the temporary argument list which is either used for evaluation
    // or committed to the argument list upon function definition (assignment)
    EBFunction & operator() (const EBTerm & arg);
    EBFunction & operator() (const EBTermList & args);

    // convenience operators to allow single bracket syntax
    EBFunction & operator() (const EBTerm & a1, const EBTerm & a2) { return (*this)((a1,a2)); }
    EBFunction & operator() (const EBTerm & a1, const EBTerm & a2, const EBTerm & a3) { return (*this)((a1,a2,a3)); }
    EBFunction & operator() (const EBTerm & a1, const EBTerm & a2, const EBTerm & a3, const EBTerm & a4) { return (*this)((a1,a2,a3,a4)); }
    EBFunction & operator() (const EBTerm & a1, const EBTerm & a2, const EBTerm & a3, const EBTerm & a4, const EBTerm & a5) { return (*this)((a1,a2,a3,a4,a5)); }
    EBFunction & operator() (const EBTerm & a1, const EBTerm & a2, const EBTerm & a3, const EBTerm & a4, const EBTerm & a5, const EBTerm & a6) { return (*this)((a1,a2,a3,a4,a5,a6)); }
    EBFunction & operator() (const EBTerm & a1, const EBTerm & a2, const EBTerm & a3, const EBTerm & a4, const EBTerm & a5, const EBTerm & a6, const EBTerm & a7) { return (*this)((a1,a2,a3,a4,a5,a6,a7)); }
    EBFunction & operator() (const EBTerm & a1, const EBTerm & a2, const EBTerm & a3, const EBTerm & a4, const EBTerm & a5, const EBTerm & a6, const EBTerm & a7, const EBTerm & a8) { return (*this)((a1,a2,a3,a4,a5,a6,a7,a8)); }
    EBFunction & operator() (const EBTerm & a1, const EBTerm & a2, const EBTerm & a3, const EBTerm & a4, const EBTerm & a5, const EBTerm & a6, const EBTerm & a7, const EBTerm & a8, const EBTerm & a9) { return (*this)((a1,a2,a3,a4,a5,a6,a7,a8,a9)); }

    // cast an EBFunction into an EBTerm
    operator EBTerm() const;

    // cast into a string (via the cast into a term above)
    operator std::string() const;

    // function definition (assignment)
    EBFunction & operator= (const EBTerm &);
    EBFunction & operator= (const EBFunction &);

    // get the list of arguments and check if they are all symbols
    std::string args();

    /**
     * Unary operators on functions
     */
    EBTerm operator- () { return -EBTerm(*this); }
    EBTerm operator! () { return !EBTerm(*this); }

    // perform a substitution (returns substituton count)
    unsigned int substitute(const EBSubstitutionRule & rule);
    unsigned int substitute(const EBSubstitutionRuleList & rules);

  protected:
    EBTermList arguments, eval_arguments;
    EBTerm term;
  };

  #define BINARYFUNC_OP_IMPLEMENT(op,OP) \
  friend EBTerm operator op (const EBFunction & left, const EBFunction & right) { \
    return EBTerm(new EBBinaryOpTermNode(EBTerm(left).root->clone(), EBTerm(right).root->clone(), EBBinaryOpTermNode::OP)); \
  } \
  friend EBTerm operator op (int left, const EBFunction & right) { \
    return EBTerm(new EBBinaryOpTermNode(new EBNumberNode<int>(left), EBTerm(right).root->clone(), EBBinaryOpTermNode::OP)); \
  } \
  friend EBTerm operator op (Real left, const EBFunction & right) { \
    return EBTerm(new EBBinaryOpTermNode(new EBNumberNode<Real>(left), EBTerm(right).root->clone(), EBBinaryOpTermNode::OP)); \
  }
  BINARYFUNC_OP_IMPLEMENT(+,ADD)
  BINARYFUNC_OP_IMPLEMENT(-,SUB)
  BINARYFUNC_OP_IMPLEMENT(*,MUL)
  BINARYFUNC_OP_IMPLEMENT(/,DIV)
  BINARYFUNC_OP_IMPLEMENT(%,MOD)
  BINARYFUNC_OP_IMPLEMENT(<,LESS)
  BINARYFUNC_OP_IMPLEMENT(>,GREATER)
  BINARYFUNC_OP_IMPLEMENT(<=,LESSEQ)
  BINARYFUNC_OP_IMPLEMENT(>=,GREATEREQ)
  BINARYFUNC_OP_IMPLEMENT(==,EQ)
  BINARYFUNC_OP_IMPLEMENT(!=,NOTEQ)
};

// convenience function for numeric exponent
template<typename T>
ExpressionBuilder::EBTerm pow(const ExpressionBuilder::EBTerm & left, T exponent) {
  return ExpressionBuilder::EBTerm(
    new ExpressionBuilder::EBBinaryOpTermNode(
      left.root->clone(),
      new ExpressionBuilder::EBNumberNode<T>(exponent),
      ExpressionBuilder::EBBinaryOpTermNode::POW
    )
  );
}

// convert a number node into a string
template<typename T>
std::string
ExpressionBuilder::EBNumberNode<T>::stringify() const
{
  std::ostringstream s;
  s << std::setprecision(12) << value;
  return s.str();
}

#endif //EXPRESSIONBUILDER_H
