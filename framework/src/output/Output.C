#include "Output.h"
#include "Problem.h"

#include "Outputter.h"
#include "ExodusOutput.h"

namespace Moose {

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

} // namespace
