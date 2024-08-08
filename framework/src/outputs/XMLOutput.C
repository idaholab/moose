//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Moose includes
#include "XMLOutput.h"
#include "FEProblem.h"
#include "MooseApp.h"

registerMooseObject("MooseApp", XMLOutput);

InputParameters
XMLOutput::validParams()
{
  InputParameters params = AdvancedOutput::validParams();
  params.addClassDescription("Output for VectorPostprocessor using XML format.");
  params += AdvancedOutput::enableOutputTypes("vector_postprocessor");
  return params;
}

XMLOutput::XMLOutput(const InputParameters & parameters) : AdvancedOutput(parameters)
{
  // Creates section in VPP
  _xml_doc.append_child("VectorPostprocessors");
}

std::string
XMLOutput::filename()
{
  if (processor_id() > 0)
  {
    std::ostringstream file_name;
    int digits = MooseUtils::numDigits(n_processors());
    file_name << _file_base << ".xml"
              << "." << std::setw(digits) << std::setfill('0') << processor_id();
    return file_name.str();
  }
  return _file_base + ".xml";
}

void
XMLOutput::outputVectorPostprocessors()
{
  // Create pugi node for storing vector data
  auto vpp_node = _xml_doc.child("VectorPostprocessors");
  auto vec_node = vpp_node.append_child("Vectors");

  // Populate output information from FEProblem, do not use AdvancedOutput because the psuedo time
  // is not required.
  vec_node.append_attribute("time") = _problem_ptr->time();
  vec_node.append_attribute("timestep") = _problem_ptr->timeStep();
  if (_execute_enum.isValueSet(EXEC_LINEAR) && !_on_nonlinear_residual)
    vec_node.append_attribute("linear_iteration") = _linear_iter;
  if (_execute_enum.isValueSet(EXEC_NONLINEAR))
    vec_node.append_attribute("nonlinear_iteration") = _nonlinear_iter;

  // The VPP objects to be output
  const std::set<std::string> & out = getVectorPostprocessorOutput();

  // Loop through Reporter values and search for VPP objects that should be output
  for (const auto & r_name : _reporter_data.getReporterNames())
  {
    const std::string & vpp_name = r_name.getObjectName();
    const std::string & vec_name = r_name.getValueName();
    const bool vpp_out = out.find(vpp_name) != out.end();
    if (vpp_out && (_reporter_data.hasReporterValue<VectorPostprocessorValue>(r_name)))
    {
      const VectorPostprocessor & vpp_obj =
          _problem_ptr->getVectorPostprocessorObjectByName(vpp_name);
      auto distributed = vpp_obj.isDistributed();
      if (processor_id() == 0 || distributed)
      {
        // Create a Vector node and associated operators
        auto data_node = vec_node.append_child("Vector");
        data_node.append_attribute("object") = vpp_name.c_str();
        data_node.append_attribute("name") = vec_name.c_str();
        data_node.append_attribute("distributed") = distributed;
        if (distributed)
        {
          data_node.append_attribute("processor_id") = processor_id();
          data_node.append_attribute("n_processors") = n_processors();
          _distributed = true;
        }

        // Write the vector of data
        const auto & vector = _reporter_data.getReporterValue<VectorPostprocessorValue>(r_name);
        std::ostringstream oss;
        std::copy(vector.begin(), vector.end(), infix_ostream_iterator<Real>(oss, " "));
        data_node.text().set(oss.str().c_str());
      }
    }
  }
}

void
XMLOutput::output()
{
  AdvancedOutput::output();
  if (processor_id() == 0 || _distributed)
    _xml_doc.save_file(filename().c_str());
}
