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

// MOOSE includes
#include "GeneralVectorPostprocessor.h"
#include "DelimitedFileReader.h"

// Forward declarations
class CSVReader;

template <>
InputParameters validParams<CSVReader>();

class CSVReader : public GeneralVectorPostprocessor
{
public:
  CSVReader(const InputParameters & parameters);
  void virtual initialize() override;
  void virtual execute() override;

protected:
  /// The MOOSE delimited file reader.
  MooseUtils::DelimitedFileReader _csv_reader;

  /// Data vectors, which are stored in a map to allow for late declarations to occur, i.e., it
  /// is possible for the file to change and add new vectors during the simulation.
  std::map<std::string, VectorPostprocessorValue *> _column_data;
};
