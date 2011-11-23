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
#include "MooseInit.h"

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
    _out->set_output_variables(_output_variables);
    _out->use_mesh_dimension_instead_of_spatial_dimension(true);  //Skip output of z coordinates for 2D meshes
    _file_num++;
  }

  _num++;
  _out->write_timestep(getFileName(file_base), _es, _num, time);
  _out->write_element_data(_es);
}

void
ExodusOutput::outputPps(const std::string & /*file_base*/, const FormattedTable & table, Real time)
{
  if (_out == NULL)
    return;     // do nothing and safely return - we can write global vars (i.e. PPS only when output() occured)

  // Check to see if the FormattedTable is empty, if so, return
  if (table.getData().empty())
    return;

  // Search through the map, find a time in the table which matches the input time.
  // Note: search in reverse, since the input time is most likely to be the most recent time.
  const Real time_tol = 1.e-12;

  std::map<Real, std::map<std::string, Real> >::const_reverse_iterator
    rit  = table.getData().rbegin(),
    rend = table.getData().rend();

  for (; rit != rend; ++rit)
    {
      // Difference between input time and the time stored in the table
      Real time_diff = std::abs((time - (*rit).first));

      // Get relative difference, but don't divide by zero!
      if ( std::abs(time) > 0.)
        time_diff /= std::abs(time);

      // Break out of the loop if we found the right time
      if (time_diff < time_tol)
        break;
    }

  // If we didn't find anything, print an error message
  if ( rit == rend )
    {
      std::cerr << "Input time: " << time
                << "\nLatest Table time: " << (*(table.getData().rbegin())).first << std::endl;
      mooseError("Time mismatch in outputting Nemesis global variables\n"
                 "Have the postprocessor values been computed with the correct time?");
    }

  // Otherwise, fill local vectors with name/value information and write to file.
  const std::map<std::string, Real> & tmp = (*rit).second;

  std::vector<Real> global_vars;
  std::vector<std::string> global_var_names;
  global_vars.reserve(tmp.size());
  global_var_names.reserve(tmp.size());

  for (std::map<std::string, Real>::const_iterator ii = tmp.begin();
       ii != tmp.end(); ++ii)
  {
    // Push back even though we know the exact size, saves us keeping
    // track of one more index.
    global_var_names.push_back( (*ii).first );
    global_vars.push_back( (*ii).second );
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
  // parser/action system are not mandatory subsystems to use, thus empty action system -> no input output
  if (Moose::action_warehouse.empty())
    return;

  std::vector<std::string> input_file_record;
  std::string s;

  if (_out == NULL)
    _out = new ExodusII_IO(_es.get_mesh());

  input_file_record.push_back("####################");
  input_file_record.push_back("# Created by MOOSE #");
  input_file_record.push_back("####################");

  {
    std::stringstream ss;
    // Grab the command line arguments first
    Moose::command_line->print("", ss, 1);

    input_file_record.push_back("### Command Line Arguments ###");
    while (std::getline(ss, s))
    {
      mooseAssert(s.length() <= MAX_LINE_LENGTH, "Command line argument length too long to fit into Exodus Record");
      input_file_record.push_back(s);
    }
  }

  {
    std::stringstream ss;
    // Save the input file into our string stream
    Moose::action_warehouse.printInputFile(ss);

    input_file_record.push_back("### Input File ###");
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
  }


  _out->write_information_records( input_file_record );
}
