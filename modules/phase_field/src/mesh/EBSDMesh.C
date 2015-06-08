/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "EBSDMesh.h"

template<>
InputParameters validParams<EBSDMesh>()
{
  InputParameters params = validParams<GeneratedMesh>();
  params.addClassDescription("Mesh generated from a specified EBSD data file");
  params.addRequiredParam<FileName>("filename", "The name of the file containing the EBSD data");
  params.addParam<unsigned int>("uniform_refine", 0, "Number of coarsening levels available in adaptive mesh refinement.");

  // suppress parameters
  params.suppressParameter<MooseEnum>("dim");
  params.set<MooseEnum>("dim") = MooseEnum("1=1 2 3", "1");
  params.suppressParameter<int>("nx");
  params.suppressParameter<int>("ny");
  params.suppressParameter<int>("nz");
  params.suppressParameter<int>("xmin");
  params.suppressParameter<int>("ymin");
  params.suppressParameter<int>("zmin");
  params.suppressParameter<int>("xmax");
  params.suppressParameter<int>("ymax");
  params.suppressParameter<int>("zmax");

  return params;
}

EBSDMesh::EBSDMesh(const std::string & name, InputParameters parameters) :
    GeneratedMesh(name, parameters),
    _filename(getParam<FileName>("filename"))
{
  if (_nx != 1 || _ny != 1 || _nz !=1)
    mooseWarning("Do not specify mesh geometry information, it is read from the EBSD file.");
}

EBSDMesh::~EBSDMesh()
{
}

void
EBSDMesh::readEBSDHeader()
{
  std::ifstream stream_in(_filename.c_str());

  if (!stream_in)
    mooseError("Can't open EBSD file: " << _filename);

  // Labels to look for in the header
  std::vector<std::string> labels;
  labels.push_back("X_step"); // 0
  labels.push_back("X_Dim");
  labels.push_back("Y_step"); // 2
  labels.push_back("Y_Dim");
  labels.push_back("Z_step"); // 4
  labels.push_back("Z_Dim");
  labels.push_back("X_Min");  // 6
  labels.push_back("Y_Min");
  labels.push_back("Z_Min");

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
    // in them.
    if (line.find("#") == 0)
    {
      // Process lines that start with a comment character (comments and meta data)

      for (unsigned i=0; i<labels.size(); ++i)
        if (line.find(labels[i]) != std::string::npos)
        {
          // Moose::out << "Found label " << labels[i] << ": " << line << std::endl;
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
  for (dim = 3; dim > 0 && _geometry.n[dim-1] == 0; --dim);

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

  unsigned int nr[3];
  nr[0] = _geometry.n[0];
  nr[1] = _geometry.n[1];
  nr[2] = _geometry.n[2];

  // set min/max box length
  InputParameters & params = parameters();
  params.set<Real>("xmin") = _geometry.min[0];
  params.set<Real>("xmax") = nr[0] * _geometry.d[0] + _geometry.min[0];
  params.set<Real>("ymin") = _geometry.min[1];
  params.set<Real>("ymax") = nr[1] * _geometry.d[1] + _geometry.min[1];
  params.set<Real>("zmin") = _geometry.min[2];
  params.set<Real>("zmax") = nr[2] * _geometry.d[2] + _geometry.min[2];

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
