#pragma once
#include <fstream>
#include <iostream>
#include <memory>

#include "logging.hpp"
#include "../common/pfem_extras.hpp"
#include "gridfunctions.hpp"
#include "mesh_extras.hpp"
#include "named_fields_map.hpp"

namespace hephaestus
{

class Outputs : public hephaestus::NamedFieldsMap<mfem::DataCollection>
{
  friend class ProblemBuilder;

public:
  Outputs();
  Outputs(hephaestus::GridFunctions & gridfunctions);

  ~Outputs();

  // Set output fields to write out. If output_field_names is empty, all
  // gridfunctions will be written by default.
  void SetOutputFieldNames(std::vector<std::string> & output_field_names)
  {
    _output_field_names = output_field_names;
  }

  // Enable GLVis streams for visualisation
  void EnableGLVis(const bool & use_glvis)
  {
    _use_glvis = use_glvis;
    if (_use_glvis)
      InitializeGLVis(_my_rank);
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

    // Initialize GLVis _use_glvis and send the initial condition
    // by socket to a GLVis server.
    if (_use_glvis)
    {
      DisplayToGLVis();
    }
  }

  // Write outputs out to requested streams
  void Write(double t = 1.0)
  {
    // Wait for all ranks to finish updating their solution before output
    MPI_Barrier(_my_comm);
    // Update cycle counter
    _cycle++;
    // Output timestep summary to console
    WriteConsoleSummary(_my_rank, t);
    if (_use_glvis)
    {
      DisplayToGLVis();
    }
    // Save output fields at timestep to DataCollections
    WriteOutputFields(t);
  }

private:
  std::map<std::string, mfem::socketstream *> _socks;
  hephaestus::GridFunctions * _gridfunctions;
  std::vector<std::string> _output_field_names{};
  int _cycle{0};
  bool _use_glvis{false};
  MPI_Comm _my_comm{MPI_COMM_WORLD};
  int _n_ranks, _my_rank;

  // Initialize Outputs with Gridfunctions; used in ProblemBuilder
  void Init(hephaestus::GridFunctions & gridfunctions)
  {
    SetGridFunctions(gridfunctions);
    Reset();
  }

  void SetGridFunctions(hephaestus::GridFunctions & gridfunctions)
  {
    _gridfunctions = &gridfunctions;
  }

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

  // Write out summary of last timestep to console
  void WriteConsoleSummary(int _my_rank, double t) { logger.info("step {}, \tt = {}", _cycle, t); }

  // Initialize GLVis sockets and fields
  void InitializeGLVis(int _my_rank)
  {
    logger.info("Opening GLVis sockets.");

    for (auto & gridfunction : *_gridfunctions)
    {
      _socks[gridfunction.first] = new mfem::socketstream;
      _socks[gridfunction.first]->precision(8);
    }

    logger.info("GLVis sockets open.");
  }

  // Update GLVis display of output fields (gridfunctions)
  void DisplayToGLVis()
  {
    char vishost[] = "localhost";
    int visport = 19916;

    int wx = 0, wy = 0;                 // window position
    int ww = 350, wh = 350;             // window size
    int offx = ww + 10, offy = wh + 45; // window offsets

    for (auto & gridfunction : *_gridfunctions)
    {
      mfem::common::VisualizeField(*_socks[gridfunction.first],
                                   vishost,
                                   visport,
                                   *(gridfunction.second),
                                   (gridfunction.first).c_str(),
                                   wx,
                                   wy,
                                   ww,
                                   wh);
      wx += offx;
    }
  }
};

} // namespace hephaestus
