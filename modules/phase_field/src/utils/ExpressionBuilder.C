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
    _coup_var_vecs[*it] = EBTermList(0);
    _grad_coup_var_vecs[*it] = EBTermList(0);
    _second_coup_var_vecs[*it] = EBTermList(0);
    std::string base_name = *it;
    if (vec_coupled.find(*it) != vec_coupled.end())
    {
      std::pair<std::string, std::string> variable = vec_coupled[*it];
      // std::string base_name = pars.get<std::string>(variable.first);

      for (unsigned int j = 0; j < pars.get<unsigned int>(variable.second); ++j)
      {
        std::string varname = base_name + std::to_string(j);
        EBTerm term(varname.c_str(), makeGradEBTerm(base_name, varname));
        _coup_var_vecs[base_name].push_back(term);
        _coup_vars[varname] = term;
      }
    }
    else
    {
      EBTerm term(base_name.c_str(), makeGradEBTerm(base_name, base_name));
      _coup_var_vecs[base_name].push_back(term);
      _coup_vars[base_name] = term;
    }
  }
}

template <>
std::vector<std::string>
ExpressionBuilder::getVarComps<Real>(const std::string & var_name)
{
  return std::vector<std::string>({var_name});
}

template <>
std::vector<std::string>
ExpressionBuilder::getVarComps<RealVectorValue>(const std::string & var_name)
{
  return std::vector<std::string>({var_name + "_x", var_name + "_y", var_name + "_z"});
}

template <>
std::vector<std::string>
ExpressionBuilder::getVarComps<RankTwoTensor>(const std::string & var_name)
{
  return std::vector<std::string>({var_name + "_xx",
                                   var_name + "_xy",
                                   var_name + "_xz",
                                   var_name + "_xy",
                                   var_name + "_yy",
                                   var_name + "_yz",
                                   var_name + "_xz",
                                   var_name + "_yz",
                                   var_name + "_zz"});
}

ExpressionBuilder::EBTermNode *
ExpressionBuilder::makeGradEBTerm(const std::string & base_name, const std::string & var_name)
{
  EBTerm grad_term(
      getVarComps<RealVectorValue>(var_name), {3}, makeSecondEBTerm(base_name, var_name));
  _grad_coup_var_vecs[base_name].push_back(grad_term);
  _grad_coup_vars[var_name] = grad_term;
  return grad_term.cloneRoot();
}

ExpressionBuilder::EBTermNode *
ExpressionBuilder::makeSecondEBTerm(const std::string & base_name, const std::string & var_name)
{
  EBTerm second_term(getVarComps<RankTwoTensor>(var_name), {3, 3});
  _second_coup_var_vecs[base_name].push_back(second_term);
  _second_coup_vars[var_name] = second_term;
  return second_term.cloneRoot();
}

template <>
ExpressionBuilder::EBTerm
ExpressionBuilder::getEBMaterial<Real>(const std::string & var_name,
                                       const EBTermList & coupled_depend)
{
  return EBTerm(var_name.c_str(), EBMatDeriv<Real>(var_name, {1}, coupled_depend, 1));
}

template <>
ExpressionBuilder::EBTerm
ExpressionBuilder::getEBMaterial<RealVectorValue>(const std::string & var_name,
                                                  const EBTermList & coupled_depend)
{
  return EBTerm(getVarComps<RealVectorValue>(var_name),
                {3},
                EBMatDeriv<RealVectorValue>(var_name, {3}, coupled_depend, 1));
}

template <>
ExpressionBuilder::EBTerm
ExpressionBuilder::getEBMaterial<RankTwoTensor>(const std::string & var_name,
                                                const EBTermList & coupled_depend)
{
  return EBTerm(getVarComps<RankTwoTensor>(var_name),
                {3, 3},
                EBMatDeriv<RankTwoTensor>(var_name, {3, 3}, coupled_depend, 1));
}

