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
#include "MooseUtils.h"
#include "TensorTempl.h"
#include "libmesh/libmesh_common.h"

using namespace libMesh;

class ExpressionBuilderToo
{
public:
  ExpressionBuilderToo(){};

  // forward delcarations
  class EBTerm;
  class EBTermNode;
  struct Comparer;
  typedef std::vector<EBTerm> EBTermList;
  typedef std::vector<EBTermNode *> EBTermNodeList;

  /// Base class for nodes in the expression tree
  class EBTermNode
  {
  public:
    virtual ~EBTermNode(){};
    virtual EBTermNode * clone() const = 0;

    virtual std::string stringify() const = 0;
    virtual int precedence() const = 0;
    friend std::ostream & operator<<(std::ostream & os, const EBTermNode & node)
    {
      return os << node.stringify();
    }
    virtual std::vector<EBTermNode *> getChildren() { return std::vector<EBTermNode *>(0); }
    virtual bool compare(EBTermNode *, std::vector<EBTermNode *> &, bool, int &) = 0;

    virtual bool
    compareRule(EBTermNode * rule,
                std::vector<EBTermNode *> & stars,
                std::vector<unsigned int> & current_index,
                std::vector<unsigned int> changed_indeces = std::vector<unsigned int>(0),
                bool checkNNaryLeaf = false);

    virtual EBTermNode * simplify(std::pair<EBTermNode *, EBTermNode *> rule, bool & changed);

    virtual EBTermNode * replaceRule(std::vector<EBTermNode *>) { return this; }

    virtual bool isCommutative() const { return false; }

    std::vector<std::vector<EBTermNode *>>
    permuteChildren(std::vector<EBTermNode *> children, std::vector<unsigned int> changed_indeces);
    std::vector<std::vector<EBTermNode *>>
    permuteChildren(std::vector<EBTermNode *> children,
                    std::vector<std::vector<unsigned int>> & nnary_index,
                    int max_depth,
                    std::vector<unsigned int> changed_indeces,
                    int depth = 0,
                    std::vector<unsigned int> current_index = std::vector<unsigned int>(0));

    virtual EBTermNode * toNNary() { return this; }

    virtual EBTermNode * substitute(std::vector<std::pair<EBTermNode *, EBTermNode *>> subs);

    virtual bool isConstant() = 0;
    virtual EBTermNode * constantFolding() { return this; };

    virtual Real getRealValue() { mooseError("This has no real value right now"); };

    virtual EBTermNode * derivative(const std::string & derivative_comp) = 0;
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

    T * getValuePointer()
    {
      T * t_point = new T(_value);
      return t_point;
    }

    virtual bool compare(EBTermNode * compare_to, std::vector<EBTermNode *> &, bool, int &)
    {
      EBNumberNode<int> * c_to_int = dynamic_cast<EBNumberNode<int> *>(compare_to);
      EBNumberNode<Real> * c_to_real = dynamic_cast<EBNumberNode<Real> *>(compare_to);
      if (c_to_int != NULL && abs(c_to_int->getValue() - _value) < 0.000001)
        return true;
      if (c_to_real != NULL && abs(c_to_real->getValue() - _value) < 0.000001)
        return true;
      return false;
    }

    virtual bool isConstant() { return true; }
    virtual EBTermNode * constantFolding() { return this; }

    virtual Real getRealValue() { return _value; }

