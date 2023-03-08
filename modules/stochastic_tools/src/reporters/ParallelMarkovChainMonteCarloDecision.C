// //* This file is part of the MOOSE framework
// //* https://www.mooseframework.org
// //*
// //* All rights reserved, see COPYRIGHT for full restrictions
// //* https://github.com/idaholab/moose/blob/master/COPYRIGHT
// //*
// //* Licensed under LGPL 2.1, please see LICENSE for details
// //* https://www.gnu.org/licenses/lgpl-2.1.html

// #include "ParallelMarkovChainMonteCarloDecision.h"
// #include "Sampler.h"
// #include "DenseMatrix.h"
// #include "AdaptiveMonteCarloUtils.h"
// #include "StochasticToolsUtils.h"
// #include "MooseRandom.h"
// #include "Likelihood.h"
// // #include "Uniform.h"

// registerMooseObjectAliased("StochasticToolsApp", ParallelMarkovChainMonteCarloDecision,
// "PMCMCDecision");

// InputParameters
// ParallelMarkovChainMonteCarloDecision::validParams()
// {
//   InputParameters params = GeneralReporter::validParams();
//   params.addClassDescription("Generic reporter which decides whether or not to accept a proposed
//   "
//                              "sample in parallel Markov chain Monte Carlo type of algorithms.");
//   params.addParam<ReporterValueName>("seed_inputs", "seed_inputs",
//                                         "Seed input values for proposing the next set of
//                                         samples.");
//   params.addRequiredParam<ReporterName>("output_value",
//                                         "Value of the model output from the SubApp.");
//   params.addParam<ReporterValueName>("inputs", "inputs", "Uncertain inputs to the model.");
//   params.addParam<ReporterValueName>("outputs_req", "outputs_req", "Outputs from the model.");
//   params.addParam<ReporterValueName>("tpm", "tpm", "The transition probability matrix.");
//   params.addParam<ReporterValueName>("proposal_std", "proposal_std", "Vector of standard
//   deviations to aid proposing new sample."); params.addRequiredParam<SamplerName>("sampler", "The
//   sampler object."); params.addRequiredParam<std::vector<LikelihoodName>>("likelihoods", "Names
//   of the likelihoods.");
//   params.addRequiredParam<std::vector<DistributionName>>("prior_distributions", "The prior
//   distributions of the parameters to be calibrated."); return params;
// }

// ParallelMarkovChainMonteCarloDecision::ParallelMarkovChainMonteCarloDecision(const
// InputParameters & parameters)
//   : GeneralReporter(parameters),
//     LikelihoodInterface(this),
//     // DistributionInterface(this),
//     _seed_inputs(declareValue<std::vector<Real>>("seed_inputs")),
//     _output_value(getReporterValue<std::vector<Real>>("output_value",
//     REPORTER_MODE_DISTRIBUTED)), _inputs(declareValue<std::vector<std::vector<Real>>>("inputs")),
//     _outputs(declareValue<std::vector<Real>>("outputs_req")),
//     _tpm(declareValue<std::vector<Real>>("tpm")),
//     _proposal_std(declareValue<std::vector<Real>>("proposal_std")),
//     _step(getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")->timeStep()),
//     _sampler(getSampler("sampler")),
//     _pmcmc(dynamic_cast<const ParallelMarkovChainMonteCarloBase *>(&_sampler)),
//     _check_step(std::numeric_limits<int>::max())
// {

//   for (const LikelihoodName & name : getParam<std::vector<LikelihoodName>>("likelihoods"))
//     _likelihoods.push_back(&getLikelihoodByName(name));

//   // Filling the `priors` vector with the user-provided distributions.
//   for (const DistributionName & name :
//   getParam<std::vector<DistributionName>>("prior_distributions"))
//     _priors.push_back(&getDistributionByName(name));

//   // Check whether the selected sampler is an MCMC sampler or not
//   if (!_pmcmc)
//     paramError("sampler", "The selected sampler is not an MCMC sampler.");

//   const auto props = _pmcmc->getNumParallelProposals();
//   const auto cols = _sampler.getNumberOfCols();

//   // Create communicator that only has processors with rows
//   _communicator.split(
//       _sampler.getNumberOfLocalRows() > 0 ? 1 : MPI_UNDEFINED, processor_id(), _local_comm);

//   _inputs.resize(props);
//   for (unsigned int i = 0; i < props; ++i)
//     _inputs[i].resize(cols);

//   _outputs.resize(_sampler.getNumberOfRows());
//   _tpm.resize(_pmcmc->getNumParallelProposals());

