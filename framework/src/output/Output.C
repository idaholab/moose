#include "Output.h"

#include "Outputter.h"
#include "ExodusOutput.h"


Output::Output(Moose::Problem & problem) :
  _file_base("out"),
  _problem(problem)
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
  Outputter *o = new ExodusOutput(_problem);
  _outputters.push_back(o);
}

void
Output::output()
{
  for (unsigned int i = 0; i < _outputters.size(); i++)
  {
    _outputters[i]->output(_file_base);
  }
}

