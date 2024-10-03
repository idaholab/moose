#pragma once
#include <fstream>
#include <iostream>
#include <memory>

// #include "logging.h"
#include "../common/pfem_extras.hpp"
#include "MFEMContainers.h"
#include "mesh_extras.hpp"

class MFEMProblem;

namespace platypus
{

class Outputs : public platypus::NamedFieldsMap<mfem::DataCollection>
{

public:
  Outputs();
  ~Outputs(){};

  // Set output fields to write out. If output_field_names is empty, all
  // gridfunctions will be written by default.
  void SetOutputFieldNames(std::vector<std::string> & output_field_names)
  {
    _output_field_names = output_field_names;
  }

  void SetGridFunctions(platypus::GridFunctions & gridfunctions)
  {
    _gridfunctions = &gridfunctions;
  }

  // Reset Outputs and re-register output fields from GridFunctions
  void Reset()
  {
    // Reset cycle counter
    _cycle = 0;
    // Set up DataCollections to track fields of interest.
    RegisterOutputFields();
    // Write initial fields to disk
    WriteOutputFields(0.0);
  }

  // Write outputs out to requested streams
  void Write(double t = 1.0)
  {
    // Wait for all ranks to finish updating their solution before output
    MPI_Barrier(_my_comm);
    // Update cycle counter
    _cycle++;
    // Save output fields at timestep to DataCollections
    WriteOutputFields(t);
  }

private:
  platypus::GridFunctions * _gridfunctions;
  std::vector<std::string> _output_field_names{};
  int _cycle{0};
  MPI_Comm _my_comm{MPI_COMM_WORLD};
  int _n_ranks, _my_rank;

  // Register fields (gridfunctions) to write to DataCollections
  void RegisterOutputFields()
  {
    for (auto & output : *this)
    {
      auto const & dc(output.second);
      mfem::ParMesh * pmesh(_gridfunctions->begin()->second->ParFESpace()->GetParMesh());
      dc->SetMesh(pmesh);

      // NB: data collections must NOT own pointers otherwise we will have a double-free.
      if (_output_field_names.empty())
      {
        for (auto & gridfunction : *_gridfunctions)
        {
          dc->RegisterField(gridfunction.first, gridfunction.second.get());
        }
      }
      else
      {
        for (auto field_name : _output_field_names)
        {
          dc->RegisterField(field_name, _gridfunctions->Get(field_name));
        }
      }
    }
  }

  // Write out fields (gridfunctions) to DataCollections
  void WriteOutputFields(double t)
  {
    // Write fields to disk
    for (auto & output : *this)
    {
      auto const & dc(output.second);
      dc->SetCycle(_cycle);
      dc->SetTime(t);
      dc->Save();
    }
  }
};

} // namespace platypus