    virtual EBTermNode * derivative(const std::string &)
    {
      EBTermNode * deriv = new EBNumberNode<Real>(0);
      return deriv;
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

    virtual bool compare(EBTermNode * compare_to, std::vector<EBTermNode *> &, bool, int &)
    {
      EBSymbolNode * c_to = dynamic_cast<EBSymbolNode *>(compare_to);

      // We will do a hash compare first in the future
      if (c_to != NULL && c_to->getSymbol() == _symbol)
        return true;
      return false;
    }

    virtual bool isConstant() { return false; }

    virtual EBTermNode * derivative(const std::string & derivative_comp)
    {
      EBTermNode * deriv_node = NULL;
      std::hash<std::string> string_hash;
      if (string_hash(_symbol) == string_hash(derivative_comp))
      {
        if (_symbol == derivative_comp)
          deriv_node = new EBNumberNode<Real>(1);
      }
      else if (derivative_comp == "x")
      {
        deriv_node = new EBSymbolNode(_symbol + "_dx");
      }
      else if (derivative_comp == "y")
      {
        deriv_node = new EBSymbolNode(_symbol + "_dy");
      }
      else if (derivative_comp == "z")
      {
        deriv_node = new EBSymbolNode(_symbol + "_dz");
      }
      else
        deriv_node = new EBNumberNode<Real>(0);
      return deriv_node;
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

    virtual bool compare(EBTermNode *, std::vector<EBTermNode *> &, bool, int &)
    {
      mooseError("Cannot compare with temp id node");
    }

    virtual bool isConstant() { return false; }

    virtual EBTermNode * derivative(const std::string &)
    {
      mooseError("Derivative taken with temp id node still present.");
    }
  };

  /// Base class for nodes with a single sub node (i.e. functions or operators taking one argument)
  class EBUnaryTermNode : public EBTermNode
  {
  public:
    EBUnaryTermNode(EBTermNode * subnode) : _subnode(subnode){};
    virtual ~EBUnaryTermNode() { delete _subnode; };

    const EBTermNode * getSubnode() const { return _subnode; }

    virtual std::vector<EBTermNode *> getChildren()
    {
      return std::vector<EBTermNode *>(1, _subnode);
    }

    virtual EBTermNode * simplify(std::pair<EBTermNode *, EBTermNode *> rule, bool & changed)
    {
      _subnode = _subnode->simplify(rule, changed);
      return EBTermNode::simplify(rule, changed);
    }

    virtual EBTermNode * replaceRule(std::vector<EBTermNode *> stars)
    {
      _subnode = _subnode->replaceRule(stars);
      if (_subnode == NULL)
        return NULL;
      return this;
    }

    virtual EBTermNode * toNNary()
    {
      _subnode = _subnode->toNNary();
      return this;
    }

    virtual EBTermNode * substitute(std::vector<std::pair<EBTermNode *, EBTermNode *>> subs);

    virtual bool isConstant()
    {
      if (_subnode->isConstant())
        return true;
      return false;
    }

    virtual EBTermNode * constantFolding()
    {
      if (_subnode->isConstant())
        return new EBNumberNode<Real>(getRealValue());
      _subnode = _subnode->constantFolding();
      return this;
    };

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

    virtual bool compare(EBTermNode * compare_to, std::vector<EBTermNode *> &, bool, int &)
    {
      EBUnaryFuncTermNode * c_to = dynamic_cast<EBUnaryFuncTermNode *>(compare_to);

      if (c_to != NULL && c_to->_type == _type)
        return true;
      return false;
    }
    virtual Real getRealValue();

    virtual EBTermNode * derivative(const std::string & derivative_comp);
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

    virtual bool compare(EBTermNode * compare_to, std::vector<EBTermNode *> &, bool, int &)
    {
      EBUnaryOpTermNode * c_to = dynamic_cast<EBUnaryOpTermNode *>(compare_to);

      if (c_to != NULL && c_to->_type == _type)
        return true;
      return false;
    }

    virtual Real getRealValue();

    virtual EBTermNode * derivative(const std::string & derivative_comp);
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

    virtual std::vector<EBTermNode *> getChildren()
    {
      std::vector<EBTermNode *> children(2);
      children[0] = _right;
      children[1] = _left;
      return children;
    }

    virtual EBTermNode * simplify(std::pair<EBTermNode *, EBTermNode *> rule, bool & changed)
    {
      _left = _left->simplify(rule, changed);
      _right = _right->simplify(rule, changed);
      return EBTermNode::simplify(rule, changed);
    }

    virtual EBTermNode * replaceRule(std::vector<EBTermNode *> stars)
    {
      _left = _left->replaceRule(stars);
      _right = _right->replaceRule(stars);
      if (_left == NULL || _right == NULL)
        return NULL;
      return this;
    }

    virtual EBTermNode * toNNary()
    {
      _right = _right->toNNary();
      _left = _left->toNNary();
      return this;
    }

    EBTermNode * getLeft() { return _left; }
    EBTermNode * getRight() { return _right; }

    virtual EBTermNode * substitute(std::vector<std::pair<EBTermNode *, EBTermNode *>> subs);

    virtual bool isConstant()
    {
      if (_left->isConstant() && _right->isConstant())
        return true;
      return false;
    }

    EBTermNode * constantFolding()
    {
      if (_left->isConstant() && _right->isConstant())
        return new EBNumberNode<Real>(getRealValue());
      _left = _left->constantFolding();
      _right = _right->constantFolding();
      return this;
    }

  protected:
    EBTermNode * _left;
    EBTermNode * _right;
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
    virtual bool isCommutative() const;

    virtual bool compare(EBTermNode * compare_to, std::vector<EBTermNode *> &, bool, int &)
    {
      EBBinaryFuncTermNode * c_to_bin = dynamic_cast<EBBinaryFuncTermNode *>(compare_to);

      if (c_to_bin != NULL && c_to_bin->_type == _type)
        return true;
      return false;
    }

    virtual Real getRealValue();

    virtual EBTermNode * derivative(const std::string &);
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
    } _type;

    EBBinaryOpTermNode(EBTermNode * left, EBTermNode * right, NodeType type)
      : EBBinaryTermNode(left, right), _type(type){};
    virtual EBBinaryOpTermNode * clone() const
    {
      return new EBBinaryOpTermNode(_left->clone(), _right->clone(), _type);
    };

    virtual std::string stringify() const;
    virtual int precedence() const;
    virtual bool isCommutative() const;

    virtual bool compare(EBTermNode * compare_to, std::vector<EBTermNode *> &, bool isHead, int &);

    virtual EBTermNode * toNNary();

    virtual Real getRealValue();

    virtual EBTermNode * derivative(const std::string & derivative_comp);
  };

