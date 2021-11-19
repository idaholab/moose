#include "OptimizationReporter.h"

InputParameters
OptimizationReporter::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription("Base class for reporter holding optimization information.");
  params.addRequiredParam<std::vector<ReporterValueName>>(
      "parameter_names", "List of parameter names, one for each group of parameters.");
  params.addRequiredParam<std::vector<dof_id_type>>(
      "num_values",
      "Number of parameter values associated with each parameter group in 'parameter_names'.");
  params.addParam<std::vector<Real>>("initial_condition",
                                     "Initial condition for each parameter values, default is 0.");
  params.addParam<std::vector<Real>>(
      "lower_bounds", std::vector<Real>(), "Lower bounds for each parameter value.");
  params.addParam<std::vector<Real>>(
      "upper_bounds", std::vector<Real>(), "Upper bounds for each parameter value.");
  // make measurement_values & measurement_points addRequiredParam  but will require all tests to
  // change
  params.addParam<std::vector<Real>>(
      "measurement_values",
      "Measurement values collected from locations given by measurement_points");
  params.addParam<std::vector<Point>>("measurement_points",
                                      "Point locations corresponding to each measurement value");

  return params;
}

OptimizationReporter::OptimizationReporter(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _parameter_names(getParam<std::vector<ReporterValueName>>("parameter_names")),
    _nparam(_parameter_names.size()),
    _nvalues(getParam<std::vector<dof_id_type>>("num_values")),
    _ndof(std::accumulate(_nvalues.begin(), _nvalues.end(), 0)),
    _lower_bounds(getParam<std::vector<Real>>("lower_bounds")),
    _upper_bounds(getParam<std::vector<Real>>("upper_bounds")),
    _measurement_values(getParam<std::vector<Real>>("measurement_values")),
    _measurement_points(getParam<std::vector<Point>>("measurement_points")),

    _measurement_data(declareValueByName<std::vector<std::tuple<Point, Real, Real, Real>>>(
        "measurement_data", REPORTER_MODE_REPLICATED)),
    _misfit(declareValueByName<std::vector<Real>>("misfit", REPORTER_MODE_REPLICATED))

{
  for (size_t i = 0; i < _measurement_points.size(); ++i)
    _measurement_data.push_back(std::make_tuple(_measurement_points[i],
                                                _measurement_values[i],
                                                std::numeric_limits<Real>::max(),
                                                std::numeric_limits<Real>::max()));

  if (_parameter_names.size() != _nvalues.size())
    paramError("num_parameters",
               "There should be a number in 'num_parameters' for each name in 'parameter_names'.");

  std::vector<Real> initial_condition = isParamValid("initial_condition")
                                            ? getParam<std::vector<Real>>("initial_condition")
                                            : std::vector<Real>(_ndof, 0.0);
  if (initial_condition.size() != _ndof)
    paramError("initial_condition",
               "Initial condition must be same length as the total number of parameter values.");

  if (_upper_bounds.size() > 0 && _upper_bounds.size() != _ndof)
    paramError("upper_bounds", "Upper bound data is not equal to the total number of parameters.");
  else if (_lower_bounds.size() > 0 && _lower_bounds.size() != _ndof)
    paramError("lower_bounds", "Lower bound data is not equal to the total number of parameters.");
  else if (_lower_bounds.size() != _upper_bounds.size())
    paramError((_lower_bounds.size() == 0 ? "upper_bounds" : "lower_bounds"),
               "Both upper and lower bounds must be specified if bounds are used");

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
OptimizationReporter::setInitialCondition(libMesh::PetscVector<Number> & x)
{
  x.init(_ndof);

  dof_id_type n = 0;
  for (const auto & param : _parameters)
    for (const auto & val : *param)
      x.set(n++, val);

  x.close();
}

void
OptimizationReporter::updateParameters(const libMesh::PetscVector<Number> & x)
{
  dof_id_type n = 0;
  for (auto & param : _parameters)
    for (auto & val : *param)
      val = x(n++);
}

std::vector<Real>
OptimizationReporter::computeDefaultBounds(Real val)
{
  std::vector<Real> vec;
  vec.resize(_nparam);
  for (auto i : index_range(vec))
    vec[i] = val;
  return vec;
}

Real
OptimizationReporter::computeAndCheckObjective(bool multiapp_passed)
{
  if (!multiapp_passed)
    mooseError("Forward solve multiapp failed!");
  return computeObjective();
}

Real
OptimizationReporter::computeObjective()
{
  Real val = 0;
  for (auto & misfit : _misfit)
    val += misfit * misfit;

  val = 0.5 * val;

  return val;
}

namespace libMesh
{
void
to_json(nlohmann::json & json, const std::vector<std::tuple<Point, Real, Real, Real>> & value)
{
  Point measurement_point;
  Real measurement_value;
  Real simualtion_value;
  Real misfit_value;

  for (auto & v : value)
  {
    std::stringstream ss;
    std::tie(measurement_point, measurement_value, simualtion_value, misfit_value) = v;
    ss << "(";
    for (const auto & i : make_range(LIBMESH_DIM))
    {
      ss << measurement_point(i);
      if (i < (LIBMESH_DIM - 1))
        ss << ", ";
    }
    ss << ")";
    json["measurement_point"].push_back(ss.str());
    json["measurement_value"].push_back(measurement_value);
    json["simualtion_value"].push_back(simualtion_value);
    json["misfit_value"].push_back(misfit_value);
  }
}
}
//
template <>
void
dataStore(std::ostream & stream, std::tuple<Point, Real, Real, Real> & v, void * context)
{
  Point measurement_point;
  Real measurement_value;
  Real simualtion_value;
  Real misfit_value;
  std::tie(measurement_point, measurement_value, simualtion_value, misfit_value) = v;
  dataStore(stream, measurement_point, context);
  dataStore(stream, measurement_value, context);
  dataStore(stream, simualtion_value, context);
  dataStore(stream, misfit_value, context);
};

template <>
void
dataLoad(std::istream & stream, std::tuple<Point, Real, Real, Real> & v, void * context)
{
  Point measurement_point;
  Real measurement_value;
  Real simualtion_value;
  Real misfit_value;
  std::tie(measurement_point, measurement_value, simualtion_value, misfit_value) = v;
  dataLoad(stream, measurement_point, context);
  dataLoad(stream, measurement_value, context);
  dataLoad(stream, simualtion_value, context);
  dataLoad(stream, misfit_value, context);
};
