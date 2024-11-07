//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GenericActiveLearner.h"

registerMooseObject("StochasticToolsApp", GenericActiveLearner);

InputParameters
GenericActiveLearner::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params += ParallelAcquisitionInterface::validParams();
  params.addClassDescription("A generic reporter to support parallel active learning: re-trains GP "
                             "and picks the next best batch.");
  params.addRequiredParam<ReporterName>("output_value",
                                        "Value of the model output from the SubApp.");
  params.addParam<ReporterValueName>(
      "outputs_required",
      "outputs_required",
      "Modified value of the model output from this reporter class.");
  params.addRequiredParam<SamplerName>("sampler", "The sampler object.");
  params.addRequiredParam<UserObjectName>("al_gp", "Active learning GP trainer.");
  params.addRequiredParam<UserObjectName>("gp_evaluator", "Evaluator for the trained GP.");
  params.addParam<ReporterValueName>(
      "sorted_indices",
      "sorted_indices",
      "The sorted sample indices in order of importance to evaluate the subApp.");
  params.addParam<ReporterValueName>(
      "acquisition_function",
      "acquisition_function",
      "The values of the acquistion function in the current iteration.");
  params.addParam<ReporterValueName>("convergence_value",
                                     "convergence_value",
                                     "Value to measure convergence of active learning.");
  params.addParam<ReporterValueName>(
      "inputs", "inputs", "Modified value of the model inputs from this reporter class.");
  params.addRequiredParam<UserObjectName>("acquisition", "Name of the acquisition function.");
  params.addParam<bool>("penalize_acquisition", true,
                        "Set true to prevent clustering of the best batch inputs when operating in parallel.");
  return params;
}

GenericActiveLearner::GenericActiveLearner(const InputParameters & parameters)
  : GeneralReporter(parameters),
    ParallelAcquisitionInterface(parameters),
    SurrogateModelInterface(this),
    _output_value(getReporterValue<std::vector<Real>>("output_value", REPORTER_MODE_DISTRIBUTED)),
    _output_comm(declareValue<std::vector<Real>>("outputs_required")),
    _sampler(getSampler("sampler")),
    _al_sampler(dynamic_cast<const GenericActiveLearningSampler *>(&_sampler)),
    _sorted_indices(declareValue<std::vector<unsigned int>>(
        "sorted_indices", std::vector<unsigned int>(_al_sampler->getNumParallelProposals(), 0))),
    _inputs_test(_al_sampler->getSampleTries()),
    _al_gp(getUserObject<ActiveLearningGaussianProcess>("al_gp")),
    _gp_eval(getSurrogateModel<GaussianProcessSurrogate>("gp_evaluator")),
    _acquisition_obj(getParallelAcquisitionFunctionByName(getParam<UserObjectName>("acquisition"))),
    _acquisition_value(declareValue<std::vector<Real>>("acquisition_function")),
    _convergence_value(declareValue<Real>("convergence_value")),
    _inputs_required(declareValue<std::vector<std::vector<Real>>>("inputs")),
    _penalize_acquisition(getParam<bool>("penalize_acquisition")),
    _check_step(std::numeric_limits<int>::max()),
    _local_comm(_sampler.getLocalComm())
{
  // Check whether the selected sampler is the right type
  if (!_al_sampler)
    paramError("sampler", "The selected sampler is not of type GenericActiveLearningSampler.");

  // Fetching the sampler characteristics
  _n_dim = _sampler.getNumberOfCols();
  _props = _al_sampler->getNumParallelProposals();

  // Setting up the variable sizes to facilitate active learning
  _gp_outputs_test.resize(_inputs_test.size());
  _gp_std_test.resize(_inputs_test.size());
  _acquisition_value.resize(_props);
  _length_scales.resize(_n_dim);
  _eval_outputs_current.resize(_props);
  _generic.resize(1);
  _inputs_required.resize(_props, std::vector<Real>(_n_dim, 0.0));
}

void
GenericActiveLearner::setupGPData(const std::vector<Real> & data_out,
                                   const DenseMatrix<Real> & data_in)
{
  std::vector<Real> tmp;
  tmp.resize(_n_dim);
  for (unsigned int i = 0; i < data_out.size(); ++i)
  {
    for (unsigned int j = 0; j < _n_dim; ++j)
      tmp[j] = data_in(i, j);
    _inputs_required[i] = tmp;
    _gp_inputs.push_back(tmp);
    _gp_outputs.push_back(data_out[i]);
  }
}

void
GenericActiveLearner::computeGPOutput(std::vector<Real> & eval_outputs)
{
  for (unsigned int i = 0; i < eval_outputs.size(); ++i)
    eval_outputs[i] = _gp_eval.evaluate(_gp_inputs[i]);
}

