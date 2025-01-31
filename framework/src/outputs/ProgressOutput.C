//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ProgressOutput.h"
#include "TransientBase.h"

registerMooseObjectAliased("MooseApp", ProgressOutput, "Progress");

InputParameters
ProgressOutput::validParams()
{
  auto params = Output::validParams();
  params.addClassDescription("Output a simulation time progress bar on the console.");
  params.set<ExecFlagEnum>("execute_on") = {EXEC_TIMESTEP_END};
  params.addParam<bool>(
      "use_filename", true, "Put the input filename into the title of the progress bar");
  params.addParam<unsigned int>(
      "progress_bar_width",
      "Explicitly specify the bar width. If omitted the MOOSE_PPS_WIDTH environment variable or, "
      "if not set, the terminal width is queried.");
  return params;
}

ProgressOutput::ProgressOutput(const InputParameters & parameters)
  : Output(parameters),
    _transient_executioner(dynamic_cast<TransientBase *>(_app.getExecutioner())),
    _use_filename(getParam<bool>("use_filename")),
    _length(isParamValid("progress_bar_width") ? getParam<unsigned int>("progress_bar_width")
                                               : MooseUtils::getTermWidth(true) - 2)
{
}

void
ProgressOutput::output()
{
  if (_transient_executioner == nullptr || _current_execute_flag != EXEC_TIMESTEP_END)
    return;

  const auto passed = _transient_executioner->getTime() - _transient_executioner->getStartTime();
  const auto total = _transient_executioner->getEndTime() - _transient_executioner->getStartTime();
  if (total == 0)
    return;

  // length of filled portion
  const auto progress = std::round((passed * _length) / total);

  // title string
  std::string title = name();
  if (_use_filename)
    title += " (" + getMooseApp().getFileName() + ')';
  if (title.length() >= _length - 1)
    title = title.substr(0, _length - 4) + "...";

  // top line
  Moose::out << "+-" << title << std::string(_length - 1 - title.length(), '-') << "+\n";

  // bar
  Moose::out << '|' << std::string(progress, '#') << std::string(_length - progress, '.') << "|\n";

  // bottom line
  Moose::out << '+' << std::string(_length, '-') << "+\n";
}
