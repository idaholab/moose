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

#include "RestartableDataIO.h"
#include "MooseUtils.h"
#include "RestartableData.h"
#include "FEProblem.h"
#include "MooseApp.h"

#include <stdio.h>

RestartableDataIO::RestartableDataIO(FEProblem & fe_problem) :
    _fe_problem(fe_problem)
{
}

void
RestartableDataIO::writeRestartableData(std::string base_file_name, const RestartableDatas & restartable_datas, std::set<std::string> & _recoverable_data)
{
  unsigned int n_threads = libMesh::n_threads();
  unsigned int n_procs = libMesh::n_processors();
  unsigned int proc_id = libMesh::processor_id();

  for(unsigned int tid=0; tid<n_threads; tid++)
  {
    const std::map<std::string, RestartableDataValue *> & restartable_data = restartable_datas[tid];

    if(restartable_data.size())
    {
      const unsigned int file_version = 1;

      std::ofstream out;

      std::ostringstream file_name_stream;
      file_name_stream << base_file_name;

      file_name_stream << "-" << proc_id;

      if(n_threads > 1)
        file_name_stream << "-" << tid;

      std::string file_name = file_name_stream.str();

      { // Write out header
        out.open(file_name.c_str(), std::ios::out | std::ios::binary);

        char id[2];

        // header
        id[0] = 'R';
        id[1] = 'D';

        out.write(id, 2);
        out.write((const char *)&file_version, sizeof(file_version));

        out.write((const char *)&n_procs, sizeof(n_procs));
        out.write((const char *)&n_threads, sizeof(n_threads));

        // number of RestartableData
        unsigned int n_data = restartable_data.size();
        out.write((const char *) &n_data, sizeof(n_data));

        // data names
        for(std::map<std::string, RestartableDataValue *>::const_iterator it = restartable_data.begin();
            it != restartable_data.end();
            ++it)
        {
          std::string name = it->first;
          out.write(name.c_str(), name.length() + 1); // trailing 0!
        }
      }
      {
        std::ostringstream data_blk;

        for(std::map<std::string, RestartableDataValue *>::const_iterator it = restartable_data.begin();
            it != restartable_data.end();
            ++it)
        {
          // Moose::out<<"Storing "<<it->first<<std::endl;

          std::ostringstream data;
          it->second->store(data);

          // Store the size of the data then the data
          unsigned int data_size = data.tellp();
          data_blk.write((const char *) &data_size, sizeof(data_size));
          data_blk << data.str();
        }

        // Write out this proc's block size
        unsigned int data_blk_size = data_blk.tellp();
        out.write((const char *) &data_blk_size, sizeof(data_blk_size));

        // Write out the values
        out << data_blk.str();

        out.close();
      }
    }
  }
}

void
RestartableDataIO::readRestartableData(std::string base_file_name, RestartableDatas & restartable_datas, std::set<std::string> & _recoverable_data)
{
  bool recovering = _fe_problem.getMooseApp().isRecovering();

  unsigned int n_threads = libMesh::n_threads();
  unsigned int n_procs = libMesh::n_processors();
  unsigned int proc_id = libMesh::processor_id();

  std::vector<std::string> ignored_data;

  for(unsigned int tid=0; tid<n_threads; tid++)
  {
    std::map<std::string, RestartableDataValue *> & restartable_data = restartable_datas[tid];

    if(restartable_data.size())
    {
      std::ostringstream file_name_stream;
      file_name_stream << base_file_name;

      file_name_stream << "-" << proc_id;

      if(n_threads > 1)
        file_name_stream << "-" << tid;

      std::string file_name = file_name_stream.str();

      MooseUtils::checkFileReadable(file_name);

      const unsigned int file_version = 1;

      std::ifstream in(file_name.c_str(), std::ios::in | std::ios::binary);

      // header
      char id[2];
      in.read(id, 2);

      unsigned int this_file_version;
      in.read((char *)&this_file_version, sizeof(this_file_version));

      unsigned int this_n_procs = 0;
      unsigned int this_n_threads = 0;

      in.read((char *)&this_n_procs, sizeof(this_n_procs));
      in.read((char *)&this_n_threads, sizeof(this_n_threads));

      // check the header
      if(id[0] != 'R' || id[1] != 'D')
        mooseError("Corrupted restartable data file!");

      // check the file version
      if(this_file_version > file_version)
        mooseError("Trying to restart from a newer file version - you need to update MOOSE");

      if(this_file_version < file_version)
        mooseError("Trying to restart from an older file version - you need to checkout an older version of MOOSE.");

      if(this_n_procs != n_procs)
        mooseError("Cannot restart using a different number of processors!");

      if(this_n_threads != n_threads)
        mooseError("Cannot restart using a different number of threads!");

      // number of data
      unsigned int n_data = 0;
      in.read((char *) &n_data, sizeof(n_data));

      // data names
      std::vector<std::string> data_names(n_data);

      for(unsigned int i=0; i < n_data; i++)
      {
        std::string data_name;
        char ch = 0;
        do {
          in.read(&ch, 1);
          if (ch != '\0')
            data_name += ch;
        } while (ch != '\0');
        data_names[i] = data_name;
      }

      // Grab this processor's block size
      unsigned int data_blk_size = 0;
      in.read((char *) &data_blk_size, sizeof(data_blk_size));

      for(unsigned int i=0; i < n_data; i++)
      {
        std::string current_name = data_names[i];

        unsigned int data_size = 0;
        in.read((char *) &data_size, sizeof(data_size));

        if(restartable_data.find(current_name) != restartable_data.end() // Only restore values if they're currently being used
           && (recovering || (_recoverable_data.find(current_name) == _recoverable_data.end())) // Only read this value if we're either recovering or this hasn't been specified to be recovery only data
          )
        {
          // Moose::out<<"Loading "<<current_name<<std::endl;

          RestartableDataValue * current_data = restartable_data[current_name];
          current_data->load(in);
        }
        else
        {
          // Skip this piece of data
          in.seekg(data_size, std::ios_base::cur);
          ignored_data.push_back(current_name);
        }
      }

      in.close();
    }
  }

  if(ignored_data.size())
  {
    std::ostringstream names;

    for(unsigned int i=0; i<ignored_data.size(); i++)
      names << ignored_data[i] << "\n";

    mooseWarning("The following RestorableData was found in restart file but is being ignored:\n" << names.str());

  }
}
