//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EBSDMesh.h"
#include "MooseApp.h"

#include <fstream>

registerMooseObject("PhaseFieldApp", EBSDMesh);

InputParameters
EBSDMesh::validParams()
{
  InputParameters params = GeneratedMesh::validParams();
  params.addClassDescription("Mesh generated from a specified DREAM.3D EBSD data file.");
  params.addRequiredParam<FileName>("filename", "The name of the file containing the EBSD data");
  params.addParam<unsigned int>(
      "uniform_refine", 0, "Number of coarsening levels available in adaptive mesh refinement.");

  // suppress parameters
  params.suppressParameter<MooseEnum>("dim");
  params.set<MooseEnum>("dim") = MooseEnum("1=1 2 3", "1");
  params.suppressParameter<unsigned int>("nx");
  params.suppressParameter<unsigned int>("ny");
  params.suppressParameter<unsigned int>("nz");
  params.suppressParameter<Real>("xmin");
  params.suppressParameter<Real>("ymin");
  params.suppressParameter<Real>("zmin");
  params.suppressParameter<Real>("xmax");
  params.suppressParameter<Real>("ymax");
  params.suppressParameter<Real>("zmax");

  return params;
}

EBSDMesh::EBSDMesh(const InputParameters & parameters)
  : GeneratedMesh(parameters), _filename(getParam<FileName>("filename"))
{
  mooseDeprecated(
      "EBSDMesh is deprecated, please use the EBSDMeshGenerator instead. For example:\n\n[Mesh]\n  "
      "type = EBDSMesh\n  filename = my_ebsd_data.dat\n[]\n\nbecomes\n\n[Mesh]\n  [ebsd_mesh]\n    "
      "type = EBDSMeshGenerator\n    filename = my_ebsd_data.dat\n  []\n[]");

  if (_nx != 1 || _ny != 1 || _nz != 1)
    mooseWarning("Do not specify mesh geometry information, it is read from the EBSD file.");
}

EBSDMesh::~EBSDMesh() {}

void
EBSDMesh::readEBSDHeader()
{
  std::ifstream stream_in(_filename.c_str());

  if (!stream_in)
    paramError("filename", "Can't open EBSD file: ", _filename);

  // Labels to look for in the header
  std::vector<std::string> labels = {
      "x_step", "x_dim", "y_step", "y_dim", "z_step", "z_dim", "x_min", "y_min", "z_min"};

  // Dimension variables to store once they are found in the header
  // X_step, X_Dim, Y_step, Y_Dim, Z_step, Z_Dim
  // We use Reals even though the Dim values should all be integers...
  std::vector<Real> label_vals(labels.size(), 0.0);

  std::string line;
  while (std::getline(stream_in, line))
  {
    // We need to process the comment lines that have:
    // X_step, X_Dim
    // Y_step, Y_Dim
    // Z_step, Z_Dim
    // in them. The labels are case insensitive.
    if (line.find("#") == 0)
    {
      // Process lines that start with a comment character (comments and meta data)
      std::transform(line.begin(), line.end(), line.begin(), ::tolower);

      for (unsigned i = 0; i < labels.size(); ++i)
        if (line.find(labels[i]) != std::string::npos)
        {
          std::string dummy;
          std::istringstream iss(line);
          iss >> dummy >> dummy >> label_vals[i];

          // One label per line, break out of for loop over labels
          break;
        }
    }
    else
      // first non comment line marks the end of the header
      break;
  }

  // Copy stuff out of the label_vars array into class variables
  _geometry.d[0] = label_vals[0];
  _geometry.n[0] = label_vals[1];
  _geometry.min[0] = label_vals[6];

  _geometry.d[1] = label_vals[2];
  _geometry.n[1] = label_vals[3];
  _geometry.min[1] = label_vals[7];

  _geometry.d[2] = label_vals[4];
  _geometry.n[2] = label_vals[5];
  _geometry.min[2] = label_vals[8];

  unsigned int dim;

  // determine mesh dimension
  for (dim = 3; dim > 0 && _geometry.n[dim - 1] == 0; --dim)
    ;

  // check if the data has nonzero stepsizes
  for (unsigned i = 0; i < dim; ++i)
  {
    if (_geometry.n[i] == 0)
      mooseError("Error reading header, EBSD grid size is zero.");
    if (_geometry.d[i] == 0.0)
      mooseError("Error reading header, EBSD data step size is zero.");
  }

  if (dim == 0)
    mooseError("Error reading header, EBSD data is zero dimensional.");

  _geometry.dim = dim;
}

void
EBSDMesh::buildMesh()
{
  readEBSDHeader();

  unsigned int uniform_refine = getParam<unsigned int>("uniform_refine");
  _dim = (_geometry.dim == 1 ? "1" : (_geometry.dim == 2 ? "2" : "3"));

  std::array<unsigned int, 3> nr;
  nr[0] = _geometry.n[0];
  nr[1] = _geometry.n[1];
  nr[2] = _geometry.n[2];

  // set min/max box length
  _xmin = _geometry.min[0];
  _xmax = nr[0] * _geometry.d[0] + _geometry.min[0];
  _ymin = _geometry.min[1];
  _ymax = nr[1] * _geometry.d[1] + _geometry.min[1];
  _zmin = _geometry.min[2];
  _zmax = nr[2] * _geometry.d[2] + _geometry.min[2];

  // check if the requested uniform refine level is possible and determine initial grid size
  for (unsigned int i = 0; i < uniform_refine; ++i)
    for (unsigned int j = 0; j < _geometry.dim; ++j)
    {
      if (nr[j] % 2 != 0)
        mooseError("EBSDMesh error. Requested uniform_refine levels not possible.");
      nr[j] /= 2;
    }

  _nx = nr[0];
  _ny = nr[1];
  _nz = nr[2];

  GeneratedMesh::buildMesh();
}
