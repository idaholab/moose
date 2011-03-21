#include "ExodusOutput.h"
#include "Problem.h"

// libMesh
#include "exodusII_io.h"

namespace Moose {

ExodusOutput::ExodusOutput(EquationSystems & es) :
  Outputter(es),
  _out(NULL),
  _seq(false),
  _file_num(0),
  _num(0)
{
}

ExodusOutput::~ExodusOutput()
{
  delete _out;
}

std::string
ExodusOutput::getFileName(const std::string & file_base)
{
  OStringStream exodus_stream_file_base;

  exodus_stream_file_base << file_base;
  if (_seq)
  {
    OSSRealzeroright(exodus_stream_file_base, 4, 0, _file_num);
  }

  return exodus_stream_file_base.str() + ".e";
}


void
ExodusOutput::output(const std::string & file_base, Real time)
{
  if (_out == NULL)
    _out = new ExodusII_IO(_es.get_mesh());

  _num++;
  _out->write_timestep(getFileName(file_base), _es, _num, time);
  _out->write_element_data(_es);
}

void
ExodusOutput::meshChanged()
{
  _file_num++;
  _num = 0;

  delete _out;
  _out = NULL;
}

} // namespace
