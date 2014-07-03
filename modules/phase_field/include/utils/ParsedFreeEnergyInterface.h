#ifndef PARSEDFREEENERGYINTERFACE_H
#define PARSEDFREEENERGYINTERFACE_H

#include "MooseVariable.h"
#include "libmesh/fparser_ad.hh"

/**
 * ParsedFreeEnergyInterface allows a free energy functionals to be provided as a parsed
 * expression in the input file. It uses automatic differentiation
 * to generate the necessary derivative.
 * This requires the autodiff patch (https://github.com/libMesh/libmesh/pull/238)
 * to Function Parser in libmesh.
 */
template<class T>
class ParsedFreeEnergyInterface : public T
{
public:
  ParsedFreeEnergyInterface(const std::string & name, InputParameters parameters);
  virtual ~ParsedFreeEnergyInterface();

  /// set the function parameter vector that is pased to FParser
  void updateFuncParams();

  /// as partial template specialization is not allowed in C++ we have to implement this as astatic method
  static InputParameters validParams();

  /// derivative shortcut functions
  Real firstDerivative(unsigned int i) { return _first_derivatives[i]->Eval(_func_params); }
  Real secondDerivative(unsigned int i) { return _second_derivatives[i]->Eval(_func_params); }
  Real thirdDerivative(unsigned int i) { return _third_derivatives[i]->Eval(_func_params); }

protected:
  /// Shorthand for an autodiff function parser object
  typedef FunctionParserADBase<Real> ADFunction;

  /// Free energy function parser object and its derivatives
  ADFunction * _function;

  /// Free energy function derivatives \f$ dF/dv_i \f$
  std::vector<ADFunction *> _first_derivatives;

  /// Free energy function derivatives \f$ d^2F/dudv_i \f$
  std::vector<ADFunction *> _second_derivatives;

  /// Free energy function derivatives \f$ d^3F/du^2dv_i \f$
  std::vector<ADFunction *> _third_derivatives;

  /// Number of material properties used
  unsigned int _nmat_props;

  /// Material properties used by the kernel
  std::vector<MaterialProperty<Real> *> _mat_props;

  /// Number of coupled variables
  unsigned int _nargs;

  /// Number of variables
  unsigned int _nvars;

  /// String vector of all variable names (kernel variable and coupled variables).
  std::vector<std::string> _var_names;

  /// Values for all variables v_i (kernel variable u==v_0 and coupled variables v_1..v_nargs)
  std::vector<VariableValue *> _vars;

  /// Gradients for all variables (kernel variable and coupled variables)
  std::vector<VariableGradient *> _grad_vars;

  /// Array to stage the parameters passed to the functions when calling Eval.
  Real * _func_params;

private:
  void functionsDerivative();
  void functionsOptimize();
};


template<class T>
InputParameters
ParsedFreeEnergyInterface<T>::validParams()
{
  InputParameters params = ::validParams<T>();

  // Constants
  params.addParam<std::vector<std::string> >(
    "constant_names", std::vector<std::string>(),
    "Vector of constants used in the parsed function (use this for kB etc.)");
  params.addParam<std::vector<Real> >(
    "constant_values", std::vector<Real>(),
    "Vector of values for the constants in constant_names");
  params.addRequiredParam<std::string>("function", "FParser function expression for the free energy F()");

  // Material properties
  params.addParam<std::vector<std::string> >(
    "material_property_names", std::vector<std::string>(),
    "Vector of material properties used in the parsed function (these should have a zero gradient!)");

  // Coupled variables
  params.addCoupledVar("args", "Coupled variables used in F() - use vector coupling");

  return params;
}