void
GenericActiveLearner::convertToEigen(const std::vector<Real> & vec, RealEigenMatrix & mat)
{
  for (unsigned int i = 0; i < vec.size(); ++i)
    mat(i, 0) = vec[i];
}

void
GenericActiveLearner::convertToVector(const RealEigenMatrix & mat, std::vector<Real> & vec)
{
  for (unsigned int i = 0; i < vec.size(); ++i)
    vec[i] = mat(i, 0);
}

void
GenericActiveLearner::setupGeneric()
{
  _generic.resize(1);
}

void
GenericActiveLearner::includeAdditionalInputs()
{
  _inputs_test_modified = _inputs_test;
}

void
GenericActiveLearner::getAcquisition(std::vector<Real> & acq_new,
                                     std::vector<unsigned int> & indices)
{
  std::vector<Real> acq;
  acq.resize(_inputs_test.size());
  includeAdditionalInputs();
  _acquisition_obj->computeAcquisition(
      acq, _gp_outputs_test, _gp_std_test, _inputs_test_modified, _gp_inputs, _generic);
  acq_new = acq;
  if (_penalize_acquisition)
    _acquisition_obj->penalizeAcquisition(
        acq_new, indices, acq, _length_scales, _inputs_test_modified);
}

void
GenericActiveLearner::computeConvergenceValue()
{
  for (unsigned int ii = 0; ii < _output_comm.size(); ++ii)
    _convergence_value += Utility::pow<2>(_output_comm[ii] - _eval_outputs_current[ii]);
  _convergence_value = std::sqrt(_convergence_value) / _output_comm.size();
}

void
GenericActiveLearner::evaluateGPTest()
{
  std::vector<Real> tmp;
  tmp.resize(_n_dim);
  for (unsigned int i = 0; i < _gp_outputs_test.size(); ++i)
  {
    for (unsigned int j = 0; j < _n_dim; ++j)
      tmp[j] = _inputs_test[i][j];
    _gp_outputs_test[i] = _gp_eval.evaluate(tmp, _gp_std_test[i]);
  }

  // Converting the GP mean and std to the standardized domain
  RealEigenMatrix eval_outputs_mat(_gp_outputs_test.size(), 1);
  RealEigenMatrix eval_std_mat(_gp_outputs_test.size(), 1);
  convertToEigen(_gp_outputs_test, eval_outputs_mat);
  convertToEigen(_gp_std_test, eval_std_mat);
  StochasticTools::Standardizer standardizer;
  _al_gp.getTrainingStandardizer(standardizer);
  standardizer.getStandardized(eval_outputs_mat);
  standardizer.getScaled(eval_std_mat);
  convertToVector(eval_outputs_mat, _gp_outputs_test);
  convertToVector(eval_std_mat, _gp_std_test);
}

void
GenericActiveLearner::execute()
{
  if (_sampler.getNumberOfLocalRows() == 0 || _check_step == _t_step)
  {
    _check_step = _t_step;
    return;
  }

  DenseMatrix<Real> data_in(_sampler.getNumberOfRows(), _sampler.getNumberOfCols());
  for (dof_id_type ss = _sampler.getLocalRowBegin(); ss < _sampler.getLocalRowEnd(); ++ss)
  {
    const auto data = _sampler.getNextLocalRow();
    for (unsigned int j = 0; j < _sampler.getNumberOfCols(); ++j)
      data_in(ss, j) = data[j];
  }
  _local_comm.sum(data_in.get_values());
  _output_comm = _output_value;
  _local_comm.allgather(_output_comm);

  if (_t_step > 1)
  {
    // Setup the GP training data
    setupGPData(_output_comm, data_in);

    // Compute the convergence value before re-training the GP
    _convergence_value = 0.0;
    if (_t_step > 2)
    {
      computeGPOutput(_eval_outputs_current);
      computeConvergenceValue();
    }

    // Retrain the GP and get the length scales
    _al_gp.reTrain(_gp_inputs, _gp_outputs);
    _al_gp.getLengthScales(_length_scales);

    // Evaluate the GP on all the test samples sent by the Sampler
    evaluateGPTest();

    // Setup the generic variable for acquisition computation (depends on the objective:
    // optimization, UQ, etc.)
    setupGeneric();

    // Get the acquisition function values and ordering of indices as per the acquisition
    std::vector<Real> acq_new;
    std::vector<unsigned int> indices;
    indices.resize(_inputs_test.size());
    getAcquisition(acq_new, indices);

    // Output the acquisition function values and the best ordering of the indices
    for (unsigned int i = 0; i < _props; ++i)
    {
      _sorted_indices[i] = indices[i];
      _acquisition_value[i] = acq_new[i];
    }
  }
  else
    for (unsigned int i = 0; i < _props; ++i)
      _sorted_indices[i] = i;

  // Track the current step
  _check_step = _t_step;
}
