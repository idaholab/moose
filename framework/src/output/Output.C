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

#include "Output.h"
#include "Problem.h"

#include "Outputter.h"
#include "ExodusOutput.h"

Output::Output(Problem & problem) :
    _file_base("out"),
    _problem(problem),
    _time(_problem.time())
{
}

Output::~Output()
{
  for (unsigned int i = 0; i < _outputters.size(); i++)
    delete _outputters[i];
}

void
Output::addExodus()
{
  Outputter *o = new ExodusOutput(_problem.es());
  _outputters.push_back(o);
}

void
Output::output()
{
  for (unsigned int i = 0; i < _outputters.size(); i++)
  {
    _outputters[i]->output(_file_base, _time);
  }
}

void
Output::outputPps(const FormattedTable & table)
{
  for (unsigned int i = 0; i < _outputters.size(); i++)
    _outputters[i]->outputPps(_file_base, table, _time);
}

void
Output::meshChanged()
{
  for (unsigned int i = 0; i < _outputters.size(); i++)
    _outputters[i]->meshChanged();
}

void
Output::sequence(bool state)
{
  for (unsigned int i = 0; i < _outputters.size(); i++)
    _outputters[i]->sequence(state);
}
