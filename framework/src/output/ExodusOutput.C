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
ExodusOutput::outputPps(const std::string & file_base, const FormattedTable & table, Real time)
{
  const std::map<Real, std::map<std::string, Real> > & data = table.getData();

  // iterators
  std::map<Real, std::map<std::string, Real> >::const_iterator i(data.end());
  if ( i == data.begin() )
  {
    return;
  }
  --i;
  const Real TIME_TOL(1e-12);
  if (std::abs((time - i->first)/time) > TIME_TOL)
  {
    // Try to find a match
    for ( i = data.begin(); i != data.end(); ++i )
    {
      if (std::abs((time - i->first)/time) < TIME_TOL)
      {
        break;
      }
    }
    if ( i == data.end() )
    {
      --i;
      std::cerr << "Input time: " << time
                << "\nTable time: " << i->first << std::endl;
      mooseError("Time mismatch in outputting Exodus global variables\n"
                 "Have the postprocessor values been computed with the correct time?");
    }
  }
  const std::map<std::string, Real> & tmp = i->second;
  std::vector<Real> global_vars;
  std::vector<std::string> global_var_names;
  for (std::map<std::string, Real>::const_iterator ii(tmp.begin()); ii != tmp.end(); ++ii)
  {
    global_var_names.push_back( ii->first );
    global_vars.push_back( ii->second );
  }

  if (global_vars.size() != global_var_names.size())
  {
    mooseError("Error in outputting global vars to exodus.");
  }

  if (_out == NULL)
    _out = new ExodusII_IO(_es.get_mesh());

  _out->write_global_data( global_vars, global_var_names );
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
