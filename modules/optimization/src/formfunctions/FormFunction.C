#include "FormFunction.h"

InputParameters
FormFunction::validParams()
{
  InputParameters params = GeneralReporter::validParams();

  params.addRequiredParam<std::vector<ReporterValueName>>(
      "parameter_names", "List of parameter names, one for each group of parameters.");
  params.addRequiredParam<std::vector<dof_id_type>>(
      "num_values",
      "Number of parameter values associated with each parameter group in 'parameter_names'.");
  params.addParam<std::vector<Real>>("initial_condition",
                                     "Initial condition for each parameter values, default is 0.");
  return params;
}

FormFunction::FormFunction(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _parameter_names(getParam<std::vector<ReporterValueName>>("parameter_names")),
    _nparam(_parameter_names.size()),
    _nvalues(getParam<std::vector<dof_id_type>>("num_values")),
    _ndof(std::accumulate(_nvalues.begin(), _nvalues.end(), 0))
{
  if (_parameter_names.size() != _nvalues.size())
    paramError("num_parameters",
               "There should be a number in 'num_parameters' for each name in 'parameter_names'.");

  std::vector<Real> initial_condition = isParamValid("initial_condition")
                                            ? getParam<std::vector<Real>>("initial_condition")
                                            : std::vector<Real>(_ndof, 0.0);
  if (initial_condition.size() != _ndof)
    paramError("initial_condition",
               "Initial condition must be same length as the total number of parameter values.");

  _parameters.reserve(_nparam);
  unsigned int v = 0;
  for (unsigned int i = 0; i < _parameter_names.size(); ++i)
  {
    _parameters.push_back(
        &declareValueByName<std::vector<Real>>(_parameter_names[i], REPORTER_MODE_REPLICATED));
    _parameters[i]->assign(initial_condition.begin() + v,
                           initial_condition.begin() + v + _nvalues[i]);
    v += _nvalues[i];
  }
}

void
FormFunction::setInitialCondition(libMesh::PetscVector<Number> & x)
{
  x.init(_ndof);

  dof_id_type n = 0;
  for (const auto & param : _parameters)
    for (const auto & val : *param)
      x.set(n++, val);

  x.close();
}

void
FormFunction::updateParameters(const libMesh::PetscVector<Number> & x)
{
  dof_id_type n = 0;
  for (auto & param : _parameters)
    for (auto & val : *param)
      val = x(n++);
}

const std::vector<Real> &
FormFunction::getDataValueHelper(const std::string & get_param, const std::string & declare_param)
{
  if (!isParamValid(get_param) && !isParamValid(declare_param))
    mooseError("Must provide either ", get_param, " or ", declare_param, " in ", type());
  else if (isParamValid(get_param) && isParamValid(declare_param))
    paramError(declare_param, "Cannot specify both ", get_param, " and ", declare_param);
  else if (isParamValid(get_param))
    return getReporterValue<std::vector<Real>>(get_param, REPORTER_MODE_REPLICATED);
  else
    return declareValue<std::vector<Real>>(declare_param, REPORTER_MODE_REPLICATED);
}