//   _seed_inputs.resize(cols);

//   _proposal_std.resize(cols);

//   _output_prev.resize(_pmcmc->getNumParallelProposals() * _pmcmc->getNumberOfConfigParams());

//   // _output_value.resize(rows);
//   _inputs_sto.resize(cols);
// }

// void
// ParallelMarkovChainMonteCarloDecision::computeTransitionVector(std::vector<Real> & tv,
// std::vector<const Distribution *> priors, std::vector<const Likelihood *> likelihoods, const
// DenseMatrix<Real> & inputs, const std::vector<Real> & outputs, const dof_id_type & num_confg)
// {
//   Real sum1 = 0.0;
//   Real quant1;
//   std::vector<Real> out1(num_confg);
//   std::vector<Real> out2(num_confg);
//   for (unsigned int j = 0; j < num_confg; ++j)
//     out2[j] = outputs[j + (tv.size() - 1) * num_confg];
//   dof_id_type count1 = 0;
//   for (unsigned int i = 0; i < tv.size()-1; ++i)
//   {
//     quant1 = 0.0;
//     for (unsigned int j = 0; j < priors.size(); ++j)
//         quant1 += (std::log(priors[j]->pdf(inputs(count1, j))) -
//         std::log(priors[j]->pdf(inputs(tv.size() * num_confg - 1, j))));
//     for (unsigned int j = 0; j < num_confg; ++j)
//         out1[j] = outputs[j + count1];
//     for (unsigned int j = 0; j < likelihoods.size(); ++j)
//         quant1 += (likelihoods[j]->function(out1) - likelihoods[j]->function(out2));
//     quant1 = (1.0 / (tv.size() - 1)) * std::exp(std::min(quant1, 0.0));
//     tv[i] = quant1;
//     sum1 += quant1;
//     count1 += num_confg;
//   }
//   tv[tv.size() - 1] = 1 - sum1;
// }

// void
// ParallelMarkovChainMonteCarloDecision::resample(const DenseMatrix<Real> & given_inputs, const
// std::vector<Real> & weights, std::vector<Real> & req_inputs, const dof_id_type & num_confg)
// {
//   Real rnd = MooseRandom::rand();
//   dof_id_type req_index = AdaptiveMonteCarloUtils::weightedResample(weights, rnd);
//   for (unsigned int i = 0; i < req_inputs.size(); ++i)
//     req_inputs[i] = given_inputs(req_index * num_confg, i);
// }

// void
// ParallelMarkovChainMonteCarloDecision::execute()
// {
//     DenseMatrix<Real> data_in(_sampler.getNumberOfRows(), _sampler.getNumberOfCols());
//     for (dof_id_type ss = _sampler.getLocalRowBegin(); ss < _sampler.getLocalRowEnd(); ++ss)
//     {
//     const auto data = _sampler.getNextLocalRow();
//     for (unsigned int j = 0; j < _sampler.getNumberOfCols(); ++j)
//         data_in(ss, j) = data[j];
//     }
//     _local_comm.sum(data_in.get_values());
//     _output_comm = _output_value;
//     _local_comm.allgather(_output_comm);
//     _outputs = _output_comm;

//     if (_step > 1)
//     {
//       // std::cout << "Outputs " << Moose::stringify(_output_comm) << std::endl;
//       // std::vector<Real> tmp1;
//       // tmp1.resize(_sampler.getNumberOfCols());
//       // for (unsigned int i = 0; i < _sampler.getNumberOfRows(); ++i)
//       // {
//       //   for (unsigned int j = 0; j < _sampler.getNumberOfCols(); ++j)
//       //     tmp1[j] = data_in(i, j);
//       //   std::cout << "Inputs row " << i << ": " << Moose::stringify(tmp1) << std::endl;
//       // }

//       std::vector<Real> tpm;
//       tpm.resize(_pmcmc->getNumParallelProposals() + 1);
//       computeTransitionVector(tpm, _priors, _likelihoods, data_in, _output_comm,
//       _pmcmc->getNumberOfConfigParams()); // _data_prev std::cout << "TPM " <<
//       Moose::stringify(tpm) << std::endl; _tpm = tpm; std::vector<Real>
//       req_inputs(_sampler.getNumberOfCols()); for (unsigned int i = 0; i <
//       _pmcmc->getNumParallelProposals(); ++i)
//       {
//         resample(data_in, tpm, req_inputs, _pmcmc->getNumberOfConfigParams()); // _data_prev
//         _inputs[i] = req_inputs;
//         for (unsigned int j = 0; j < _sampler.getNumberOfCols(); ++j)
//           _inputs_sto[j].push_back(req_inputs[j]);
//       }
//       resample(data_in, tpm, _seed_inputs, _pmcmc->getNumberOfConfigParams()); // _data_prev
//       for (unsigned int i = 0; i < _sampler.getNumberOfCols(); ++i)
//         _proposal_std[i] = AdaptiveMonteCarloUtils::computeSTD(_inputs_sto[i], 0);
//       std::cout << "Proposal std " << Moose::stringify(_proposal_std) << std::endl;
//     }
//     _data_prev = data_in;
// }