template <typename T>
ExpressionBuilder::EBTermNode *
ExpressionBuilder::EBMatDeriv(const std::string & var_base,
                              std::vector<unsigned int> shape,
                              const EBTermList & coupled_depend,
                              unsigned int order)
{
  std::vector<std::string> mat_deriv_vec;
  bool add_to_props = false;

  // Expand shape
  if (shape.size() == 1 && shape[0] == 1)
    shape[0] = 3;
  else
    shape.push_back(3);

  // Get Coupled Property Derivative Names
  std::vector<std::string> first_derivs;
  for (auto & coupled_var : coupled_depend)
  {
    EBTermNode * var_deriv = coupled_var.getRoot()->getDeriv();
    if (var_deriv == NULL)
      return NULL;
    std::vector<std::string> coupled_deriv = var_deriv->fullStringify();
    first_derivs.insert(first_derivs.end(), coupled_deriv.begin(), coupled_deriv.end());
  }

  // Get Material Property Derivative Names
  if (order == 1)
    add_to_props = true;
  std::vector<std::vector<std::string>> mat_derivs(
      getMatDerivNames<T>(var_base, coupled_depend, add_to_props));

  if (order == 1)
  {
    for (unsigned int i = 0; i < mat_derivs[0].size(); ++i)
    {
      std::vector<std::string> var_deriv(3, "(");
      for (unsigned int j = 0; j < mat_derivs.size(); ++j)
        for (unsigned int k = 0; k < 3; ++k)
          var_deriv[k] += mat_derivs[j][i] + "*" + first_derivs[3 * j + k] + "+";
      for (auto & deriv : var_deriv)
        deriv.back() = ')';
      mat_deriv_vec.insert(mat_deriv_vec.begin(), var_deriv.begin(), var_deriv.end());
    }
    return new EBSymbolNode(
        mat_deriv_vec, shape, EBMatDeriv<T>(var_base, shape, coupled_depend, order + 1));
  }

  std::vector<std::string> second_derivs;
  for (auto & coupled_var : coupled_depend)
  {
    EBTermNode * var_deriv = coupled_var.getRoot()->getDeriv()->getDeriv();
    if (var_deriv == NULL)
      return NULL;
    std::vector<std::string> coupled_deriv = var_deriv->fullStringify();
    second_derivs.insert(second_derivs.end(), coupled_deriv.begin(), coupled_deriv.end());
  }

  // Get Material Property Derivative Names
  if (order == 2)
    add_to_props = true;
  std::vector<std::vector<std::vector<std::string>>> mat_second_derivs(
      getMatDerivNames<T>(var_base, coupled_depend, add_to_props, false));

  if (order == 2)
  {
    for (unsigned int i = 0; i < mat_derivs[0].size(); ++i)
    {
      std::vector<std::string> var_deriv(9, "(");
      for (unsigned int j = 0; j < mat_derivs.size(); ++j)
        for (unsigned int l = 0; l < 3; ++l)
          for (unsigned int m = 0; m < 3; ++m)
          {
            for (unsigned int k = 0; k < mat_derivs.size(); ++k)
              var_deriv[3 * l + m] += mat_second_derivs[j][k][i] + "*" + first_derivs[3 * j + l] +
                                      "*" + first_derivs[3 * k + m] + "+";
            var_deriv[3 * l + m] += mat_derivs[j][i] + "*" + second_derivs[9 * j + 3 * l + m] + "+";
          }
      for (auto & deriv : var_deriv)
        deriv.back() = ')';
      mat_deriv_vec.insert(mat_deriv_vec.begin(), var_deriv.begin(), var_deriv.end());
    }
    return new EBSymbolNode(
        mat_deriv_vec, shape, EBMatDeriv<T>(var_base, shape, coupled_depend, order + 1));
  }
  return NULL;
}

template <typename T>
std::vector<std::vector<std::string>>
ExpressionBuilder::getMatDerivNames(const std::string & var_base,
                                    const EBTermList & coupled_depend,
                                    bool add_to_props)
{
  std::vector<std::string> coupled_strings;
  for (auto & term : coupled_depend)
  {
    std::vector<std::string> term_string = term.getRoot()->fullStringify();
    coupled_strings.insert(coupled_strings.end(), term_string.begin(), term_string.end());
  }

  std::vector<std::string> derivative_names;
  for (unsigned int i = 0; i < coupled_strings.size(); ++i)
  {
    derivative_names.push_back("d" + var_base + std::to_string(i));
    if (add_to_props)
      _mat_prop_names.push_back(derivative_names.back() + ":=D[" + var_base + "," +
                                coupled_strings[i] + "]");
  }

  std::vector<std::vector<std::string>> result_vec(0);
  for (auto & deriv : derivative_names)
    result_vec.push_back(getVarComps<T>(deriv));

  return result_vec;
}