template<class T>
ParsedFreeEnergyInterface<T>::ParsedFreeEnergyInterface(const std::string & name, InputParameters parameters) :
    T(name, parameters),
    _nargs(this->coupledComponents("args")),
    _nvars(_nargs + 1)
{
  // values and gradients for all variables
  _vars.resize(_nvars);
  _vars[0] = &(this->_u);
  _grad_vars.resize(_nvars);
  _grad_vars[0] = &(this->_grad_u);
  for (unsigned int i = 0; i < _nargs; ++i)
  {
    _vars[i+1] = &(this->coupledValue("args",i));
    _grad_vars[i+1] = &(this->coupledGradient("args",i));
  }

  // 'variables' argument for fparser
  std::string variables = this->_var.name();

  // fetch names of variables in args and kernel var
  _var_names.resize(_nvars);
  _var_names[0] = this->_var.name();
  for (unsigned int i = 0; i < _nargs; ++i)
  {
    _var_names[i+1] = this->getVar("args", i)->name();
    variables += "," + _var_names[i+1];
  }

  // get material property names
  std::vector<std::string> mat_prop_names = this->template getParam<std::vector<std::string> >("material_property_names");
  _nmat_props = mat_prop_names.size();

  // get all material properties
  _mat_props.resize(_nmat_props);
  for (unsigned int i = 0; i < _nmat_props; ++i)
  {
    _mat_props[i] = &(this->template getMaterialProperty<Real>(mat_prop_names[i]));
    variables += "," + mat_prop_names[i];
  }

  // get and check constant vectors
  std::vector<std::string> constant_names = this->template getParam<std::vector<std::string> >("constant_names");
  std::vector<Real> constant_values = this->template getParam<std::vector<Real> >("constant_values");
  if (constant_names.size() != constant_values.size())
    mooseError("The parameter vectors constant_names and constant_values must have equal length.");

  // initialize constants
  _function = new ADFunction();
  for (unsigned int i = 0; i < constant_names.size(); ++i)
    if (!_function->AddConstant(constant_names[i], constant_values[i]))
      mooseError("Invalid constant name in ParsedFreeEnergyInterface");

  // build the base function
  if (_function->Parse(this->template getParam<std::string>("function"), variables) >= 0)
     mooseError(std::string("Invalid function ParsedFreeEnergyInterface.") + _function->ErrorMsg());

  functionsDerivative();
  functionsOptimize();

  // create parameter passing buffer
  _func_params = new Real[_nvars + _nmat_props];
}

template<class T>
void
ParsedFreeEnergyInterface<T>::functionsDerivative()
{
  // compute first derivatives
  _first_derivatives.resize(_nvars);
  for (unsigned int i = 0; i < _nvars; ++i)
  {
    _first_derivatives[i] = new ADFunction(*_function);
    if (!_first_derivatives[i]->AutoDiff(_var_names[i]))
      mooseError("Automatic differentiation of the free energy failed.");
  }

  // compute second derivatives
  _second_derivatives.resize(_nvars);
  for (unsigned int i = 0; i < _nvars; ++i)
  {
    _second_derivatives[i] = new ADFunction(*_first_derivatives[i]);

    if (!_second_derivatives[i]->AutoDiff(_var_names[0]))
      mooseError("Automatic differentiation of the free energy failed.");
  }

  // compute third derivatives
  _third_derivatives.resize(_nvars);
  for (unsigned int i = 0; i < _nvars; ++i)
  {
    _third_derivatives[i] = new ADFunction(*_second_derivatives[i]);

    if (!_third_derivatives[i]->AutoDiff(_var_names[0]))
      mooseError("Automatic differentiation of the free energy failed.");
  }
}

template<class T>
void
ParsedFreeEnergyInterface<T>::functionsOptimize()
{
  _function->Optimize();

  // optimize first derivatives
  for (unsigned int i = 0; i < _nvars; ++i)
    _first_derivatives[i]->Optimize();

  // optimize second derivatives
  for (unsigned int i = 0; i < _nvars; ++i)
    _second_derivatives[i]->Optimize();

  // compute third derivatives
  for (unsigned int i = 0; i < _nvars; ++i)
    _third_derivatives[i]->Optimize();
}

template<class T>
void
ParsedFreeEnergyInterface<T>::updateFuncParams()
{
  // setup function parameters (coupled variables)
  for (unsigned int i = 0; i < _nvars; ++i)
    _func_params[i] = (*_vars[i])[this->_qp];

  // setup function parameters (material properties)
  for (unsigned int i = 0; i < _nmat_props; ++i)
    _func_params[i + _nvars] = (*_mat_props[i])[this->_qp];
}

template<class T>
ParsedFreeEnergyInterface<T>::~ParsedFreeEnergyInterface()
{
  delete[] _func_params;
}

#endif // PARSEDFREEENERGYINTERFACE_H