// // Affine invariant

// // void
// // ParallelMarkovChainMonteCarloDecision::computeTransitionVector(std::vector<Real> & tv, std::vector<const Distribution *> priors, std::vector<const Likelihood *> likelihoods, const DenseMatrix<Real> & inputs, const std::vector<Real> & outputs, const dof_id_type & num_confg, const DenseMatrix<Real> & prev_inputs, const std::vector<Real> & prev_outputs)
// // {
// //   Real quant1;
// //   std::vector<Real> out1(num_confg);
// //   std::vector<Real> out2(num_confg);
// //   std::vector<Real> z = _pmcmc->getAffineStepSize();
// //   // for (unsigned int j = 0; j < num_confg; ++j)
// //   //   out2[j] = outputs[j + (tv.size() - 1) * num_confg];
// //   dof_id_type count1 = 0;
// //   for (unsigned int i = 0; i < tv.size(); ++i)
// //   {
// //     quant1 = 0.0;
// //     for (unsigned int j = 0; j < priors.size(); ++j)
// //       quant1 += (std::log(priors[j]->pdf(inputs(count1, j))) - std::log(priors[j]->pdf(prev_inputs(count1, j))));
// //     for (unsigned int j = 0; j < num_confg; ++j)
// //     {
// //       out1[j] = outputs[j + count1];
// //       out2[j] = prev_outputs[j + count1];
// //     }
// //     for (unsigned int j = 0; j < likelihoods.size(); ++j)
// //       quant1 += (likelihoods[j]->function(out1) - likelihoods[j]->function(out2));
// //     quant1 = std::exp(std::min((priors.size()-1) * std::log(z[count1]) + quant1, 0.0)); // std::min(std::pow(z[count1], priors.size()-1) * std::exp(quant1), 1.0); //
// //     tv[i] = quant1;
// //     count1 += num_confg;
// //   }
// // }

// // void
// // ParallelMarkovChainMonteCarloDecision::resample(const DenseMatrix<Real> & given_inputs, const std::vector<Real> & weights, std::vector<Real> & req_inputs, const dof_id_type & num_confg)
// // {
// //   Real rnd = MooseRandom::rand();
// //   dof_id_type req_index = AdaptiveMonteCarloUtils::weightedResample(weights, rnd);
// //   for (unsigned int i = 0; i < req_inputs.size(); ++i)
// //     req_inputs[i] = given_inputs(req_index * num_confg, i);
// // }

// // void
// // ParallelMarkovChainMonteCarloDecision::execute()
// // {
// //     DenseMatrix<Real> data_in(_sampler.getNumberOfRows(), _sampler.getNumberOfCols());
// //     for (dof_id_type ss = _sampler.getLocalRowBegin(); ss < _sampler.getLocalRowEnd(); ++ss)
// //     {
// //       const auto data = _sampler.getNextLocalRow();
// //       for (unsigned int j = 0; j < _sampler.getNumberOfCols(); ++j)
// //           data_in(ss, j) = data[j];
// //     }
// //     _local_comm.sum(data_in.get_values());
// //     _output_comm = _output_value;
// //     _local_comm.allgather(_output_comm);
// //     _outputs = _output_comm;

// //     if (_step > 1)
// //     {
// //       // std::cout << "Outputs " << Moose::stringify(_output_comm) << std::endl;
// //       // std::vector<Real> tmp1;
// //       // tmp1.resize(_sampler.getNumberOfCols());
// //       // for (unsigned int i = 0; i < _sampler.getNumberOfRows(); ++i)
// //       // {
// //       //   for (unsigned int j = 0; j < _sampler.getNumberOfCols(); ++j)
// //       //     tmp1[j] = data_in(i, j);
// //       //   std::cout << "Inputs row " << i << ": " << Moose::stringify(tmp1) << std::endl;
// //       // }

