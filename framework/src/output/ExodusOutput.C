#include "ExodusOutput.h"
#include "SubProblem.h"

// libMesh
#include "exodusII_io.h"

namespace Moose {

ExodusOutput::ExodusOutput(SubProblem & problem) :
  Outputter(problem),
  _out(problem.mesh()),
  _num(0),
  _time(_problem.time())
{
}

ExodusOutput::~ExodusOutput()
{
}

std::string
ExodusOutput::getFileName(const std::string & file_base)
{
  return file_base + ".e";
}


void
ExodusOutput::output(const std::string & file_base)
{
  _num++;
  _out.write_timestep(getFileName(file_base), _problem.es(), _num, _time);
  _out.write_element_data(_problem.es());
}

} // namespace
