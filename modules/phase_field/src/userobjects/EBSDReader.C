/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "EBSDReader.h"
#include "EBSDMesh.h"
#include "MooseMesh.h"

template<>
InputParameters validParams<EBSDReader>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addRequiredParam<unsigned int>("op_num", "Specifies the number of order parameters to create");
  return params;
}

EBSDReader::EBSDReader(const std::string & name, InputParameters params) :
    GeneralUserObject(name, params),
    _mesh(_fe_problem.mesh()),
    _nl(_fe_problem.getNonlinearSystem()),
    _op_num(getParam<unsigned int>("op_num")),
    _feature_num(0),
    _mesh_dimension(_mesh.dimension()),
    _nx(0),
    _ny(0),
    _nz(0),
    _dx(0.),
    _dy(0.),
    _dz(0.)
{
}

void
EBSDReader::initialSetup()
{
  // No need to re-read data upon recovery
  if (_app.isRecovering())
    return;

  // Fetch and check mesh
  EBSDMesh * mesh = dynamic_cast<EBSDMesh *>(&_mesh);
  if (mesh == NULL)
    mooseError("Please use an EBSDMesh in your simulation.");

  std::ifstream stream_in(mesh->getEBSDFilename().c_str());
  if (!stream_in)
    mooseError("Can't open EBSD file: " << mesh->getEBSDFilename());

  const EBSDMesh::EBSDMeshGeometry & g = mesh->getEBSDGeometry();

  // Copy file header data from the EBSDMesh
  _dx = g.d[0];
  _nx = g.n[0];
  _minx = g.min[0];
  _maxx = _minx + _dx * _nx;

  _dy = g.d[1];
  _ny = g.n[1];
  _miny = g.min[1];
  _maxy = _miny + _dy * _ny;

  _dz = g.d[2];
  _nz = g.n[2];
  _minz = g.min[2];
  _maxz = _minz + _dz * _nz;

  // Resize the _data array
  unsigned total_size = g.dim < 3 ? _nx*_ny : _nx*_ny*_nz;
  _data.resize(total_size);

  std::string line;
  while (std::getline(stream_in, line))
  {
    if (line.find("#") != 0)
    {
      // Temporary variables to read in on each line
      EBSDPointData d;
      Real x, y, z;

      std::istringstream iss(line);
      iss >> d.phi1 >> d.phi >> d.phi2 >> x >> y >> z >> d.grain >> d.phase >> d.symmetry;

      if (x < _minx || y < _miny || x > _maxx || y > _maxy || (g.dim == 3 && (z < _minz || z > _maxz)))
        mooseError("EBSD Data ouside of the domain declared in the header ([" << _minx << ':' << _maxx << "], [" << _miny << ':' << _maxy << "], [" << _minz << ':' << _maxz << "]) dim=" << g.dim << "\n" << line);

      d.p = Point(x,y,z);

      // determine number of grains in the dataset
      if (d.grain > _feature_num) _feature_num = d.grain;

      // The Order parameter is not yet assigned.
      // We initialize it to zero in order not to have undefined values that break the testing.
      d.op = 0;

      unsigned global_index = indexFromPoint(Point(x, y, z));
      _data[global_index] = d;
    }
  }
  stream_in.close();

  // total number of grains is one higher than the maximum grain id
  _feature_num += 1;

  // Resize the variables
  _avg_data.resize(_feature_num);

  // clear the averages
  for (unsigned int i = 0; i < _feature_num; ++i)
  {
    EBSDAvgData & a = _avg_data[i];
    a.p = a.phi1 = a.phi = a.phi2 = 0.0;
    a.symmetry = a.phase = a.n = 0;
  }

  // Iterate through data points to get average variable values for each grain
  for (std::vector<EBSDPointData>::iterator j = _data.begin(); j != _data.end(); ++j)
  {
    EBSDAvgData & a = _avg_data[j->grain];

    //use Eigen::Quaternion<Real> here?
    a.phi1 += j->phi1;
    a.phi  += j->phi;
    a.phi2 += j->phi2;

    if (a.n == 0)
      a.phase = j->phase;
    else
      if (a.phase != j->phase)
        mooseError("An EBSD feature needs to have a uniform phase.");

    if (a.n == 0)
      a.symmetry = j->symmetry;
    else
      if (a.symmetry != j->symmetry)
        mooseError("An EBSD feature needs to have a uniform symmetry parameter.");

    a.p += j->p;
    a.n++;
  }

  for (unsigned int i = 0; i < _feature_num; ++i)
  {
    EBSDAvgData & a = _avg_data[i];

    if (a.n == 0) continue;

    a.phi1 /= Real(a.n);
    a.phi  /= Real(a.n);
    a.phi2 /= Real(a.n);

    if (a.phase >= _feature_id.size())
      _feature_id.resize(a.phase + 1);

    a.grain = _feature_id[a.phase].size();
    _feature_id[a.phase].push_back(i);

    a.p *= 1.0/Real(a.n);
  }

  // Build map
  buildNodeToGrainWeightMap();
}