// //       std::vector<Real> tpm;
// //       tpm.resize(_pmcmc->getNumParallelProposals());
// //       computeTransitionVector(tpm, _priors, _likelihoods, data_in, _output_comm, _pmcmc->getNumberOfConfigParams(), _data_prev, _output_prev); // _data_prev
// //       std::cout << "TPM " << Moose::stringify(tpm) << std::endl;
// //       _tpm = tpm;
// //       dof_id_type count1 = 0;
// //       // std::vector<Real> req_inputs(_sampler.getNumberOfCols());
// //       for (unsigned int i = 0; i < _pmcmc->getNumParallelProposals(); ++i)
// //       {
// //         if (tpm[i] >= MooseRandom::rand())
// //         {
// //           for (unsigned int j = 0; j < _sampler.getNumberOfCols(); ++j)
// //             _inputs[i][j] = data_in(count1, j);
// //           std::cout << "Accept!" << std::endl;
// //         }
// //         else
// //         {
// //           for (unsigned int j = 0; j < _sampler.getNumberOfCols(); ++j)
// //           {
// //             _inputs[i][j] = _data_prev(count1, j);
// //             for (unsigned int k = count1; k < (_pmcmc->getNumberOfConfigParams()+count1); ++k)
// //               data_in(k, j) = _data_prev(k, j);
// //           }
// //           std::cout << "Reject!" << std::endl;
// //         }
// //         // resample(data_in, tpm, req_inputs, _pmcmc->getNumberOfConfigParams()); // _data_prev
// //         // _inputs[i] = req_inputs;
// //         // for (unsigned int j = 0; j < _sampler.getNumberOfCols(); ++j)
// //         //   _inputs_sto[j].push_back(req_inputs[j]);
// //         count1 += _pmcmc->getNumberOfConfigParams();
// //       }
// //       // resample(data_in, tpm, _seed_inputs, _pmcmc->getNumberOfConfigParams()); // _data_prev
// //       // for (unsigned int i = 0; i < _sampler.getNumberOfCols(); ++i)
// //       //   _proposal_std[i] = AdaptiveMonteCarloUtils::computeSTD(_inputs_sto[i], 0);
// //       // std::cout << "Proposal std " << Moose::stringify(_proposal_std) << std::endl;
// //     }
// //     else
// //     {
// //       for (unsigned int i = 0; i < _pmcmc->getNumParallelProposals(); ++i)
// //       {
// //         for (unsigned int j = 0; j < _sampler.getNumberOfCols(); ++j)
// //         {
// //           _inputs[i][j] = 0.05;
// //           data_in(i, j) = 0.05;
// //         }

// //       }
// //     }
// //     for (unsigned int i = 0; i < _pmcmc->getNumParallelProposals(); ++i)
// //       std::cout << "Accpeted samples: " << Moose::stringify(_inputs[i]) << std::endl; //
// //     _data_prev = data_in;
// //     _output_prev = _outputs;
// // }

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParallelMarkovChainMonteCarloDecision.h"
#include "Sampler.h"
#include "DenseMatrix.h"
// #include "AdaptiveMonteCarloUtils.h"
// #include "StochasticToolsUtils.h"
// #include "MooseRandom.h"

registerMooseObjectAliased("StochasticToolsApp", ParallelMarkovChainMonteCarloDecision, "PMCMCDecision");

InputParameters
ParallelMarkovChainMonteCarloDecision::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription("Generic reporter which decides whether or not to accept a proposed "
                             "sample in parallel Markov chain Monte Carlo type of algorithms.");
  params.addRequiredParam<ReporterName>("output_value",
                                        "Value of the model output from the SubApp.");
  params.addParam<ReporterValueName>(
      "outputs_required",
      "outputs_required",
      "Modified value of the model output from this reporter class.");
  params.addParam<ReporterValueName>("inputs", "inputs", "Uncertain inputs to the model.");
  params.addParam<ReporterValueName>("tpm", "tpm", "The transition probability matrix.");
  params.addRequiredParam<SamplerName>("sampler", "The sampler object.");
  params.addRequiredParam<std::vector<LikelihoodName>>("likelihoods", "Names of the likelihoods.");
  params.addRequiredParam<std::vector<DistributionName>>("prior_distributions", "The prior distributions of the parameters to be calibrated.");
  return params;
}