template <typename T>
std::vector<std::vector<std::vector<std::string>>>
ExpressionBuilder::getMatDerivNames(const std::string & var_base,
                                    const EBTermList & coupled_depend,
                                    bool add_to_props,
                                    bool dummy)
{
  if (dummy)
  {
  } // Suppress unused parameter warning
  std::vector<std::string> coupled_strings;
  for (auto & term : coupled_depend)
  {
    std::vector<std::string> term_string = term.getRoot()->fullStringify();
    coupled_strings.insert(coupled_strings.end(), term_string.begin(), term_string.end());
  }

  std::vector<std::vector<std::string>> derivative_names;
  for (unsigned int i = 0; i < coupled_strings.size(); ++i)
  {
    std::vector<std::string> sub_derivative_names;
    for (unsigned int j = 0; j < i; ++j)
    {
      sub_derivative_names.push_back("d2" + var_base +
                                     std::to_string(j * coupled_strings.size() + i));
    }
    for (unsigned int j = i; j < coupled_strings.size(); ++j)
    {
      sub_derivative_names.push_back("d2" + var_base +
                                     std::to_string(i * coupled_strings.size() + j));
      if (add_to_props)
        _mat_prop_names.push_back(sub_derivative_names.back() + ":=D[" + var_base + "," +
                                  coupled_strings[i] + "," + coupled_strings[j] + "]");
    }
    derivative_names.push_back(sub_derivative_names);
  }

  std::vector<std::vector<std::vector<std::string>>> result_vec(0);
  for (auto & deriv : derivative_names)
  {
    std::vector<std::vector<std::string>> sub_result;
    for (auto & sub_deriv : deriv)
      sub_result.push_back(getVarComps<T>(sub_deriv));
    result_vec.push_back(sub_result);
  }
  return result_vec;
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
    else
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
ExpressionBuilder::EBSymbolNode::stringify(std::vector<unsigned int> component,
                                           std::vector<unsigned int> deriv_comp) const
{
  if (_isTransposed)
    transposeComponent(component);

  unsigned int position = 0;
  for (unsigned int i = 0; i < component.size(); ++i)
  {
    unsigned int multiplier = 1;
    for (unsigned int j = i + 1; j < component.size(); ++j)
      multiplier *= _shape[j];
    for (unsigned int j = 0; j < deriv_comp.size(); ++j)
      multiplier *= 3;
    position += component[i] * multiplier;
  }

  for (unsigned int i = 0; i < deriv_comp.size(); ++i)
  {
    unsigned int multiplier = 1;
    for (unsigned int j = i + 1; j < deriv_comp.size(); ++j)
      multiplier *= 3;
    position += deriv_comp[i] * multiplier;
  }

  return _symbol[position];
}

std::string
ExpressionBuilder::EBTempIDNode::stringify(std::vector<unsigned int> component,
                                           std::vector<unsigned int> deriv_comp) const
{
  std::ostringstream s;
  s << '[' << _id << ']';
  component.clear(); // Suppresses unused parameter warning
  deriv_comp.clear();
  return s.str();
}

std::string
ExpressionBuilder::EBUnaryFuncTermNode::stringify(std::vector<unsigned int> component,
                                                  std::vector<unsigned int> deriv_comp) const
{
  if (_isTransposed)
    transposeComponent(component);

  switch (_type)
  {
    case GRAD:
      return gradRule(component, deriv_comp);
    case DIVERG:
      return divergenceRule(component, deriv_comp);
    case CURL:
      return curlRule(component, deriv_comp);
    case SIN:
    case COS:
    case TAN:
    case ABS:
    case LOG:
    case LOG2:
    case LOG10:
    case EXP:
    case SINH:
    case COSH:
      const char * name[] = {
          "sin", "cos", "tan", "abs", "log", "log2", "log10", "exp", "sinh", "cosh"};
      std::ostringstream s;
      s << name[_type] << '(' << _subnode->stringify(component, deriv_comp) << ')';
      return s.str();
  }
  return std::string();
}

ExpressionBuilder::EBTermNode *
ExpressionBuilder::EBUnaryFuncTermNode::takeDerivative() const
{
  EBTermNode * result_node = NULL;
  EBTermNode * new_left;
  EBTermNode * new_right;
  EBTermNode * cos_left;
  EBTermNode * pow_left;
  EBTermNode * cond_left;
  EBTermNode * log_left;
  EBTermNode * mult_left;
  switch (_type)
  {
    case SIN:
      new_left = new EBUnaryFuncTermNode(_subnode->clone(), EBUnaryFuncTermNode::NodeType::COS);
      new_right = _subnode->takeDerivative();
      result_node = new EBBinaryOpTermNode(
          new_left, new_right, EBBinaryOpTermNode::NodeType::MUL, _isTransposed);
      break;
    case COS:
      new_left = new EBUnaryFuncTermNode(_subnode->clone(), EBUnaryFuncTermNode::NodeType::COS);
      new_right =
          new EBUnaryOpTermNode(_subnode->takeDerivative(), EBUnaryOpTermNode::NodeType::NEG);
      result_node = new EBBinaryOpTermNode(
          new_left, new_right, EBBinaryOpTermNode::NodeType::MUL, _isTransposed);
      break;
    case TAN:
      cos_left = new EBUnaryFuncTermNode(_subnode->clone(), EBUnaryFuncTermNode::NodeType::COS);
      pow_left = new EBBinaryOpTermNode(
          cos_left, new EBNumberNode<Real>(2), EBBinaryOpTermNode::NodeType::POW, _isTransposed);
      new_left = new EBBinaryOpTermNode(
          new EBNumberNode<Real>(1), pow_left, EBBinaryOpTermNode::NodeType::DIV);
      new_right = _subnode->takeDerivative();
      result_node = new EBBinaryOpTermNode(
          new_left, new_right, EBBinaryOpTermNode::NodeType::MUL, _isTransposed);
      break;
    case ABS:
      cond_left = new EBBinaryOpTermNode(
          _subnode->clone(), new EBNumberNode<Real>(0), EBBinaryOpTermNode::NodeType::GREATER);
      new_left = new EBTernaryFuncTermNode(cond_left,
                                           new EBNumberNode<Real>(1),
                                           new EBNumberNode<Real>(-1),
                                           EBTernaryFuncTermNode::NodeType::CONDITIONAL);
      new_right = _subnode->takeDerivative();
      result_node = new EBBinaryOpTermNode(
          new_left, new_right, EBBinaryOpTermNode::NodeType::MUL, _isTransposed);
      break;
    case LOG:
      new_left = new EBBinaryOpTermNode(
          new EBNumberNode<Real>(1), _subnode->clone(), EBBinaryOpTermNode::NodeType::DIV);
      new_right = _subnode->takeDerivative();
      result_node = new EBBinaryOpTermNode(
          new_left, new_right, EBBinaryOpTermNode::NodeType::MUL, _isTransposed);
      break;
    case LOG2:
      log_left =
          new EBUnaryFuncTermNode(new EBNumberNode<Real>(2), EBUnaryFuncTermNode::NodeType::LOG);
      mult_left =
          new EBBinaryOpTermNode(_subnode->clone(), log_left, EBBinaryOpTermNode::NodeType::MUL);
      new_left = new EBBinaryOpTermNode(
          new EBNumberNode<Real>(1), mult_left, EBBinaryOpTermNode::NodeType::DIV);
      new_right = _subnode->takeDerivative();
      result_node = new EBBinaryOpTermNode(
          new_left, new_right, EBBinaryOpTermNode::NodeType::MUL, _isTransposed);
      break;
    case LOG10:
      log_left =
          new EBUnaryFuncTermNode(new EBNumberNode<Real>(10), EBUnaryFuncTermNode::NodeType::LOG);
      mult_left =
          new EBBinaryOpTermNode(_subnode->clone(), log_left, EBBinaryOpTermNode::NodeType::MUL);
      new_left = new EBBinaryOpTermNode(
          new EBNumberNode<Real>(1), mult_left, EBBinaryOpTermNode::NodeType::DIV);
      new_right = _subnode->takeDerivative();
      result_node = new EBBinaryOpTermNode(
          new_left, new_right, EBBinaryOpTermNode::NodeType::MUL, _isTransposed);
      break;
    case EXP:
      new_left = clone();
      new_right = _subnode->takeDerivative();
      result_node = new EBBinaryOpTermNode(
          new_left, new_right, EBBinaryOpTermNode::NodeType::MUL, _isTransposed);
      break;
    case SINH:
      new_left = new EBUnaryFuncTermNode(_subnode->clone(), EBUnaryFuncTermNode::NodeType::COSH);
      new_right = _subnode->takeDerivative();
      result_node = new EBBinaryOpTermNode(
          new_left, new_right, EBBinaryOpTermNode::NodeType::MUL, _isTransposed);
      break;
    case COSH:
      new_left = new EBUnaryFuncTermNode(_subnode->clone(), EBUnaryFuncTermNode::NodeType::SINH);
      new_right = _subnode->takeDerivative();
      result_node = new EBBinaryOpTermNode(
          new_left, new_right, EBBinaryOpTermNode::NodeType::MUL, _isTransposed);
      break;
    case GRAD:
      new_left = _subnode->takeDerivative();
      new_left->stringify({0}, {0});
      result_node = new EBUnaryFuncTermNode(
          _subnode->takeDerivative(), EBUnaryFuncTermNode::NodeType::GRAD, _isTransposed);
      break;
    case DIVERG:
      result_node = new EBUnaryFuncTermNode(
          _subnode->takeDerivative(), EBUnaryFuncTermNode::NodeType::DIVERG, _isTransposed);
      break;
    case CURL:
      result_node = new EBUnaryFuncTermNode(
          _subnode->takeDerivative(), EBUnaryFuncTermNode::NodeType::CURL, _isTransposed);
      break;
  }
  return result_node;
}

std::string
ExpressionBuilder::EBUnaryFuncTermNode::gradRule(std::vector<unsigned int> component,
                                                 std::vector<unsigned int> deriv_comp) const
{
  EBTermNode * derivative = _subnode->takeDerivative();
  deriv_comp.insert(deriv_comp.begin(), component.back());
  if (component.size() == 1)
    component[0] = 0;
  else
    component.pop_back();
  return derivative->stringify(component, deriv_comp);
}

std::string
ExpressionBuilder::EBUnaryFuncTermNode::divergenceRule(std::vector<unsigned int> component,
                                                       std::vector<unsigned int> deriv_comp) const
{
  std::ostringstream s;
  EBTermNode * derivative = _subnode->takeDerivative();
  deriv_comp.insert(deriv_comp.begin(), 0);

  s << '(';
  for (unsigned int i = 0; i < 3; ++i)
  {
    deriv_comp[0] = i;
    component[0] = i;
    s << derivative->stringify(component, deriv_comp) << '+';
  }
  std::string result = s.str();
  result.back() = ')';
  return result;
}

std::string
ExpressionBuilder::EBUnaryFuncTermNode::curlRule(std::vector<unsigned int> component,
                                                 std::vector<unsigned int> deriv_comp) const
{
  std::ostringstream s;
  EBTermNode * derivative = _subnode->takeDerivative();
  std::vector<unsigned int> temp_comp = component;
  std::vector<unsigned int> temp_deriv = deriv_comp;

  temp_deriv.insert(temp_deriv.begin(), (component[0] + 1) % 3);
  temp_comp.back() = (component[0] + 2) % 3;
  s << '(' << derivative->stringify(temp_comp, temp_deriv) << '-';

  temp_deriv[0] = (component[0] + 2) % 3;
  temp_comp.back() = (component[0] + 1) % 3;
  s << derivative->stringify(temp_comp, temp_deriv) << ')';
  return s.str();
}

std::string
ExpressionBuilder::EBUnaryFuncTermNode::laplaceRule(std::vector<unsigned int> component,
                                                    std::vector<unsigned int> deriv_comp) const
{
  std::ostringstream s;
  unsigned int comp_position = deriv_comp.size();
  deriv_comp.push_back(0);
  deriv_comp.push_back(0);

  EBTermNode * derivative = _subnode->takeDerivative();
  derivative = derivative->takeDerivative();

  s << '(';
  for (unsigned int i = 0; i < 3; ++i)
  {
    deriv_comp[comp_position] = i;
    deriv_comp[comp_position + 1] = i;
    s << derivative->stringify(component, deriv_comp) << '+';
  }
  std::string result = s.str();
  result.back() = ')';
  return result;
}

std::vector<unsigned int>
ExpressionBuilder::EBUnaryFuncTermNode::setShape()
{
  std::vector<unsigned int> sub_shape = _subnode->getShape();
  switch (_type)
  {
    case GRAD:
      if (sub_shape.back() == 1)
        sub_shape.back() = 3;
      else
        sub_shape.push_back(3);
      _shape = sub_shape;
      break;
    case DIVERG:
      if ((sub_shape.size() != 1 || sub_shape[0] != 3))
        mooseError("Improper shape for unary node");
      _shape = std::vector<unsigned int>(1, 1);
      break;
    case CURL:
      if ((sub_shape.size() != 1 || sub_shape[0] != 3))
        mooseError("Improper shape for unary node");
      _shape = sub_shape;
      break;
    case SIN:
    case COS:
    case TAN:
    case ABS:
    case LOG:
    case LOG2:
    case LOG10:
    case EXP:
    case SINH:
    case COSH:
      _shape = sub_shape; // Acts component-wise
      break;
  }
  return _shape;
}

std::string
ExpressionBuilder::EBUnaryOpTermNode::stringify(std::vector<unsigned int> component,
                                                std::vector<unsigned int> deriv_comp) const
{
  if (_isTransposed)
    transposeComponent(component);

  const char * name[] = {"-", "!"};
  std::ostringstream s;

  switch (_type)
  {
    case NEG:
    case LOGICNOT:
      s << name[_type];

      if (_subnode->precedence() > precedence())
        s << '(' << _subnode->stringify(component, deriv_comp) << ')';
      else
        s << _subnode->stringify(component, deriv_comp);

      return s.str();
  }
  return std::string();
}

ExpressionBuilder::EBTermNode *
ExpressionBuilder::EBUnaryOpTermNode::takeDerivative() const
{
  EBTermNode * result_node = NULL;
  switch (_type)
  {
    case NEG:
      result_node = new EBUnaryOpTermNode(
          _subnode->takeDerivative(), EBUnaryOpTermNode::NodeType::NEG, _isTransposed);
      break;
    case LOGICNOT:
      result_node = clone();
  }
  return result_node;
}

std::vector<unsigned int>
ExpressionBuilder::EBUnaryOpTermNode::setShape()
{
  std::vector<unsigned int> sub_shape = _subnode->getShape();

  switch (_type)
  {
    case NEG:
      _shape = sub_shape;
      break;
    case LOGICNOT:
      if ((sub_shape.size() != 1 || sub_shape[0] != 1))
        mooseError("Improper shape for unary node");
      _shape = sub_shape;
      break;
  }

  return _shape;
}

std::string
ExpressionBuilder::EBBinaryFuncTermNode::stringify(std::vector<unsigned int> component,
                                                   std::vector<unsigned int> deriv_comp) const
{
  if (_isTransposed)
    transposeComponent(component);

  const char * name[] = {"min", "max", "atan2", "hypot", "plog"};
  std::ostringstream s;
  s << name[_type] << '(' << _left->stringify(component, deriv_comp) << ','
    << _left->stringify(component, deriv_comp) << ')';
  return s.str();
}

ExpressionBuilder::EBTermNode *
ExpressionBuilder::EBBinaryFuncTermNode::takeDerivative() const
{
  mooseError("Derivative not yet defined for Binary Functions");
  return NULL;
}

std::vector<unsigned int>
ExpressionBuilder::EBBinaryFuncTermNode::setShape()
{
  std::vector<unsigned int> left_shape = _left->getShape();
  std::vector<unsigned int> right_shape = _right->getShape();

  if (left_shape.size() != 1 || left_shape[0] != 1)
    mooseError("Improper shape for binary function node");
  if (right_shape.size() != 1 || right_shape[0] != 1)
    mooseError("Improper shape for binary function node");
  _shape = left_shape;
  return left_shape;
}

std::string
ExpressionBuilder::EBBinaryOpTermNode::stringify(std::vector<unsigned int> component,
                                                 std::vector<unsigned int> deriv_comp) const
{
  if (_isTransposed)
    transposeComponent(component);

  const char * name[] = {"+", "-", "*", "/", "%", "^", "<", ">", "<=", ">=", "=", "!="};
  std::ostringstream s;

  if (_type == MUL)
    return multRule(component, deriv_comp);

  if (_type == CROSS)
    return crossRule(component, deriv_comp);

  std::vector<unsigned int> left_component = component;
  std::vector<unsigned int> right_component = component;

  if (_left->precedence() > precedence())
    s << '(' << _left->stringify(left_component, deriv_comp) << ')';
  else
    s << _left->stringify(left_component, deriv_comp);

  s << name[_type];

  // these operators are left associative at equal precedence
  // (this matters for -,/,&,^ but not for + and *)
  if (_right->precedence() > precedence() ||
      (_right->precedence() == precedence() && (_type == SUB || _type == DIV)))
    s << '(' << _right->stringify(right_component, deriv_comp) << ')';
  else
    s << _right->stringify(right_component, deriv_comp);

  return s.str();
}

ExpressionBuilder::EBTermNode *
ExpressionBuilder::EBBinaryOpTermNode::takeDerivative() const
{
  EBTermNode * result_node = NULL;
  EBTermNode * left_node;
  EBTermNode * right_node;
  EBTermNode * pow_node;
  EBTermNode * mult_node;
  EBTermNode * new_pow;
  switch (_type)
  {
    case ADD:
    case SUB:
      result_node = new EBBinaryOpTermNode(
          _left->takeDerivative(), _right->takeDerivative(), _type, _isTransposed);
      break;
    case MUL:
      left_node =
          new EBBinaryOpTermNode(_left->clone(), _right->takeDerivative(), _type, _isTransposed);
      right_node =
          new EBBinaryOpTermNode(_left->takeDerivative(), _right->clone(), _type, _isTransposed);
      result_node =
          new EBBinaryOpTermNode(left_node, right_node, EBBinaryOpTermNode::NodeType::ADD);
      break;
    case DIV:
      pow_node = new EBBinaryOpTermNode(
          _right->clone(), new EBNumberNode<Real>(-1), EBBinaryOpTermNode::NodeType::POW);
      mult_node = new EBBinaryOpTermNode(
          _left->clone(), pow_node, EBBinaryOpTermNode::NodeType::MUL, _isTransposed);
      result_node = mult_node->takeDerivative();
      break;
    case POW:
      new_pow = new EBBinaryOpTermNode(
          _right->clone(), new EBNumberNode<Real>(1), EBBinaryOpTermNode::NodeType::SUB);
      pow_node = new EBBinaryOpTermNode(_left->clone(), new_pow, EBBinaryOpTermNode::NodeType::POW);
      result_node = new EBBinaryOpTermNode(
          _right->clone(), pow_node, EBBinaryOpTermNode::NodeType::MUL, _isTransposed);
      break;
    case CROSS:
      left_node = new EBBinaryOpTermNode(_right->clone(),
                                         _left->takeDerivative(),
                                         EBBinaryOpTermNode::NodeType::CROSS,
                                         _isTransposed);
      right_node = new EBBinaryOpTermNode(_right->takeDerivative(),
                                          _left->clone(),
                                          EBBinaryOpTermNode::NodeType::CROSS,
                                          _isTransposed);
      result_node =
          new EBBinaryOpTermNode(left_node, right_node, EBBinaryOpTermNode::NodeType::ADD);
      break;
    case MOD:
    case LESS:
    case GREATER:
    case LESSEQ:
    case GREATEREQ:
    case EQ:
    case NOTEQ:
      mooseError("Derivative of conditional and mod operators not defined");
      break;
  }
  return result_node;
}

std::vector<unsigned int>
ExpressionBuilder::EBBinaryOpTermNode::setShape()
{
  std::vector<unsigned int> left_shape = _left->getShape();
  std::vector<unsigned int> right_shape = _right->getShape();

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
ExpressionBuilder::EBBinaryOpTermNode::multRule(std::vector<unsigned int> component,
                                                std::vector<unsigned int> deriv_comp) const
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
      s << _left->stringify(current_comp, deriv_comp) << '*'
        << _right->stringify(current_comp, deriv_comp) << '+';
    }
    std::string return_string = s.str();
    return_string.back() = ')';
    return return_string;
  }

  std::vector<unsigned int> zero_vector(1, 0);
  if (left_dims[0] == 1 && left_dims.size() == 1)
  {
    s << _left->stringify(zero_vector, deriv_comp) << '*'
      << (_right->stringify(component, deriv_comp));
    return s.str();
  }

  if (right_dims[0] == 1 && right_dims.size() == 1)
  {
    s << _left->stringify(component, deriv_comp) << '*'
      << _right->stringify(zero_vector, deriv_comp);
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
    s << _left->stringify(left_component, deriv_comp) << '*'
      << _right->stringify(right_component, deriv_comp) << '+';
  }
  std::string finished = s.str();
  finished.back() = ')';
  return finished;
}