  /// Base class for nodes with two sub nodes (i.e. functions or operators taking two arguments)
  class EBTernaryTermNode : public EBBinaryTermNode
  {
  public:
    EBTernaryTermNode(EBTermNode * left, EBTermNode * middle, EBTermNode * right)
      : EBBinaryTermNode(left, right), _middle(middle){};
    virtual ~EBTernaryTermNode() { delete _middle; };

    virtual std::vector<EBTermNode *> getChildren()
    {
      std::vector<EBTermNode *> children(3);
      children[0] = _right;
      children[1] = _middle;
      children[2] = _left;
      return children;
    }
    virtual EBTermNode * simplify(std::pair<EBTermNode *, EBTermNode *> rule, bool & changed)
    {
      _middle = _middle->simplify(rule, changed);
      return EBBinaryTermNode::simplify(rule, changed);
    }

    virtual EBTermNode * replaceRule(std::vector<EBTermNode *> stars)
    {
      _middle = _middle->replaceRule(stars);
      _left = _left->replaceRule(stars);
      _right = _right->replaceRule(stars);
      if (_middle == NULL || _right == NULL || _left == NULL)
        return NULL;
      return this;
    }

    virtual EBTermNode * toNNary()
    {
      _right = _right->toNNary();
      _middle = _middle->toNNary();
      _left = _left->toNNary();
      return this;
    }

    virtual EBTermNode * substitute(std::vector<std::pair<EBTermNode *, EBTermNode *>> subs);

    virtual bool isConstant()
    {
      if (_left->isConstant() && _middle->isConstant() && _right->isConstant())
        return true;
      return false;
    }

    virtual EBTermNode * constantFolding()
    {
      if (_left->isConstant() && _middle->isConstant() && _right->isConstant())
        return new EBNumberNode<Real>(getRealValue());
      _left = _left->constantFolding();
      _middle = _middle->constantFolding();
      _right = _right->constantFolding();
      return this;
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

    virtual bool compare(EBTermNode * compare_to, std::vector<EBTermNode *> &, bool, int &)
    {
      EBTernaryFuncTermNode * c_to = dynamic_cast<EBTernaryFuncTermNode *>(compare_to);

      if (c_to != NULL && c_to->_type == _type)
        return true;
      return false;
    }

    virtual Real getRealValue();

    virtual EBTermNode * derivative(const std::string & derivative_comp);
  };

