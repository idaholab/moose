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

#include "ExodusOutput.h"

#include "ActionWarehouse.h"
#include "Problem.h"
#include "ActionFactory.h"
#include "MooseObjectAction.h"

// libMesh
#include "exodusII.h"
#include "exodusII_io.h"

ExodusOutput::ExodusOutput(EquationSystems & es) :
    Outputter(es),
    _out(NULL),
    _seq(false),
    _file_num(-1),
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
    exodus_stream_file_base << "_";
    OSSRealzeroright(exodus_stream_file_base, 4, 0, _file_num);
  }

  return exodus_stream_file_base.str() + ".e";
}


void
ExodusOutput::output(const std::string & file_base, Real time)
{
  if (_out == NULL)
  {
    _out = new ExodusII_IO(_es.get_mesh());
    _file_num++;
  }

  _num++;
  _out->write_timestep(getFileName(file_base), _es, _num, time);
  _out->write_element_data(_es);
}

void
ExodusOutput::outputPps(const std::string & /*file_base*/, const FormattedTable & table, Real time)
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
  {
    // FIXME: If _out==NULL and we create it here and immediately call
    // write_global_data() it will fail because it's not initialized.
    // Therefore this should probably be an error, and the check could
    // go at the beginning of the function.
    _out = new ExodusII_IO(_es.get_mesh());
  }

  _out->write_global_data( global_vars, global_var_names );
}


void
ExodusOutput::meshChanged()
{
  _num = 0;

  delete _out;
  _out = NULL;
}

void
ExodusOutput::outputInput()
{
  if (_out == NULL)
    _out = new ExodusII_IO(_es.get_mesh());

  std::stringstream ss;
  Moose::action_warehouse.printInputFile(ss);

  std::vector<std::string> input_file_record;
  input_file_record.push_back("####################");
  input_file_record.push_back("# Created by MOOSE #");
  input_file_record.push_back("####################");
  std::string s;
  while (std::getline(ss, s))
  {
    // MAX_LINE_LENGTH is from ExodusII
    if ( s.length() > MAX_LINE_LENGTH )
    {
      const std::string continuation("...");
      const size_t cont_len(continuation.length());
      size_t num_lines = s.length() / (MAX_LINE_LENGTH - cont_len) + 1;
      std::string split_line;
      for (size_t j(0), l_begin(0); j < num_lines; ++j, l_begin+=MAX_LINE_LENGTH-cont_len)
      {
        size_t l_len = MAX_LINE_LENGTH-cont_len;
        if (s.length() < l_begin + l_len )
        {
          l_len = s.length() - l_begin;
        }
        split_line = s.substr( l_begin, l_len );
        if ( l_begin + l_len != s.length())
        {
          split_line += continuation;
        }
        input_file_record.push_back(split_line);
      }
    }
    else
    {
      input_file_record.push_back(s);
    }
  }

  _out->write_information_records( input_file_record );
}