std::string
ExpressionBuilder::EBBinaryOpTermNode::crossRule(std::vector<unsigned int> component,
                                                 std::vector<unsigned int> deriv_comp) const
{
  std::vector<unsigned int> comp1(1, (component[0] + 1) % 3);
  std::vector<unsigned int> comp2(1, (component[0] + 2) % 3);
  std::ostringstream s;

  s << '(' << _left->stringify(comp1, deriv_comp) << '*' << _right->stringify(comp2, deriv_comp);
  s << '-' << _left->stringify(comp2, deriv_comp) << '*' << _right->stringify(comp1, deriv_comp)
    << ')';
  return s.str();
}

std::string
ExpressionBuilder::EBTernaryFuncTermNode::stringify(std::vector<unsigned int> component,
                                                    std::vector<unsigned int> deriv_comp) const
{
  if (_isTransposed)
    transposeComponent(component);

  std::vector<unsigned int> zero_vector(1, 0);
  const char * name[] = {"if"};
  std::ostringstream s;
  s << name[_type] << '(' << _left->stringify(zero_vector, deriv_comp) << ','
    << _middle->stringify(component, deriv_comp) << ',' << _right->stringify(component, deriv_comp)
    << ')';
  return s.str();
}

ExpressionBuilder::EBTermNode *
ExpressionBuilder::EBTernaryFuncTermNode::takeDerivative() const
{
  EBTermNode * result_node = NULL;
  switch (_type)
  {
    case CONDITIONAL:
      result_node = new EBTernaryFuncTermNode(_left->clone(),
                                              _middle->takeDerivative(),
                                              _right->takeDerivative(),
                                              _type,
                                              _isTransposed);
      break;
  }
  return result_node;
}

std::vector<unsigned int>
ExpressionBuilder::EBTernaryFuncTermNode::setShape()
{
  std::vector<unsigned int> left_shape = _left->getShape();
  std::vector<unsigned int> right_shape = _right->getShape();
  std::vector<unsigned int> middle_shape = _middle->getShape();

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
UNARY_FUNC_IMPLEMENT(grad, GRAD)
UNARY_FUNC_IMPLEMENT(divergence, DIVERG)
UNARY_FUNC_IMPLEMENT(curl, CURL)

ExpressionBuilder::EBTerm
laplacian(const ExpressionBuilder::EBTerm & term)
{
  mooseAssert(term._root != NULL, "Empty term provided as argument of function laplacian()");
  if (term.getShape()[0] == 1)
    return divergence(grad(term));
  else
    return grad(divergence(term)) - curl(curl(term));
}

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
