#include "ObjectiveGradientMinimize.h"
#include "PostprocessorInterface.h"
#include "VectorPostprocessorInterface.h"

registerMooseObject("isopodApp", ObjectiveGradientMinimize);

InputParameters
ObjectiveGradientMinimize::validParams()
{
  InputParameters params = FormFunction::validParams();
  params.addRequiredParam<std::vector<PostprocessorName>>(
      "data_computed",
      "List of names of postprocessors used to obtain measurement values from simulation.");
  params.addRequiredParam<std::vector<Real>>("data_target",
                                             "Target measured value for each postprocessor.");
  params.addRequiredParam<VectorPostprocessorName>(
      "adjoint_vpp",
      "Adjoint OptimizationParameterVpp used for transferring misfit parameters between simulation "
      "and optimizer.");
  params.addRequiredParam<std::vector<PostprocessorName>>(
      "adjoint_data_computed",
      "List of names of postprocessors used to obtain adjoint values from simulation.");
  return params;
}

ObjectiveGradientMinimize::ObjectiveGradientMinimize(const InputParameters & parameters)
  : FormFunction(parameters),
    _data_size(parameters.get<std::vector<PostprocessorName>>("data_computed").size()),
    _data_computed(_data_size),
    _data_target(getParam<std::vector<Real>>("data_target")),
    _data_misfit(_data_size),
    _adjoint_vpp(getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")
                     ->getUserObject<OptimizationParameterVectorPostprocessor>(
                         getParam<VectorPostprocessorName>("adjoint_vpp")))
{
  if (_data_computed.size() != _data_target.size())
    mooseError("The number of values in data_target must equal the number of postprocessors in "
               "data_computed. ");

  auto data_pp_names = parameters.get<std::vector<PostprocessorName>>("data_computed");
  for (std::size_t i = 0; i < _data_size; ++i)
    _data_computed[i] = &getPostprocessorValueByName(data_pp_names[i]);

  auto adjoint_pp_names = parameters.get<std::vector<PostprocessorName>>("adjoint_data_computed");
  for (std::size_t i = 0; i < adjoint_pp_names.size(); ++i)
    _adjoint_data_computed.push_back(&getPostprocessorValueByName(adjoint_pp_names[i]));
}

Real
ObjectiveGradientMinimize::computeObjective()
{
  for (std::size_t i = 0; i < _data_size; ++i)
    _data_misfit[i] = (*_data_computed[i]) - _data_target[i];

  _adjoint_vpp.setParameterValues(_data_misfit);

  Real val = 0;
  for (auto & misfit : _data_misfit)
    val += misfit * misfit;

  val = 0.5 * val;

  return val;
}

void
ObjectiveGradientMinimize::computeGradient()
{
  for (dof_id_type i = 0; i < _ndof; ++i)
    _gradient.set(i, *_adjoint_data_computed[i]);
  _gradient.close();
}
