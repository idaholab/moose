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
#include "nemesis_io.h"

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

std::string
NemesisOutput::getFileName(const std::string & file_base)
{
  OStringStream nemesis_stream_file_base;

  nemesis_stream_file_base << file_base;
  if (_seq)
  {
    nemesis_stream_file_base << "_";
    OSSRealzeroright(nemesis_stream_file_base, 4, 0, _file_num);
  }

  return nemesis_stream_file_base.str() + ".e";
}


void
NemesisOutput::output(const std::string & file_base, Real time)
{
  if (_out == NULL)
  {
#if LIBMESH_ENABLE_PARMESH    
    _out = new Nemesis_IO(libmesh_cast_ref<ParallelMesh&>(_es.get_mesh()));
    _file_num++;
#else
   mooseError("Nemesis not supported when compiled without --enable-parmesh");                      
#endif                          
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
{/*
  if (_out == NULL)
    _out = new NemesisII_IO(_es.get_mesh());

  std::stringstream ss;
  std::ostream * previous_ostream( Action::getOStream() );
  Action::setOStream( ss );
  const std::string * prev_name( NULL );
  for (ActionIterator a = Moose::action_warehouse.inputFileActionsBegin();
       a != Moose::action_warehouse.inputFileActionsEnd();
       ++a)
  {
    if (ActionFactory::instance()->isParsed((*a)->name()))
    {
      (*a)->printInputFile(prev_name);
      prev_name = &((*a)->name());
    }
  }
  Action::setOStream( *previous_ostream );

  std::vector<std::string> input_file_record;
  input_file_record.push_back("####################");
  input_file_record.push_back("# Created by MOOSE #");
  input_file_record.push_back("####################");
  std::string s;
  while (std::getline(ss, s))
  {
    // MAX_LINE_LENGTH is from NemesisII
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
 */
}