ParallelMarkovChainMonteCarloDecision::ParallelMarkovChainMonteCarloDecision(
    const InputParameters & parameters)
  : GeneralReporter(parameters),
    LikelihoodInterface(this),
    _output_value(getReporterValue<std::vector<Real>>("output_value", REPORTER_MODE_DISTRIBUTED)),
    _outputs_required(declareValue<std::vector<Real>>("outputs_required")),
    _inputs(declareValue<std::vector<std::vector<Real>>>("inputs")),
    _tpm(declareValue<std::vector<Real>>("tpm")),
    _sampler(getSampler("sampler")),
    _pmcmc(dynamic_cast<const ParallelMarkovChainMonteCarloBase *>(&_sampler)),
    _rnd_vec(_pmcmc->getRandomNumbers()),
    _step(getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")->timeStep()),
    _check_step(std::numeric_limits<int>::max())
{
  // Filling the `likelihoods` vector with the user-provided distributions.
  for (const LikelihoodName & name : getParam<std::vector<LikelihoodName>>("likelihoods"))
    _likelihoods.push_back(&getLikelihoodByName(name));

  // Filling the `priors` vector with the user-provided distributions.
  for (const DistributionName & name : getParam<std::vector<DistributionName>>("prior_distributions"))
    _priors.push_back(&getDistributionByName(name));

  // Check whether the selected sampler is an MCMC sampler or not
  if (!_pmcmc)
    paramError("sampler", "The selected sampler is not of type MCMC.");

  // Create communicator that only has processors with rows
  _communicator.split(
      _sampler.getNumberOfLocalRows() > 0 ? 1 : MPI_UNDEFINED, processor_id(), _local_comm);

  // Fetching the sampler characteristics
  _props = _pmcmc->getNumParallelProposals();
  _num_confg = _pmcmc->getNumberOfConfigParams();

  // Resizing the data arrays to transmit to the output file
  _inputs.resize(_props);
  for (unsigned int i = 0; i < _props; ++i)
    _inputs[i].resize(_sampler.getNumberOfCols() - 1);
  _outputs_required.resize(_sampler.getNumberOfRows());
  _tpm.resize(_props);
}

void
ParallelMarkovChainMonteCarloDecision::computeTransitionVector(
    std::vector<Real> & tv, DenseMatrix<Real> & /*inputs_matrix*/)
{
  tv.assign(_props, 1.0);
}

void
ParallelMarkovChainMonteCarloDecision::nextSamples(std::vector<Real> & req_inputs,
                                                   DenseMatrix<Real> & inputs_matrix,
                                                   const std::vector<Real> & tv,
                                                   const unsigned int & parallel_index)
{
  if (tv[parallel_index] >= _rnd_vec[parallel_index])
  {
    for (unsigned int k = 0; k < _sampler.getNumberOfCols() - 1; ++k)
      req_inputs[k] = inputs_matrix(parallel_index, k);
  }
  else
  {
    for (unsigned int k = 0; k < _sampler.getNumberOfCols() - 1; ++k)
    {
      req_inputs[k] = _data_prev(parallel_index, k);
      inputs_matrix(parallel_index, k) = _data_prev(parallel_index, k);
    }
    for (unsigned int k = 0; k < _num_confg; ++k)
      _outputs_required[k * _pmcmc->getNumParallelProposals() + parallel_index] =
          _outputs_prev[k * _pmcmc->getNumParallelProposals() + parallel_index];
  }
}

void
ParallelMarkovChainMonteCarloDecision::execute()
{
  if (_sampler.getNumberOfLocalRows() == 0 || _check_step == _step)
  {
    _check_step = _step;
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
  _outputs_required = _output_value;
  _local_comm.allgather(_outputs_required);
  // _local_comm.gather(0, _outputs_required);

  // if (_step > _pmcmc->decisionStep())
  // {
  //   computeTransitionVector(_tpm, data_in);
  //   std::vector<Real> req_inputs(_sampler.getNumberOfCols() - 1);
  //   for (unsigned int i = 0; i < _pmcmc->getNumParallelProposals(); ++i)
  //   {
  //     nextSamples(req_inputs, data_in, _tpm, i);
  //     _inputs[i] = req_inputs;
  //   }
  // }
  if (_step > _pmcmc->decisionStep())
    computeTransitionVector(_tpm, data_in);
  else
    _tpm.assign(_props, 1.0);
  std::vector<Real> req_inputs(_sampler.getNumberOfCols() - 1);
  for (unsigned int i = 0; i < _pmcmc->getNumParallelProposals(); ++i)
  {
    nextSamples(req_inputs, data_in, _tpm, i);
    _inputs[i] = req_inputs;
  }
  // Store data from previous step
  _data_prev = data_in;
  _outputs_prev = _outputs_required;
  nextSeeds();

  // Track the current step
  _check_step = _step;
}