  class EBNNaryTermNode : public EBTermNode
  {
  public:
    EBNNaryTermNode(std::vector<EBTermNode *> children) : _children(children){};

    virtual EBTermNode * replaceRule(std::vector<EBTermNode *> stars)
    {
      for (unsigned int i = 0; i < _children.size(); ++i)
      {
        _children[i] = _children[i]->replaceRule(stars);
        if (_children[i] == NULL)
          return NULL;
      }
      return this;
    }

    std::vector<EBTermNode *> getChildren() { return _children; }
    unsigned int childrenSize() { return _children.size(); }

    virtual bool isConstant()
    {
      for (unsigned int i = 0; i < _children.size(); ++i)
        if (!_children[i]->isConstant())
          return false;
      return true;
    }

    bool hasRest()
    {
      EBRestNode * checkRest;
      for (unsigned int i = 0; i < _children.size(); ++i)
      {
        checkRest = dynamic_cast<EBRestNode *>(_children[i]);
        if (checkRest != NULL)
          return true;
      }
      return false;
    }

  protected:
    std::vector<EBTermNode *> _children;
  };

  class EBNNaryOpTermNode : public EBNNaryTermNode
  {
  public:
    enum NodeType
    {
      MUL,
      ADD
    } _type;

    EBNNaryOpTermNode(std::vector<EBTermNode *> children, NodeType type)
      : EBNNaryTermNode(children), _type(type)
    {
    }
    virtual EBNNaryOpTermNode * clone() const
    {
      std::vector<EBTermNode *> children(_children.size());
      for (unsigned int i = 0; i < children.size(); ++i)
        children[i] = _children[i]->clone();
      return new EBNNaryOpTermNode(children, _type);
    }

    virtual std::string stringify() const;
    virtual int precedence() const;
    virtual bool isCommutative() const { return true; }

    virtual bool
    compare(EBTermNode * compare_to, std::vector<EBTermNode *> &, bool isHead, int & depth);

    virtual EBTermNode * simplify(std::pair<EBTermNode *, EBTermNode *> rule, bool & changed);
    virtual EBTermNode * substitute(std::vector<std::pair<EBTermNode *, EBTermNode *>> subs);
    virtual EBTermNode * toNNary();

    virtual EBTermNode * constantFolding();
    virtual Real getRealValue();

    virtual EBTermNode * derivative(const std::string & derivative_comp);

    virtual EBTermNode * replaceRule(std::vector<EBTermNode *> stars)
    {
      for (unsigned int i = 0; i < _children.size(); ++i)
      {
        _children[i] = _children[i]->replaceRule(stars);
        if (_children[i] == NULL)
          return NULL;
      }
      return this;
    }
  };

  class EBStarNode : public EBTermNode
  {
  public:
    EBStarNode(unsigned int index) : _index(index) {}

    virtual EBStarNode * clone() const { return new EBStarNode(_index); }

    virtual std::string stringify() const { return std::to_string(_index); }
    virtual int precedence() const { return 0; }
    virtual EBTermNode * derivative(const std::string &)
    {
      mooseError("Star Node should not be present while a derivative is being taken.");
    }
    virtual bool compare(EBTermNode * compare_to, std::vector<EBTermNode *> & stars, bool, int &)
    {
      if (stars[_index] == NULL)
      {
        stars[_index] = compare_to->clone();
        return true;
      }
      std::vector<EBTermNode *> empty_vec(0);
      std::vector<unsigned int> empty_int_vec(0);
      if (compare_to->compareRule(stars[_index], empty_vec, empty_int_vec, empty_int_vec, true))
        return true;
      return false;
    }

    virtual EBTermNode * replaceRule(std::vector<EBTermNode *> stars)
    {
      EBStarNode * checkStar = dynamic_cast<EBStarNode *>(stars[_index]);
      if (checkStar == NULL)
        return stars[_index]->clone();
      return NULL;
    }

    virtual bool isConstant() { return false; }

    unsigned int getIndex() { return _index; }

  private:
    unsigned int _index;
  };

  class EBRestNode : public EBTermNode
  {
  public:
    EBRestNode() {}

    virtual EBRestNode * clone() const { return new EBRestNode(); }

