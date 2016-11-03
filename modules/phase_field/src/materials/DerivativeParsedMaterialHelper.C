/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "DerivativeParsedMaterialHelper.h"
#include "Conversion.h"

#include <deque>

// libmesh includes
#include "libmesh/quadrature.h"

template<>
InputParameters validParams<DerivativeParsedMaterialHelper>()
{
  InputParameters params = validParams<ParsedMaterialHelper>();
  params.addClassDescription("Parsed Function Material with automatic derivatives.");
  params.addDeprecatedParam<bool>("third_derivatives", "Flag to indicate if third derivatives are needed", "Use derivative_order instead.");
  params.addParam<unsigned int>("derivative_order", 3, "Maximum order of derivatives taken");

  return params;
}

DerivativeParsedMaterialHelper::DerivativeParsedMaterialHelper(const InputParameters & parameters,
                                                               VariableNameMappingMode map_mode) :
    ParsedMaterialHelper(parameters, map_mode),
    //_derivative_order(getParam<unsigned int>("derivative_order"))
    _dmatvar_base("matpropautoderiv"),
    _dmatvar_index(0),
    _derivative_order(isParamValid("third_derivatives") ? (getParam<bool>("third_derivatives") ? 3 : 2) : getParam<unsigned int>("derivative_order"))
{
}

void
DerivativeParsedMaterialHelper::functionsPostParse()
{
  // optimize base function
  ParsedMaterialHelper::functionsOptimize();

  // generate derivatives
  assembleDerivatives();

  // force a value update to get the property at least once and register it for the dependencies
  unsigned int nmat_props = _mat_prop_descriptors.size();
  for (unsigned int i = 0; i < nmat_props; ++i)
    _mat_prop_descriptors[i].value();
}

ParsedMaterialHelper::MatPropDescriptorList::iterator
DerivativeParsedMaterialHelper::findMatPropDerivative(const FunctionMaterialPropertyDescriptor & m)
{
  std::string name = m.getPropertyName();
  for (MatPropDescriptorList::iterator i = _mat_prop_descriptors.begin(); i != _mat_prop_descriptors.end(); ++i)
    if (i->getPropertyName() == name)
      return i;

  return _mat_prop_descriptors.end();
}

/**
 * Perform a breadth first construction of all requested derivatives.
 */
void
DerivativeParsedMaterialHelper::assembleDerivatives()
{
  // need to check for zero derivatives here, otherwise at least one order is generated
  if (_derivative_order < 1) return;

  // if we are not on thread 0 we fetch all data from the thread 0 copy that already did all the work
  if (_tid > 0)
  {
    // get the master object from thread 0
    const MaterialWarehouse & material_warehouse = _fe_problem.getMaterialWarehouse();
    const MooseObjectWarehouse<Material> & warehouse = material_warehouse[_material_data_type];

    MooseSharedPointer<DerivativeParsedMaterialHelper> master =
      MooseSharedNamespace::dynamic_pointer_cast<DerivativeParsedMaterialHelper>(warehouse.getActiveObject(name()));

    // copy parsers and declare properties
    for (unsigned int i = 0; i < master->_derivatives.size(); ++i)
    {
      Derivative newderivative;
      newderivative.first = &declarePropertyDerivative<Real>(_F_name, master->_derivatives[i].darg_names);
      newderivative.second = ADFunctionPtr(new ADFunction(*master->_derivatives[i].second));
      _derivatives.push_back(newderivative);
    }

    // copy coupled material properties
    for (unsigned int i = 0; i < master->_mat_prop_descriptors.size(); ++i)
    {
      FunctionMaterialPropertyDescriptor newdescriptor(master->_mat_prop_descriptors[i]);
      _mat_prop_descriptors.push_back(newdescriptor);
    }

    // size parameter buffer
    _func_params.resize(master->_func_params.size());
  }

  // set up job queue. We need a deque here to be able to iterate over the currently queued items.
  std::deque<QueueItem> queue;
  queue.push_back(QueueItem(_func_F));

  // generate derivatives until the queue is exhausted
  while (!queue.empty())
  {
    QueueItem current = queue.front();

    // all permutations of one set of derivatives are equal, so we make sure to generate only one each
    unsigned int first = current._dargs.empty() ? 0 : current._dargs.back();

    // add necessary derivative steps
    for (unsigned int i = first; i < _nargs; ++i)
    {
      // go through list of material properties and check if derivatives are needed
      unsigned int ndesc = _mat_prop_descriptors.size();
      for (unsigned int jj = 0; jj < ndesc; ++jj)
      {
        FunctionMaterialPropertyDescriptor * j = &_mat_prop_descriptors[jj];

        // take a property descriptor and check if it depends on the current derivative variable
        if (j->dependsOn(_arg_names[i]))
        {
          FunctionMaterialPropertyDescriptor matderivative(*j);
          matderivative.addDerivative(_arg_names[i]);

          // search if this new derivative is not yet in the list of material properties
          MatPropDescriptorList::iterator m = findMatPropDerivative(matderivative);
          if (m == _mat_prop_descriptors.end())
          {
            // construct new variable name for the material property derivative as base name + number
            std::string newvarname = _dmatvar_base + Moose::stringify(_dmatvar_index++);
            matderivative.setSymbolName(newvarname);

            // loop over all queue items to register the new dmatvar variable (includes 'current' which is popped below)
            for (std::deque<QueueItem>::iterator k = queue.begin(); k != queue.end(); ++k)
            {
              k->_F->AddVariable(newvarname);
              k->_F->RegisterDerivative(j->getSymbolName(), _arg_names[i], newvarname);
            }

            _mat_prop_descriptors.push_back(matderivative);
          }
        }
      }

      // construct new derivative
      QueueItem newitem = current;
      newitem._dargs.push_back(i);

      // build derivative
      newitem._F = ADFunctionPtr(new ADFunction(*current._F));
      if (newitem._F->AutoDiff(_variable_names[i]) != -1)
        mooseError("Failed to take order " << newitem._dargs.size() << " derivative in material " << _name);

      // optimize and compile
      if (!_disable_fpoptimizer)
        newitem._F->Optimize();
      if (_enable_jit && !newitem._F->JITCompile())
        mooseWarning("Failed to JIT compile expression, falling back to byte code interpretation.");

      // generate material property argument vector
      std::vector<VariableName> darg_names(0);
      for (unsigned int j = 0; j < newitem._dargs.size(); ++j)
        darg_names.push_back(_arg_names[newitem._dargs[j]]);

      // append to list of derivatives if the derivative is non-vanishing
      if (!newitem._F->isZero())
      {
        Derivative newderivative;
        newderivative.first = &declarePropertyDerivative<Real>(_F_name, darg_names);
        newderivative.second = newitem._F;
        newderivative.darg_names = darg_names;
        _derivatives.push_back(newderivative);
      }

      // push item to queue if further differentiation is required
      if (newitem._dargs.size() < _derivative_order)
        queue.push_back(newitem);
    }

    // remove the 'current' element from the queue
    queue.pop_front();
  }

  // increase the parameter buffer to provide storage for the material property derivatives
  _func_params.resize(_nargs + _mat_prop_descriptors.size());
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
