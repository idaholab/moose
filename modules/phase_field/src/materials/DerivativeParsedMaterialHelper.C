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
  InputParameters params = validParams<ParsedMaterialHelper>();
  params.addClassDescription("Parsed Function Material with automatic derivatives.");
  params.addDeprecatedParam<bool>("third_derivatives", "Flag to indicate if third derivatives are needed", "Use derivative_order instead.");
  params.addParam<unsigned int>("derivative_order", 3, "Maximum order of derivatives taken");

  return params;
}

DerivativeParsedMaterialHelper::DerivativeParsedMaterialHelper(const std::string & name,
                                                               InputParameters parameters,
                                                               VariableNameMappingMode map_mode) :
    ParsedMaterialHelper(name, parameters, map_mode),
    //_derivative_order(getParam<unsigned int>("derivative_order"))
    _derivative_order(isParamValid("third_derivatives") ? (getParam<bool>("third_derivatives") ? 3 : 2) : getParam<unsigned int>("derivative_order"))
{
}

DerivativeParsedMaterialHelper::~DerivativeParsedMaterialHelper()
{
  for (unsigned int i = 0; i < _derivatives.size(); ++i)
    delete _derivatives[i].second;
}

void DerivativeParsedMaterialHelper::functionsPostParse()
{
  // optimize base function
  ParsedMaterialHelper::functionsOptimize();

  // generate derivatives
  assembleDerivatives();
}

/**
 * Peform a breadth first construction of all requeste derivatives.
 */
void
DerivativeParsedMaterialHelper::assembleDerivatives()
{
  // need to check for zero derivatives here, otherwise at least one order is generated
  if (_derivative_order < 1) return;

  // set up deque
  std::deque<QueueItem> queue;
  queue.push_back(QueueItem(_func_F));

  // generate derivatives until the queue is exhausted
  while (!queue.empty())
  {
    QueueItem current = queue.front();
    queue.pop_front();

    // all permutations of one set of derivatives are equal, so we make sure to generate only one each
    unsigned int first = current._dargs.empty() ? 0 : current._dargs.back();

    // add necessary derivative stepos
    for (unsigned int i = first; i < _nargs; ++i)
    {
      // here we will eventually check if we need to create more variables to hold material property derivatives
      // if material property derivative needed)
      // {
      //   // add variable to the current parent and..
      //   current._F->AddVariable(newvarname);
      //
      //   // ..all siblings in the queue
      //   for (std::deque<QueueItem>::iterator j = queue.begin(); j != queue.end(); ++j)
      //     j->_F->AddVariable(newvarname);
      // }

      // construct new derivative
      QueueItem newitem = current;
      newitem._dargs.push_back(i);

      // build derivative
      newitem._F = new ADFunction(*current._F);
      if (newitem._F->AutoDiff(_variable_names[i]) != -1)
        mooseError("Failed to take order " << newitem._dargs.size() << " derivative in material " << _name);

      // optimize and compile
      if (!_disable_fpoptimizer)
        newitem._F->Optimize();
      if (_enable_jit && !newitem._F->JITCompile())
        mooseWarning("Failed to JIT compile expression, falling back to byte code interpretation.");

      // generate material property argument vector
      std::vector<std::string> darg_names(0);
      for (unsigned int j = 0; j < newitem._dargs.size(); ++j)
        darg_names.push_back(_variable_names[newitem._dargs[j]]);

      // append to list of derivatives if the derivative is non-vanishing
      if (!newitem._F->isZero())
      {
        Derivative newderivative;
        newderivative.first = &declarePropertyDerivative<Real>(_F_name, darg_names);
        newderivative.second = newitem._F;
        _derivatives.push_back(newderivative);
      }

      // push item to queue if further differentiation is required
      if (newitem._dargs.size() < _derivative_order)
        queue.push_back(newitem);
    }
  }
}

// TODO: computeQpProperties()
void
DerivativeParsedMaterialHelper::computeProperties()
{
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    // fill the parameter vector, apply tolerances
    for (unsigned int i = 0; i < _nargs; ++i)
    {
      if (_tol[i] < 0.0)
        _func_params[i] = (*_args[i])[_qp];
      else
      {
        Real a = (*_args[i])[_qp];
        _func_params[i] = a < _tol[i] ? _tol[i] : (a > 1.0 - _tol[i] ? 1.0 - _tol[i] : a);
      }
    }

    // insert material property values
    unsigned int nmat_props = _mat_prop_descriptors.size();
    for (unsigned int i = 0; i < nmat_props; ++i)
      _func_params[i + _nargs] = _mat_prop_descriptors[i].value()[_qp];

    // set function value
    if (_prop_F)
      (*_prop_F)[_qp] = evaluate(_func_F);

    // set derivatives
    for (unsigned int i = 0; i < _derivatives.size(); ++i)
      (*_derivatives[i].first)[_qp] = evaluate(_derivatives[i].second);
  }
}