    virtual std::string stringify() const { return std::to_string(26); }
    virtual int precedence() const { return 0; }
    virtual EBTermNode * derivative(const std::string &)
    {
      mooseError("Star Node should not be present while a derivative is being taken.");
    }
    virtual bool compare(EBTermNode * compare_to, std::vector<EBTermNode *> & stars, bool, int &)
    {
      if (stars[26] == NULL)
      {
        stars[26] = compare_to->clone();
        return true;
      }
      std::vector<EBTermNode *> empty_vec(0);
      std::vector<unsigned int> empty_int_vec(0);
      if (compare_to->compareRule(stars[26], empty_vec, empty_int_vec, empty_int_vec, true))
        return true;
      return false;
    }

    virtual EBTermNode * replaceRule(std::vector<EBTermNode *> stars)
    {
      EBRestNode * checkRest = dynamic_cast<EBRestNode *>(stars[26]);
      if (checkRest == NULL)
        return stars[26]->clone();
      return NULL;
    }

    virtual bool isConstant() { return false; }
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
    EBTerm() : _root(new EBTempIDNode(reinterpret_cast<unsigned long>(this))) {}

    EBTerm(const EBTerm & term) : _root(term.cloneRoot()) {}
    ~EBTerm() { delete _root; };

  private:
    // construct a term from a node
    EBTerm(EBTermNode * root) : _root(root) {}
    EBTerm(EBTermNode * root, EBTermList term_args) : _root(root), _arguments(term_args) {}

  public:
    // construct from number or string
    EBTerm(int number) : _root(new EBNumberNode<int>(number)) {}
    EBTerm(Real number) : _root(new EBNumberNode<Real>(number)) {}
    EBTerm(const char * symbol) : _root(new EBSymbolNode(symbol)) {}

    // These two should only be used by EBSimpleTrees!
    // They are for making EBStarNodes & EBRestNodes
    // The dummy variable is just so people check before using them.
    EBTerm(unsigned int index, bool) : _root(new EBStarNode(index)) {}
    EBTerm(EBNNaryOpTermNode root, bool) : _root(root.clone()) {}
    EBTerm(bool) : _root(new EBRestNode()) {}

    // concatenate terms to form a parameter list with (()) syntax (those need to be out-of-class!)
    friend EBTermList operator,(const ExpressionBuilderToo::EBTerm & larg,
                                const ExpressionBuilderToo::EBTerm & rarg);
    friend EBTermList operator,(const ExpressionBuilderToo::EBTerm & larg,
                                const ExpressionBuilderToo::EBTermList & rargs);
    friend EBTermList operator,(const ExpressionBuilderToo::EBTermList & largs,
                                const ExpressionBuilderToo::EBTerm & rarg);

    // dump term as FParser expression
    friend std::ostream & operator<<(std::ostream & os, const EBTerm & term);
    // cast into a string
    operator std::string() const
    {
      if (_eval_arguments.size() == 0)
        return _root->stringify();
      else
      {
        if (_eval_arguments.size() != _arguments.size())
          mooseError("Incorrect number of arguments to substitute");
        std::vector<std::pair<EBTermNode *, EBTermNode *>> subs;
        for (unsigned int i = 0; i < _arguments.size(); ++i)
        {
          std::pair<EBTermNode *, EBTermNode *> one_sub;
          one_sub.first = _arguments[i].cloneRoot();
          one_sub.second = _eval_arguments[i].cloneRoot();
          subs.emplace_back(one_sub);
        }
        _eval_arguments.clear();
        return _root->clone()->substitute(subs)->stringify();
      }
    }

    // assign a term
    EBTerm & operator=(const EBTerm & term)
    {
      delete _root;
      _root = term.cloneRoot();
      _arguments = _eval_arguments;
      _eval_arguments.clear();
      return *this;
    }

    template <typename... T>
    EBTerm & operator()(T... terms)
    {
      return (*this)({terms...});
    };
    EBTerm & operator()(EBTermList term_list);

