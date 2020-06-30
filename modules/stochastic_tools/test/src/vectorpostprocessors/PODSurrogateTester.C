//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Stocastic Tools Includes
#include "PODSurrogateTester.h"

#include "Sampler.h"

registerMooseObject("StochasticToolsTestApp", PODSurrogateTester);

InputParameters
PODSurrogateTester::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params += SamplerInterface::validParams();
  params += SurrogateModelInterface::validParams();
  params.addClassDescription("Tool for sampling POD surrogate model.");
  params.addRequiredParam<UserObjectName>("model", "Name of surrogate model.");
  params += SamplerInterface::validParams();
  params.addRequiredParam<SamplerName>(
      "sampler", "Sampler to use for evaluating POD model (mainly for testing).");
  params.addParam<bool>("output_samples",
                        false,
                        "True to output value of samples from sampler (this may be VERY large).");
  params.addRequiredParam<std::string>(
      "variable_name",
      "The name of the variable this prostprocessor is supposed to operate on.");
  MooseEnum pptype("nodal_max=0 nodal_min=1 nodal_l1=2 nodal_l2=3 nodal_linf=4");
  params.addRequiredParam<MooseEnum>(
      "to_compute", pptype,
      "The global data the postprocessor should compute.");
  return params;
}

PODSurrogateTester::PODSurrogateTester(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    SamplerInterface(this),
    SurrogateModelInterface(this),
    _sampler(getSampler("sampler")),
    _output_samples(getParam<bool>("output_samples")),
    _value_vector(declareVector("value")),
    _variable_name(getParam<std::string>("variable_name")),
    _to_compute(getParam<MooseEnum>("to_compute"))
{
  if (_output_samples)
    for (unsigned int d = 0; d < _sampler.getNumberOfCols(); ++d)
      _sample_vector.push_back(&declareVector("sample_p" + std::to_string(d)));

  std::vector<PODReducedBasisSurrogate *> models;

  FEProblemBase& problem = *(this->parameters().get<FEProblemBase *>("_fe_problem_base"));

  UserObjectName name = getParam<UserObjectName>("model");
  problem.theWarehouse()
          .query()
          .condition<AttribName>(name)
          .queryInto(models);
  if (models.empty())
    mooseError("Unable to find a PODReducedBasisSurrogate object with the name '" + name + "'");
  _model = models[0];
}

void
PODSurrogateTester::initialSetup()
{}

void
PODSurrogateTester::initialize()
{
  _value_vector.resize(_sampler.getNumberOfLocalRows(), 0);
  if (_output_samples)
    for (unsigned int d = 0; d < _sampler.getNumberOfCols(); ++d)
      _sample_vector[d]->resize(_sampler.getNumberOfLocalRows(), 0);
}

void
PODSurrogateTester::execute()
{
  // Loop over samples
  for (dof_id_type p = _sampler.getLocalRowBegin(); p < _sampler.getLocalRowEnd(); ++p)
  {
    std::vector<Real> data = _sampler.getNextLocalRow();

    _model->evaluateSolution(data);
    switch(_to_compute)
    {
      case 0 :
        _value_vector[p - _sampler.getLocalRowBegin()] = _model->getNodalMax(_variable_name);
        break;

      case 1 :
        _value_vector[p - _sampler.getLocalRowBegin()] = _model->getNodalMin(_variable_name);
        break;

      case 2 :
        _value_vector[p - _sampler.getLocalRowBegin()] = _model->getNodalL1(_variable_name);
        break;

      case 3 :
        _value_vector[p - _sampler.getLocalRowBegin()] = _model->getNodalL2(_variable_name);
        break;

      case 4 :
        _value_vector[p - _sampler.getLocalRowBegin()] = _model->getNodalLinf(_variable_name);
        break;
    }

    if (_output_samples)
      for (unsigned int d = 0; d < _sampler.getNumberOfCols(); ++d)
      {
        (*_sample_vector[d])[p - _sampler.getLocalRowBegin()] = data[d];
      }
  }
}

void
PODSurrogateTester::finalize()
{
  _communicator.gather(0, _value_vector);
  if (_output_samples)
    for (auto & ppv_ptr : _sample_vector)
      _communicator.gather(0, *ppv_ptr);
}