EBSDReader::~EBSDReader()
{
}

const EBSDReader::EBSDPointData &
EBSDReader::getData(const Point & p) const
{
  return _data[indexFromPoint(p)];
}

const EBSDReader::EBSDAvgData &
EBSDReader::getAvgData(unsigned int var) const
{
  return _avg_data[indexFromIndex(var)];
}

const EBSDReader::EBSDAvgData &
EBSDReader::getAvgData(unsigned int phase, unsigned int grain) const
{
  return _avg_data[indexFromIndex(_feature_id[phase][grain])];
}

unsigned int
EBSDReader::getGrainNum() const
{
  return _feature_num;
}

unsigned int
EBSDReader::getGrainNum(unsigned int phase) const
{
  return _feature_id[phase].size();
}

unsigned int
EBSDReader::indexFromPoint(const Point & p) const
{
  // Don't assume an ordering on the input data, use the (x, y,
  // z) values of this centroid to determine the index.
  unsigned int x_index, y_index, z_index, global_index;

  x_index = (unsigned int)((p(0) - _minx) / _dx);
  y_index = (unsigned int)((p(1) - _miny) / _dy);

  if (_mesh_dimension == 3)
  {
    z_index = (unsigned int)((p(2) - _minz) / _dz);
    global_index = z_index * _ny;
  }
  else
    global_index = 0;

  // Compute the global index into the _data array.  This stores points
  // in a [z][y][x] ordering.
  global_index = (global_index + y_index) * _nx + x_index;

  // Don't access out of range!
  mooseAssert(global_index < _data.size(), "global_index points out of _data range");

  return global_index;
}

unsigned int
EBSDReader::indexFromIndex(unsigned int var) const
{

  // Transfer the index into the _avg_data array.
  unsigned avg_index = var;

  // Don't access out of range!
  if (avg_index >= _avg_data.size())
    mooseError("Error! Index out of range in EBSDReader::indexFromIndex()");

  return avg_index;
}

const std::map<dof_id_type, std::vector<Real> > &
EBSDReader::getNodeToGrainWeightMap() const
{
  return _node_to_grn_weight_map;
}

void
EBSDReader::buildNodeToGrainWeightMap()
{
  // Import nodeToElemMap from MooseMesh for current node
  // This map consists of the node index followed by a vector of element indices that are associated with that node
  std::map<dof_id_type, std::vector<dof_id_type> > & node_to_elem_map = _mesh.nodeToElemMap();
  libMesh::MeshBase &mesh = _mesh.getMesh();

  // Loop through each node in mesh and calculate eta values for each grain associated with the node
  MeshBase::const_node_iterator ni = mesh.nodes_begin();
  const MeshBase::const_node_iterator nend = mesh.nodes_end();
  for (; ni != nend; ++ni)
  {
    // Get node_id
    const dof_id_type node_id = (*ni)->id();

    // Initialize node_to_grn_weight_map
    _node_to_grn_weight_map[node_id].resize(_feature_num, 0);

    // Loop through element indices associated with the current node and record weighted eta value in new map
    unsigned int n_elems = node_to_elem_map[node_id].size();  // n_elems can range from 1 to 4 for 2D and 1 to 8 for 3D problems

    for (unsigned int ne = 0; ne < n_elems; ++ne)
    {
      // Current element index
      unsigned int elem_id = node_to_elem_map[node_id][ne];

      // Retrieve EBSD grain number for the current element index
      unsigned int grain_id;
      const Elem * elem = mesh.elem(elem_id);
      const EBSDReader::EBSDPointData & d = getData(elem->centroid());
      grain_id = d.grain;

      // Calculate eta value and add to map
      _node_to_grn_weight_map[node_id][grain_id] += 1.0 / n_elems;
    }
  }
}