    const EBTermNode * getRoot() const { return _root; }
    EBTermNode * cloneRoot() const
    {
      if (_root == NULL)
        return NULL;
      else
      {
        if (_eval_arguments.size() == 0)
          return _root->clone();
        else
        {
          if (_eval_arguments.size() != _arguments.size())
            mooseError("Incorrect number of arguments to substitute");
          std::vector<std::pair<EBTermNode *, EBTermNode *>> subs;
          for (unsigned int i = 0; i < _arguments.size(); ++i)
          {
            std::pair<EBTermNode *, EBTermNode *> one_sub;
            one_sub.first = _arguments[i].cloneRoot();
            one_sub.second = _eval_arguments[i].cloneRoot();
            subs.emplace_back(one_sub);
          }
          _eval_arguments.clear();
          return _root->clone()->substitute(subs);
        }
      }
    }

    void simplify();
    void substitute(std::vector<std::vector<EBTerm>> subs);
    void derivative(std::string & derivative_comp) { _root = _root->derivative(derivative_comp); }
    EBTerm & getDerivative(std::string derivative_comp)
    {
      return *(new EBTerm(cloneRoot()->derivative(derivative_comp)));
    }

    std::string args();

  protected:
    EBTermNode * _root;
    EBTermList _arguments;
    mutable EBTermList _eval_arguments;

    static std::vector<std::pair<EBTermNode *, EBTermNode *>> _prep_simplification_rules;
    static std::vector<std::pair<EBTermNode *, EBTermNode *>> _simplification_rules;

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
  }
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

  struct Comparer
  {
    Comparer(std::stack<EBTermNode *> self_stack,
             std::stack<EBTermNode *> rule_stack,
             std::vector<EBTermNode *> stars,
             std::vector<unsigned int> current_index)
      : _self_stack(self_stack),
        _rule_stack(rule_stack),
        _stars(stars),
        _current_index(current_index)
    {
    }

    std::stack<EBTermNode *> _self_stack;
    std::stack<EBTermNode *> _rule_stack;
    std::vector<EBTermNode *> _stars;
    std::vector<unsigned int> _current_index;
  };

  class EBTensor : public TensorTempl<EBTerm>
  {
  public:
    /// zero scalar
    EBTensor() : TensorTempl<EBTerm>() {}

    /// scalar with value t
    EBTensor(EBTerm t) : TensorTempl<EBTerm>(t) {}

    /// up to order four tensors, convenience constructors
    EBTensor(NestedInitializerList<EBTerm, 1> t) : TensorTempl<EBTerm>(t) {}
    EBTensor(NestedInitializerList<EBTerm, 2> t) : TensorTempl<EBTerm>(t) {}
    EBTensor(NestedInitializerList<EBTerm, 3> t) : TensorTempl<EBTerm>(t) {}
    EBTensor(NestedInitializerList<EBTerm, 4> t) : TensorTempl<EBTerm>(t) {}

    void simplify()
    {
      for (unsigned int i = 0; i < _data.size(); ++i)
        _data[i].simplify();
    }
    void substitute(std::vector<std::vector<EBTerm>> subs)
    {
      for (unsigned int i = 0; i < _data.size(); ++i)
        _data[i].substitute(subs);
    }
    void derivative(std::string & derivative_comp)
    {
      for (unsigned int i = 0; i < _data.size(); ++i)
        _data[i].derivative(derivative_comp);
    }
  };

  EBTensor & grad(EBTerm term)
  {
    return *(
        new EBTensor({term.getDerivative("x"), term.getDerivative("y"), term.getDerivative("z")}));
  }

  EBTerm & divergence(EBTensor tens)
  {
    return *(new EBTerm(tens(0).getDerivative("x") + tens(1).getDerivative("y") +
                        tens(2).getDerivative("z")));
  }
};

// convenience function for numeric exponent
template <typename T>
ExpressionBuilderToo::EBTerm
pow(const ExpressionBuilderToo::EBTerm & left, T exponent)
{
  return ExpressionBuilderToo::EBTerm(new ExpressionBuilderToo::EBBinaryOpTermNode(
      left.cloneRoot(),
      new ExpressionBuilderToo::EBNumberNode<T>(exponent),
      ExpressionBuilderToo::EBBinaryOpTermNode::POW));
}

// convert a number node into a string
template <typename T>
std::string
ExpressionBuilderToo::EBNumberNode<T>::stringify() const
{
  std::ostringstream s;
  s << std::setprecision(12) << _value;
  return s.str();
}
