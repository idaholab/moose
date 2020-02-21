//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <vector>
#include <ostream>
#include <sstream>
#include <iomanip>

#include "MooseError.h"
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
  ExpressionBuilder(){};

  // forward delcarations
  class EBTerm;
  class EBTermNode;
  class EBFunction;
  class EBSubstitutionRule;
  class Comparer;
  typedef std::vector<EBTerm> EBTermList;
  typedef std::vector<EBTermNode *> EBTermNodeList;
  typedef std::vector<const EBSubstitutionRule *> EBSubstitutionRuleList;

  /// Base class for nodes in the expression tree
  class EBTermNode
  {
  public:
    virtual ~EBTermNode(){};
    virtual EBTermNode * clone() const = 0;

    virtual std::string stringify() const = 0;
    virtual unsigned int substitute(const EBSubstitutionRuleList & /*rule*/) { return 0; }
    virtual int precedence() const = 0;
    friend std::ostream & operator<<(std::ostream & os, const EBTermNode & node)
    {
      return os << node.stringify();
    }
    virtual std::vector<EBTermNode *> getChildren() { return std::vector<EBTermNode *>(0); }
    virtual bool compare(EBTermNode *, std::vector<EBTermNode *>) = 0;
    virtual std::vector<EBTermNode *> getPermutations() { return std::vector<EBTermNode *>(0); }
  };

  /// Template class for leaf nodes holding numbers in the expression tree
  template <typename T>
  class EBNumberNode : public EBTermNode
  {
    T _value;

  public:
    EBNumberNode(T value) : _value(value){};
    virtual EBNumberNode<T> * clone() const { return new EBNumberNode(_value); }

    virtual std::string stringify() const;
    virtual int precedence() const { return 0; }

    T getValue() { return _value; }

    virtual bool compare(EBTermNode * compare_to, std::vector<EBTermNode *>)
    {
      EBNumberNode<T> * c_to = dynamic_cast<EBNumberNode<T> *>(compare_to);

      if(c_to != NULL && c_to->getValue() - _value < 0.000001)
        return true;
      return false;
    }
  };

  /// Template class for leaf nodes holding symbols (i.e. variables) in the expression tree
  class EBSymbolNode : public EBTermNode
  {
    std::string _symbol;

  public:
    EBSymbolNode(std::string symbol) : _symbol(symbol){};
    virtual EBSymbolNode * clone() const { return new EBSymbolNode(_symbol); }

    virtual std::string stringify() const;
    virtual int precedence() const { return 0; }

    std::string getSymbol() { return _symbol; }

    virtual bool compare(EBTermNode * compare_to, std::vector<EBTermNode *>)
    {
      EBSymbolNode * c_to = dynamic_cast<EBSymbolNode *>(compare_to);

      // We will do a hash compare first in the future
      if(c_to != NULL && c_to->getSymbol() == _symbol)
        return true;
      return false;
    }
  };

  /**
   * Template class for leaf nodes holding anonymous IDs in the expression tree.
   * No such node must be left in the final expression that is serialized and passed to FParser
   */
  class EBTempIDNode : public EBTermNode
  {
    unsigned long _id;

  public:
    EBTempIDNode(unsigned int id) : _id(id){};
    virtual EBTempIDNode * clone() const { return new EBTempIDNode(_id); }

    virtual std::string stringify() const; // returns "[idnumber]"
    virtual int precedence() const { return 0; }

    virtual bool compare(EBTermNode *, std::vector<EBTermNode *>)
    {
      mooseError("Cannot compare with temp id node");
    }
  };

  /// Base class for nodes with a single sub node (i.e. functions or operators taking one argument)
  class EBUnaryTermNode : public EBTermNode
  {
  public:
    EBUnaryTermNode(EBTermNode * subnode) : _subnode(subnode){};
    virtual ~EBUnaryTermNode() { delete _subnode; };

    virtual unsigned int substitute(const EBSubstitutionRuleList & rule);
    const EBTermNode * getSubnode() const { return _subnode; }

    virtual std::vector<EBTermNode *> getChildren() { return std::vector<EBTermNode *>(1,_subnode); }

  protected:
    EBTermNode * _subnode;
  };

  /// Node representing a function with two arguments
  class EBUnaryFuncTermNode : public EBUnaryTermNode
  {
  public:
    enum NodeType
    {
      SIN,
      COS,
      TAN,
      ABS,
      LOG,
      LOG2,
      LOG10,
      EXP,
      SINH,
      COSH
    } _type;

    EBUnaryFuncTermNode(EBTermNode * subnode, NodeType type)
      : EBUnaryTermNode(subnode), _type(type){};
    virtual EBUnaryFuncTermNode * clone() const
    {
      return new EBUnaryFuncTermNode(_subnode->clone(), _type);
    };

    virtual std::string stringify() const;
    virtual int precedence() const { return 2; }

    virtual bool compare(EBTermNode * compare_to, std::vector<EBTermNode *>)
    {
      EBUnaryFuncTermNode * c_to = dynamic_cast<EBUnaryFuncTermNode *>(compare_to);

      // We will do a hash compare first in the future
      if(c_to != NULL && c_to->_type == _type)
        return true;
      return false;
    }
  };

  /// Node representing a unary operator
  class EBUnaryOpTermNode : public EBUnaryTermNode
  {
  public:
    enum NodeType
    {
      NEG,
      LOGICNOT
    } _type;

    EBUnaryOpTermNode(EBTermNode * subnode, NodeType type)
      : EBUnaryTermNode(subnode), _type(type){};
    virtual EBUnaryOpTermNode * clone() const
    {
      return new EBUnaryOpTermNode(_subnode->clone(), _type);
    };

    virtual std::string stringify() const;
    virtual int precedence() const { return 3; }

    virtual bool compare(EBTermNode * compare_to, std::vector<EBTermNode *>)
    {
      EBUnaryOpTermNode * c_to = dynamic_cast<EBUnaryOpTermNode *>(compare_to);

      // We will do a hash compare first in the future
      if(c_to != NULL && c_to->_type == _type)
        return true;
      return false;
    }
  };

  /// Base class for nodes with two sub nodes (i.e. functions or operators taking two arguments)
  class EBBinaryTermNode : public EBTermNode
  {
  public:
    EBBinaryTermNode(EBTermNode * left, EBTermNode * right) : _left(left), _right(right){};
    virtual ~EBBinaryTermNode()
    {
      delete _left;
      delete _right;
    };

    virtual unsigned int substitute(const EBSubstitutionRuleList & rule);

    virtual std::vector<EBTermNode *> getChildren()
    {
      std::vector<EBTermNode *> children(2);
      children[0] = _right;
      children[1] = _left;
      return children;
    }

  protected:
    EBTermNode * _left;
    EBTermNode * _right;
  };

  /// Node representing a binary operator
  class EBBinaryOpTermNode : public EBBinaryTermNode
  {
  public:
    enum NodeType
    {
      ADD,
      SUB,
      MUL,
      DIV,
      MOD,
      POW,
      LESS,
      GREATER,
      LESSEQ,
      GREATEREQ,
      EQ,
      NOTEQ
    };

    EBBinaryOpTermNode(EBTermNode * left, EBTermNode * right, NodeType type)
      : EBBinaryTermNode(left, right), _type(type){};
    virtual EBBinaryOpTermNode * clone() const
    {
      return new EBBinaryOpTermNode(_left->clone(), _right->clone(), _type);
    };

    virtual std::string stringify() const;
    virtual int precedence() const;
    bool commutative() const;

    virtual bool compare(EBTermNode * compare_to, std::vector<EBTermNode *>)
    {
      EBBinaryOpTermNode * c_to = dynamic_cast<EBBinaryOpTermNode *>(compare_to);

      // We will do a hash compare first in the future
      if(c_to != NULL && c_to->_type == _type)
        return true;
      return false;
    }

    virtual std::vector<EBTermNode *> getPermutations()
    {
      if(commutative())
        return std::vector<EBTermNode *>(1,new EBBinaryOpTermNode(_right->clone(), _left->clone(), _type));
      return std::vector<EBTermNode *>(0);
    }

  protected:
    NodeType _type;
  };

  /// Node representing a function with two arguments
  class EBBinaryFuncTermNode : public EBBinaryTermNode
  {
  public:
    enum NodeType
    {
      MIN,
      MAX,
      ATAN2,
      HYPOT,
      PLOG
    } _type;

    EBBinaryFuncTermNode(EBTermNode * left, EBTermNode * right, NodeType type)
      : EBBinaryTermNode(left, right), _type(type){};
    virtual EBBinaryFuncTermNode * clone() const
    {
      return new EBBinaryFuncTermNode(_left->clone(), _right->clone(), _type);
    };

    virtual std::string stringify() const;
    virtual int precedence() const { return 2; }
    bool commutative() const;

    virtual bool compare(EBTermNode * compare_to, std::vector<EBTermNode *>)
    {
      EBBinaryFuncTermNode * c_to = dynamic_cast<EBBinaryFuncTermNode *>(compare_to);

      // We will do a hash compare first in the future
      if(c_to != NULL && c_to->_type == _type)
        return true;
      return false;
    }

    virtual std::vector<EBTermNode *> getPermutations()
    {
      if(commutative())
        return std::vector<EBTermNode *>(1,new EBBinaryFuncTermNode(_right->clone(), _left->clone(), _type));
      return std::vector<EBTermNode *>(0);
    }
  };

  /// Base class for nodes with two sub nodes (i.e. functions or operators taking two arguments)
  class EBTernaryTermNode : public EBBinaryTermNode
  {
  public:
    EBTernaryTermNode(EBTermNode * left, EBTermNode * middle, EBTermNode * right)
      : EBBinaryTermNode(left, right), _middle(middle){};
    virtual ~EBTernaryTermNode() { delete _middle; };

    virtual unsigned int substitute(const EBSubstitutionRuleList & rule);

    virtual std::vector<EBTermNode *> getChildren()
    {
      std::vector<EBTermNode *> children(3);
      children[0] = _right;
      children[1] = _middle;
      children[2] = _left;
      return children;
    }

  protected:
    EBTermNode * _middle;
  };

  /// Node representing a function with three arguments
  class EBTernaryFuncTermNode : public EBTernaryTermNode
  {
  public:
    enum NodeType
    {
      CONDITIONAL
    } _type;

    EBTernaryFuncTermNode(EBTermNode * left, EBTermNode * middle, EBTermNode * right, NodeType type)
      : EBTernaryTermNode(left, middle, right), _type(type){};
    virtual EBTernaryFuncTermNode * clone() const
    {
      return new EBTernaryFuncTermNode(_left->clone(), _middle->clone(), _right->clone(), _type);
    };

    virtual std::string stringify() const;
    virtual int precedence() const { return 2; }

    virtual bool compare(EBTermNode * compare_to, std::vector<EBTermNode *>)
    {
      EBTernaryFuncTermNode * c_to = dynamic_cast<EBTernaryFuncTermNode *>(compare_to);

      // We will do a hash compare first in the future
      if(c_to != NULL && c_to->_type == _type)
        return true;
      return false;
    }
  };

  class EBStarNode : public EBTermNode
  {
  public:
    EBStarNode(unsigned int index) : _index(index) {}

    virtual EBStarNode * clone() const
    {
      return new EBStarNode(_index);
    }

    virtual std::string stringify() const
    {
      mooseError("Star Node should not be present to stringify.");
    }
    virtual int precedence() const
    {
      mooseError("Star Node should not be present to be able to find a precedence.");
    }
    virtual EBTermNode * derivative(const std::string &)
    {
      mooseError("Star Node should not be present while a derivative is being taken.");
    }
    virtual bool compare(EBTermNode * compare_to, std::vector<EBTermNode *> star_compares)
    {
      if(star_compares[_index] == NULL)
      {
        star_compares[_index] = compare_to->clone();
        return true;
      }
      Comparer * comparer = new Comparer(compare_to, star_compares[_index]);
      if(comparer->compare())
        return true;
      return false;
    }

  private:
    unsigned int _index;
  };

  /**
   * Substitution rule functor base class to perform flexible term substitutions
   */
  class EBSubstitutionRule
  {
  public:
    virtual EBTermNode * apply(const EBTermNode *) const = 0;
    virtual ~EBSubstitutionRule() {}
  };

  /**
   * Substitution rule base class that applies to nodes of type Node_T
   */
  template <class Node_T>
  class EBSubstitutionRuleTyped : public EBSubstitutionRule
  {
  public:
    virtual EBTermNode * apply(const EBTermNode *) const;

  protected:
    // on successful substitution this returns a new node to replace the old one, otherwise it
    // returns NULL
    virtual EBTermNode * substitute(const Node_T &) const = 0;
  };

  /**
   * Generic Substitution rule to replace all occurences of a given symbol node
   * term with a user defined term. This is used by EBFunction.
   */
  class EBTermSubstitution : public EBSubstitutionRuleTyped<EBSymbolNode>
  {
  public:
    EBTermSubstitution(const EBTerm & find, const EBTerm & replace);
    virtual ~EBTermSubstitution() { delete _replace; }
  protected:
    virtual EBTermNode * substitute(const EBSymbolNode &) const;
    std::string _find;
    EBTermNode * _replace;
  };

  /**
   * Substitution rule to replace all occurences of log(x) with plog(x, epsilon)
   * with a user defined term for epsilon.
   */
  class EBLogPlogSubstitution : public EBSubstitutionRuleTyped<EBUnaryFuncTermNode>
  {
  public:
    EBLogPlogSubstitution(const EBTerm & epsilon) : _epsilon(epsilon.cloneRoot())
    {
      mooseAssert(_epsilon != NULL, "Epsilon must not be an empty term in EBLogPlogSubstitution");
    }
    virtual ~EBLogPlogSubstitution() { delete _epsilon; }
  protected:
    virtual EBTermNode * substitute(const EBUnaryFuncTermNode &) const;
    EBTermNode * _epsilon;
  };

  /**
   * User facing host object for an expression tree. Each EBTerm contains a _root
   * node pointer to an EBTermNode object. The _root pointer should never be NULL,
   * but it should be safe if it ever is. The default constructor assigns a
   * EBTempIDNode to _root with a unique ID.
   */
  class EBTerm
  {
  public:
    // the default constructor assigns a temporary id node to root we use the address of the
    // current EBTerm object as the ID. This could be problematic if we create and destroy terms,
    // but then we should not expect the substitution to do sane things anyways.
    EBTerm() : _root(new EBTempIDNode(reinterpret_cast<unsigned long>(this))){};

    EBTerm(const EBTerm & term) : _root(term.cloneRoot()){};
    ~EBTerm() { delete _root; };

  private:
    // construct a term from a node
    EBTerm(EBTermNode * root) : _root(root){};

  public:
    // construct from number or string
    EBTerm(int number) : _root(new EBNumberNode<int>(number)) {}
    EBTerm(Real number) : _root(new EBNumberNode<Real>(number)) {}
    EBTerm(const char * symbol) : _root(new EBSymbolNode(symbol)) {}

    // concatenate terms to form a parameter list with (()) syntax (those need to be out-of-class!)
    friend EBTermList operator,(const ExpressionBuilder::EBTerm & larg,
                                const ExpressionBuilder::EBTerm & rarg);
    friend EBTermList operator,(const ExpressionBuilder::EBTerm & larg,
                                const ExpressionBuilder::EBTermList & rargs);
    friend EBTermList operator,(const ExpressionBuilder::EBTermList & largs,
                                const ExpressionBuilder::EBTerm & rarg);

    // dump term as FParser expression
    friend std::ostream & operator<<(std::ostream & os, const EBTerm & term);
    // cast into a string
    operator std::string() const { return _root->stringify(); }

    // assign a term
    EBTerm & operator=(const EBTerm & term)
    {
      delete _root;
      _root = term.cloneRoot();
      return *this;
    }

    // perform a substitution (returns substituton count)
    unsigned int substitute(const EBSubstitutionRule & rule);
    unsigned int substitute(const EBSubstitutionRuleList & rules);

    const EBTermNode * getRoot() const { return _root; }
    EBTermNode * cloneRoot() const { return _root == NULL ? NULL : _root->clone(); }

    bool compare(const EBTerm & rhs)
    {
        Comparer comparer(this->cloneRoot(), rhs.cloneRoot());
        return comparer.compare();
    }

  protected:
    EBTermNode * _root;

  public:
/**
 * Unary operators
 */
#define UNARY_OP_IMPLEMENT(op, OP)                                                                 \
  EBTerm operator op() const                                                                       \
  {                                                                                                \
    mooseAssert(_root != NULL, "Empty term provided for unary operator " #op);                     \
    return EBTerm(new EBUnaryOpTermNode(cloneRoot(), EBUnaryOpTermNode::OP));                      \
  }
    UNARY_OP_IMPLEMENT(-, NEG)
    UNARY_OP_IMPLEMENT(!, LOGICNOT)

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

/*
 * Binary operators (including number,term operations)
 */
#define BINARY_OP_IMPLEMENT(op, OP)                                                                \
  EBTerm operator op(const EBTerm & term) const                                                    \
  {                                                                                                \
    mooseAssert(_root != NULL, "Empty term provided on left side of operator " #op);               \
    mooseAssert(term._root != NULL, "Empty term provided on right side of operator " #op);         \
    return EBTerm(new EBBinaryOpTermNode(cloneRoot(), term.cloneRoot(), EBBinaryOpTermNode::OP));  \
  }                                                                                                \
  friend EBTerm operator op(int left, const EBTerm & right)                                        \
  {                                                                                                \
    mooseAssert(right._root != NULL, "Empty term provided on right side of operator " #op);        \
    return EBTerm(new EBBinaryOpTermNode(                                                          \
        new EBNumberNode<int>(left), right.cloneRoot(), EBBinaryOpTermNode::OP));                  \
  }                                                                                                \
  friend EBTerm operator op(Real left, const EBTerm & right)                                       \
  {                                                                                                \
    mooseAssert(right._root != NULL, "Empty term provided on right side of operator " #op);        \
    return EBTerm(new EBBinaryOpTermNode(                                                          \
        new EBNumberNode<Real>(left), right.cloneRoot(), EBBinaryOpTermNode::OP));                 \
  }                                                                                                \
  friend EBTerm operator op(const EBFunction & left, const EBTerm & right)                         \
  {                                                                                                \
    mooseAssert(EBTerm(left)._root != NULL, "Empty term provided on left side of operator " #op);  \
    mooseAssert(right._root != NULL, "Empty term provided on right side of operator " #op);        \
    return EBTerm(new EBBinaryOpTermNode(                                                          \
        EBTerm(left).cloneRoot(), right.cloneRoot(), EBBinaryOpTermNode::OP));                     \
  }                                                                                                \
  friend EBTerm operator op(const EBFunction & left, const EBFunction & right);                    \
  friend EBTerm operator op(int left, const EBFunction & right);                                   \
  friend EBTerm operator op(Real left, const EBFunction & right);
    BINARY_OP_IMPLEMENT(+, ADD)
    BINARY_OP_IMPLEMENT(-, SUB)
    BINARY_OP_IMPLEMENT(*, MUL)
    BINARY_OP_IMPLEMENT(/, DIV)
    BINARY_OP_IMPLEMENT(%, MOD)
    BINARY_OP_IMPLEMENT(<, LESS)
    BINARY_OP_IMPLEMENT(>, GREATER)
    BINARY_OP_IMPLEMENT(<=, LESSEQ)
    BINARY_OP_IMPLEMENT(>=, GREATEREQ)
    BINARY_OP_IMPLEMENT(==, EQ)
    BINARY_OP_IMPLEMENT(!=, NOTEQ)

/*
 * Compound assignment operators
 */
#define BINARYCOMP_OP_IMPLEMENT(op, OP)                                                            \
  EBTerm & operator op(const EBTerm & term)                                                        \
  {                                                                                                \
    mooseAssert(_root != NULL, "Empty term provided on left side of operator " #op);               \
    mooseAssert(term._root != NULL, "Empty term provided on right side of operator " #op);         \
    if (dynamic_cast<EBTempIDNode *>(_root))                                                       \
      mooseError("Using compound assignment operator on anonymous term. Set it to 0 first!");      \
    _root = new EBBinaryOpTermNode(_root, term.cloneRoot(), EBBinaryOpTermNode::OP);               \
    return *this;                                                                                  \
  }
    BINARYCOMP_OP_IMPLEMENT(+=, ADD)
    BINARYCOMP_OP_IMPLEMENT(-=, SUB)
    BINARYCOMP_OP_IMPLEMENT(*=, MUL)
    BINARYCOMP_OP_IMPLEMENT(/=, DIV)
    BINARYCOMP_OP_IMPLEMENT(%=, MOD)

    /**
    * @{
     * Binary functions
     */
    friend EBTerm min(const EBTerm &, const EBTerm &);
    friend EBTerm max(const EBTerm &, const EBTerm &);
    friend EBTerm pow(const EBTerm &, const EBTerm &);
    template <typename T>
    friend EBTerm pow(const EBTerm &, T exponent);
    friend EBTerm atan2(const EBTerm &, const EBTerm &);
    friend EBTerm hypot(const EBTerm &, const EBTerm &);
    friend EBTerm plog(const EBTerm &, const EBTerm &);
    ///@}

    /**
     * Ternary functions
     */
    friend EBTerm conditional(const EBTerm &, const EBTerm &, const EBTerm &);
  };

  /// User facing host object for a function. This combines a term with an argument list.
  class EBFunction
  {
  public:
    EBFunction(){};

    /// @{
    /// set the temporary argument list which is either used for evaluation
    /// or committed to the argument list upon function definition (assignment)
    EBFunction & operator()(const EBTerm & arg);
    EBFunction & operator()(const EBTermList & args);
    /// @}

    /// @{
    /// convenience operators to allow single bracket syntax
    EBFunction & operator()(const EBTerm & a1, const EBTerm & a2) { return (*this)((a1, a2)); }
    EBFunction & operator()(const EBTerm & a1, const EBTerm & a2, const EBTerm & a3)
    {
      return (*this)((a1, a2, a3));
    }
    EBFunction &
    operator()(const EBTerm & a1, const EBTerm & a2, const EBTerm & a3, const EBTerm & a4)
    {
      return (*this)((a1, a2, a3, a4));
    }
    EBFunction & operator()(const EBTerm & a1,
                            const EBTerm & a2,
                            const EBTerm & a3,
                            const EBTerm & a4,
                            const EBTerm & a5)
    {
      return (*this)((a1, a2, a3, a4, a5));
    }
    EBFunction & operator()(const EBTerm & a1,
                            const EBTerm & a2,
                            const EBTerm & a3,
                            const EBTerm & a4,
                            const EBTerm & a5,
                            const EBTerm & a6)
    {
      return (*this)((a1, a2, a3, a4, a5, a6));
    }
    EBFunction & operator()(const EBTerm & a1,
                            const EBTerm & a2,
                            const EBTerm & a3,
                            const EBTerm & a4,
                            const EBTerm & a5,
                            const EBTerm & a6,
                            const EBTerm & a7)
    {
      return (*this)((a1, a2, a3, a4, a5, a6, a7));
    }
    EBFunction & operator()(const EBTerm & a1,
                            const EBTerm & a2,
                            const EBTerm & a3,
                            const EBTerm & a4,
                            const EBTerm & a5,
                            const EBTerm & a6,
                            const EBTerm & a7,
                            const EBTerm & a8)
    {
      return (*this)((a1, a2, a3, a4, a5, a6, a7, a8));
    }
    EBFunction & operator()(const EBTerm & a1,
                            const EBTerm & a2,
                            const EBTerm & a3,
                            const EBTerm & a4,
                            const EBTerm & a5,
                            const EBTerm & a6,
                            const EBTerm & a7,
                            const EBTerm & a8,
                            const EBTerm & a9)
    {
      return (*this)((a1, a2, a3, a4, a5, a6, a7, a8, a9));
    }
    /// @}

    /// cast an EBFunction into an EBTerm
    operator EBTerm() const;

    /// cast into a string (via the cast into a term above)
    operator std::string() const;

    /// @{
    /// function definition (assignment)
    EBFunction & operator=(const EBTerm &);
    EBFunction & operator=(const EBFunction &);
    /// @}

    /// get the list of arguments and check if they are all symbols
    std::string args();

    /// @{
    /// Unary operators on functions
    EBTerm operator-() { return -EBTerm(*this); }
    EBTerm operator!() { return !EBTerm(*this); }
    /// @}

    // perform a substitution (returns substituton count)
    unsigned int substitute(const EBSubstitutionRule & rule);
    unsigned int substitute(const EBSubstitutionRuleList & rules);

  protected:
    /// argument list the function is declared with
    EBTermList _arguments;
    /// argument list passed in when evaluating the function
    EBTermList _eval_arguments;

    // underlying term that the _eval_arguments are substituted in
    EBTerm _term;
  };

/*
 * Binary operators
 */
#define BINARYFUNC_OP_IMPLEMENT(op, OP)                                                            \
  friend EBTerm operator op(const EBFunction & left, const EBFunction & right)                     \
  {                                                                                                \
    mooseAssert(EBTerm(left)._root != NULL, "Empty term provided on left side of operator " #op);  \
    mooseAssert(EBTerm(right)._root != NULL,                                                       \
                "Empty term provided on right side of operator " #op);                             \
    return EBTerm(new EBBinaryOpTermNode(                                                          \
        EBTerm(left).cloneRoot(), EBTerm(right).cloneRoot(), EBBinaryOpTermNode::OP));             \
  }                                                                                                \
  friend EBTerm operator op(int left, const EBFunction & right)                                    \
  {                                                                                                \
    mooseAssert(EBTerm(right)._root != NULL,                                                       \
                "Empty term provided on right side of operator " #op);                             \
    return EBTerm(new EBBinaryOpTermNode(                                                          \
        new EBNumberNode<int>(left), EBTerm(right).cloneRoot(), EBBinaryOpTermNode::OP));          \
  }                                                                                                \
  friend EBTerm operator op(Real left, const EBFunction & right)                                   \
  {                                                                                                \
    mooseAssert(EBTerm(right)._root != NULL,                                                       \
                "Empty term provided on right side of operator " #op);                             \
    return EBTerm(new EBBinaryOpTermNode(                                                          \
        new EBNumberNode<Real>(left), EBTerm(right).cloneRoot(), EBBinaryOpTermNode::OP));         \
  }
  BINARYFUNC_OP_IMPLEMENT(+, ADD)
  BINARYFUNC_OP_IMPLEMENT(-, SUB)
  BINARYFUNC_OP_IMPLEMENT(*, MUL)
  BINARYFUNC_OP_IMPLEMENT(/, DIV)
  BINARYFUNC_OP_IMPLEMENT(%, MOD)
  BINARYFUNC_OP_IMPLEMENT(<, LESS)
  BINARYFUNC_OP_IMPLEMENT(>, GREATER)
  BINARYFUNC_OP_IMPLEMENT(<=, LESSEQ)
  BINARYFUNC_OP_IMPLEMENT(>=, GREATEREQ)
  BINARYFUNC_OP_IMPLEMENT(==, EQ)
  BINARYFUNC_OP_IMPLEMENT(!=, NOTEQ)

  class Comparer
  {
  public:
    Comparer(EBTermNode * lhs, EBTermNode * rhs) : _compare_left(lhs), _compare_right(rhs), _star_compares(32, NULL) {}
    Comparer(EBTermNode * lhs, std::stack<EBTermNode *> left_stack, EBTermNode * rhs, std::stack<EBTermNode *> right_stack, std::vector<EBTermNode *> star_compares, std::stack<Comparer *> permutations) : _compare_left(lhs), _compare_right(rhs), _left_stack_compare(left_stack), _right_stack_compare(right_stack), _star_compares(star_compares), _permutations(permutations) {}

    EBTermNode * getCompareLeft() { return _compare_left; }
    EBTermNode * getCompareRight() { return _compare_right; }
    std::stack<EBTermNode *> getLeftStack() { return _left_stack_compare; }
    std::stack<EBTermNode *> getRightStack() { return _right_stack_compare; }
    std::vector<EBTermNode *> getStarCompares() { return _star_compares; }
    std::stack<Comparer *> getPermutations() { return _permutations; }

    bool compare();

  protected:
    EBTermNode * _compare_left;
    EBTermNode * _compare_right;
    std::stack<EBTermNode *> _left_stack_compare;
    std::stack<EBTermNode *> _right_stack_compare;

    std::vector<EBTermNode *> _star_compares;

    std::stack<Comparer *> _permutations;
  };
};

// convenience function for numeric exponent
template <typename T>
ExpressionBuilder::EBTerm
pow(const ExpressionBuilder::EBTerm & left, T exponent)
{
  return ExpressionBuilder::EBTerm(
      new ExpressionBuilder::EBBinaryOpTermNode(left.cloneRoot(),
                                                new ExpressionBuilder::EBNumberNode<T>(exponent),
                                                ExpressionBuilder::EBBinaryOpTermNode::POW));
}

// convert a number node into a string
template <typename T>
std::string
ExpressionBuilder::EBNumberNode<T>::stringify() const
{
  std::ostringstream s;
  s << std::setprecision(12) << _value;
  return s.str();
}

template <class Node_T>
ExpressionBuilder::EBTermNode *
ExpressionBuilder::EBSubstitutionRuleTyped<Node_T>::apply(
    const ExpressionBuilder::EBTermNode * node) const
{
  const Node_T * match_node = dynamic_cast<const Node_T *>(node);
  if (match_node == NULL)
    return NULL;
  else
    return substitute(*match_node);
}
