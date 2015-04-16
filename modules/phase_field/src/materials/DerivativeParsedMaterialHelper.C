/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "DerivativeParsedMaterialHelper.h"

template<>
InputParameters validParams<DerivativeParsedMaterialHelper>()
{
  InputParameters params = ParsedMaterialHelper<DerivativeFunctionMaterialBase>::validParams();
  params.addClassDescription("Parsed Function Material with automatic derivatives.");
  return params;
}

DerivativeParsedMaterialHelper::DerivativeParsedMaterialHelper(const std::string & name,
                                                               InputParameters parameters,
                                                               VariableNameMappingMode map_mode) :
    ParsedMaterialHelper<DerivativeFunctionMaterialBase>(name, parameters, map_mode)
{
}

DerivativeParsedMaterialHelper::~DerivativeParsedMaterialHelper()
{
  for (unsigned int i = 0; i < _nargs; ++i)
  {
    delete _func_dF[i];

    for (unsigned int j = i; j < _nargs; ++j)
    {
      delete _func_d2F[i][j];

      if (_third_derivatives)
        for (unsigned int k = j; k < _nargs; ++k)
          delete _func_d3F[i][j][k];
    }
  }
}

void DerivativeParsedMaterialHelper::functionsPostParse()
{
  functionsDerivative();
  functionsOptimize();
}

void DerivativeParsedMaterialHelper::functionsDerivative()
{
  unsigned int i, j, k;

  // first derivatives
  _func_dF.resize(_nargs);
  _func_d2F.resize(_nargs);
  _func_d3F.resize(_nargs);
  for (i = 0; i < _nargs; ++i)
  {
    _func_dF[i] = new ADFunction(*_func_F);
    if (_func_dF[i]->AutoDiff(_arg_names[i]) != -1)
      mooseError("Failed to take first derivative w.r.t. "
                 << _arg_names[i]);

    // second derivatives
    _func_d2F[i].resize(_nargs);
    _func_d3F[i].resize(_nargs);
    for (j = i; j < _nargs; ++j)
    {
      _func_d2F[i][j] = new ADFunction(*_func_dF[i]);
      if (_func_d2F[i][j]->AutoDiff(_arg_names[j]) != -1)
        mooseError("Failed to take second derivative w.r.t. "
                   << _arg_names[i] << " and " << _arg_names[j]);

      // third derivatives
      if (_third_derivatives)
      {
        _func_d3F[i][j].resize(_nargs);
        for (k = j; k < _nargs; ++k)
        {
          _func_d3F[i][j][k] = new ADFunction(*_func_d2F[i][j]);
          if (_func_d3F[i][j][k]->AutoDiff(_arg_names[k]) != -1)
            mooseError("Failed to take third derivative w.r.t. "
                       << _arg_names[i] << ", " << _arg_names[j] << ", and " << _arg_names[k]);
        }
      }
    }
  }
}

void DerivativeParsedMaterialHelper::functionsOptimize()
{
  unsigned int i, j, k;

  // base function
  ParsedMaterialHelper<DerivativeFunctionMaterialBase>::functionsOptimize();

  // optimize first derivatives
  for (i = 0; i < _nargs; ++i)
  {
    if (!_disable_fpoptimizer)
      _func_dF[i]->Optimize();
    if (_enable_jit && !_func_dF[i]->JITCompile())
      mooseWarning("Failed to JIT compile expression, falling back to byte code interpretation.");

    // if the derivative vanishes set the function back to NULL
    if (_func_dF[i]->isZero())
    {
      delete _func_dF[i];
      _func_dF[i] = NULL;
    }

    // optimize second derivatives
    for (j = i; j < _nargs; ++j)
    {
      if (!_disable_fpoptimizer)
        _func_d2F[i][j]->Optimize();
      if (_enable_jit && !_func_d2F[i][j]->JITCompile())
        mooseWarning("Failed to JIT compile expression, falling back to byte code interpretation.");

      // if the derivative vanishes set the function back to NULL
      if (_func_d2F[i][j]->isZero())
      {
        delete _func_d2F[i][j];
        _func_d2F[i][j] = NULL;
      }

      // optimize third derivatives
      if (_third_derivatives)
      {
        for (k = j; k < _nargs; ++k)
        {
          if (!_disable_fpoptimizer)
            _func_d3F[i][j][k]->Optimize();
          if (_enable_jit && !_func_d3F[i][j][k]->JITCompile())
            mooseWarning("Failed to JIT compile expression, falling back to byte code interpretation.");

          // if the derivative vanishes set the function back to NULL
          if (_func_d3F[i][j][k]->isZero())
          {
            delete _func_d3F[i][j][k];
            _func_d3F[i][j][k] = NULL;
          }
        }
      }
    }
  }
}

// TODO: computeQpProperties()
void
DerivativeParsedMaterialHelper::computeProperties()
{
  unsigned int i, j, k;
  Real a;

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    // fill the parameter vector, apply tolerances
    for (i = 0; i < _nargs; ++i)
    {
      if (_tol[i] < 0.0)
        _func_params[i] = (*_args[i])[_qp];
      else
      {
        a = (*_args[i])[_qp];
        _func_params[i] = a < _tol[i] ? _tol[i] : (a > 1.0 - _tol[i] ? 1.0 - _tol[i] : a);
      }
    }

    // insert material property values
    for (i = 0; i < _nmat_props; ++i)
      _func_params[i + _nargs] = (*_mat_props[i])[_qp];

    // set function value
    if (_prop_F)
      (*_prop_F)[_qp] = evaluate(_func_F);

    for (i = 0; i < _nargs; ++i)
    {
      if (_prop_dF[i])
        (*_prop_dF[i])[_qp] = evaluate(_func_dF[i]);

      // second derivatives
      for (j = i; j < _nargs; ++j)
      {
        if (_prop_d2F[i][j])
          (*_prop_d2F[i][j])[_qp] = evaluate(_func_d2F[i][j]);

        // third derivatives
        if (_third_derivatives)
          for (k = j; k < _nargs; ++k)
            if (_prop_d3F[i][j][k])
              (*_prop_d3F[i][j][k])[_qp] = evaluate(_func_d3F[i][j][k]);
      }
    }
  }
}
