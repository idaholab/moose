/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

// MOOSE includes
#include "SamplerTransfer.h"
#include "SamplerMultiApp.h"
#include "SamplerReceiver.h"

template <>
InputParameters
validParams<SamplerTransfer>()
{
  InputParameters params = validParams<MultiAppTransfer>();
  params.addClassDescription("Copies Sampler data to a SamplerReceiver object.");
  params.set<MooseEnum>("direction") = "to_multiapp";
  params.suppressParameter<MooseEnum>("direction");
  params.addParam<std::vector<std::string>>(
      "parameters",
      "A list of parameters (on the sub application) to control "
      "with the Sampler data. The order of the parameters listed "
      "here should match the order of the items in the Sampler.");
  params.addRequiredParam<std::string>("to_control",
                                       "The name of the 'SamplerReceiver' on the sub application "
                                       "to which the Sampler data will be transfered.");
  return params;
}

SamplerTransfer::SamplerTransfer(const InputParameters & parameters)
  : MultiAppTransfer(parameters),
    _parameter_names(getParam<std::vector<std::string>>("parameters")),
    _receiver_name(getParam<std::string>("to_control"))
{

  // Determine the Sampler
  std::shared_ptr<SamplerMultiApp> ptr = std::dynamic_pointer_cast<SamplerMultiApp>(_multi_app);
  if (!ptr)
    mooseError("The 'multi_app' parameter must provide a 'SamplerMultiApp' object.");
  _sampler_ptr = &(ptr->getSampler("sampler"));

  // Compute the matrix and row for each
  std::vector<DenseMatrix<Real>> out = _sampler_ptr->getSamples();
  for (auto mat = beginIndex(out); mat < out.size(); ++mat)
    for (unsigned int row = 0; row < out[mat].m(); ++row)
      _multi_app_matrix_row.push_back(std::make_pair(mat, row));
}

void
SamplerTransfer::execute()
{
  // Get the Sampler data
  const std::vector<DenseMatrix<Real>> samples = _sampler_ptr->getSamples();

  // Loop over all sub-apps
  for (unsigned int app_index = 0; app_index < _multi_app->numGlobalApps(); app_index++)
  {
    // Do nothing if the sub-app is not local
    if (!_multi_app->hasLocalApp(app_index))
      continue;

    // Get the sub-app SamplerReceiver object and perform error checking
    std::shared_ptr<SamplerReceiver> ptr = getReceiver(app_index, samples);

    // Perform the transfer
    std::pair<unsigned int, unsigned int> loc = _multi_app_matrix_row[app_index];
    ptr->reset(); // clears existing parameter settings
    for (auto j = beginIndex(_parameter_names); j < _parameter_names.size(); ++j)
    {
      const Real & data = samples[loc.first](loc.second, j);
      ptr->addControlParameter(_parameter_names[j], data);
    }
  }
}

std::shared_ptr<SamplerReceiver>
SamplerTransfer::getReceiver(unsigned int app_index, const std::vector<DenseMatrix<Real>> & samples)
{
  // Test that the sub-application has the given Control object
  FEProblemBase & to_problem = _multi_app->appProblemBase(app_index);
  ExecuteMooseObjectWarehouse<Control> & control_wh = to_problem.getControlWarehouse();
  if (!control_wh.hasActiveObject(_receiver_name))
    mooseError("The sub-application (",
               _multi_app->name(),
               ") does not contain a Control object with the name '",
               _receiver_name,
               "'.");

  std::shared_ptr<SamplerReceiver> ptr =
      std::dynamic_pointer_cast<SamplerReceiver>(control_wh.getActiveObject(_receiver_name));
  if (!ptr)
    mooseError(
        "The sub-application (",
        _multi_app->name(),
        ") Control object for the 'to_control' parameter must be of type 'SamplerReceiver'.");

  // Test the size of parameter list with the number of columns in Sampler matrix
  std::pair<unsigned int, unsigned int> loc = _multi_app_matrix_row[app_index];
  if (_parameter_names.size() != samples[loc.first].n())
    mooseError("The number of parameters (",
               _parameter_names.size(),
               ") does not match the number of columns (",
               samples[loc.first].n(),
               ") in the Sampler data matrix with index ",
               loc.first,
               ".");

  return ptr;
}
