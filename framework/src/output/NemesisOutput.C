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

#include "NemesisOutput.h"

#include "ActionWarehouse.h"
#include "Problem.h"
#include "ActionFactory.h"
#include "MooseObjectAction.h"

// libMesh
#include "libmesh/nemesis_io.h"
#include "libmesh/parallel_mesh.h"

// C++
#include <sstream>
#include <iomanip>

NemesisOutput::NemesisOutput(EquationSystems & es) :
    Outputter(es),
    _out(NULL),
    _seq(false),
    _file_num(-1),
    _num(0)
{
}

NemesisOutput::~NemesisOutput()
{
  delete _out;
}

void
NemesisOutput::sequence(bool state)
{
  _seq = state;
}

std::string
NemesisOutput::getFileName(const std::string & file_base)
{
  std::ostringstream nemesis_stream_file_base;

  nemesis_stream_file_base << file_base;
  if (_seq)
  {
    nemesis_stream_file_base << "_"
                             << std::setw(4)
                             << std::setprecision(0)
                             << std::setfill('0')
                             << std::right
                             << _file_num;
  }

  return nemesis_stream_file_base.str() + ".e";
}


void
NemesisOutput::output(const std::string & file_base, Real time, unsigned int /*t_step*/)
{
  if (_out == NULL)
  {
    _out = new Nemesis_IO( _es.get_mesh());
    _file_num++;
  }

  _num++;
  _out->write_timestep(getFileName(file_base), _es, _num, time);
//  _out->write_element_data(_es);
}

void
NemesisOutput::outputPps(const std::string & /*file_base*/, const FormattedTable & table, Real time)
{
  if (_out == NULL)
    mooseError("Error attempting to write postprocessor information to uninitialized file!");

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
NemesisOutput::meshChanged()
{
  _num = 0;

  delete _out;
  _out = NULL;
}

void
NemesisOutput::outputInput()
{
}
