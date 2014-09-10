#include "EBSDReader.h"
#include "EBSDMesh.h"

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
    _grain_num(0),
    _mesh_dimension(_mesh.dimension()),
    _nx(0),
    _ny(0),
    _nz(0),
    _dx(0.),
    _dy(0.),
    _dz(0.)
{
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

  _dy = g.d[1];
  _ny = g.n[1];

  _dz = g.d[2];
  _nz = g.n[2];

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
      d.p = Point(x,y,z);

      // determine number of grains in the dataset
      if (d.grain > _grain_num) _grain_num = d.grain;

      // The Order parameter is not yet assigned.
      // We initialize it to zero in order not to have undefined values that break the testing.
      d.op = 0;

      unsigned global_index = indexFromPoint(Point(x, y, z));
      _data[global_index] = d;
    }
  }
  stream_in.close();

  // total number of grains is one higher than the maximum grain id
  _grain_num += 1;

  // Resize the variables
  _avg_data.resize(_grain_num);

  // clear the averages
  for (unsigned int i = 0; i < _grain_num; ++i)
  {
    EBSDAvgData & a = _avg_data[i];
    a.p = a.phi1 = a.phi = a.phi2 = a.phase = a.symmetry = 0.0;
    a.n = 0;
  }
  //_centerpoints.resize(_grain_num);

  // Iterate through data points to get average variable values for each grain
  for (std::vector<EBSDPointData>::iterator j = _data.begin(); j != _data.end(); ++j)
  {
    EBSDAvgData & a = _avg_data[j->grain];

    a.phi1  += j->phi1;
    a.phi   += j->phi;
    a.phi2  += j->phi2;
    a.phase += j->phase;
    a.symmetry += j->symmetry;
    a.p     += j->p;
    a.n++;
  }

  for (unsigned int i = 0; i < _grain_num; ++i)
  {
    EBSDAvgData & a = _avg_data[i];

    if (a.n == 0) continue;

    a.phi1  /=  Real(a.n);
    a.phi   /=  Real(a.n);
    a.phi2  /=  Real(a.n);
    a.phase /= Real(a.n);
    a.symmetry /= Real(a.n);
    a.p *= 1.0/Real(a.n);

    // Save centerpoints of each grain
    //_centerpoints[i](0) = _avg_x[i];
    //_centerpoints[i](1) = _avg_y[i];
    //_centerpoints[i](2) = _avg_z[i];
  }

// Duplicated code!!!
#if 0
  // Set up domain bounds with mesh tools
  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
  {
    _bottom_left(i) = _mesh.getMinInDimension(i);
    _top_right(i) = _mesh.getMaxInDimension(i);
  }
  _range = _top_right - _bottom_left;

  // Output error message if number of order parameters is larger than number of grains from EBSD dataset
  if (_op_num > _grain_num)
    mooseError("ERROR in PolycrystalReducedIC: Number of order parameters (op_num) can't be larger than the number of grains (grain_num)");

  // Assign grains to each order parameter in a way that maximizes distance
  _assigned_op.resize(_grain_num);
  for (unsigned int grain=0; grain < _grain_num; ++grain)
  {
    std::vector<int> min_op_ind;
    std::vector<Real> min_op_dist;
    min_op_ind.resize(_op_num);
    min_op_dist.resize(_op_num);

    // Determine the distance to the closest center assigned to each order parameter
    if (grain >= _op_num)
    {
      std::fill(min_op_dist.begin() , min_op_dist.end(), _range.size());
      for (unsigned int i=0; i<grain; ++i)
      {
        Real dist =  _mesh.minPeriodicDistance(grain, _centerpoints[grain], _centerpoints[i]);
        if (min_op_dist[_assigned_op[i]] > dist)
        {
          min_op_dist[_assigned_op[i]] = dist;
          min_op_ind[_assigned_op[i]] = i;
        }
      }
    }

    // Assign the current center point to the order parameter that is furthest away.
    Real mx;
    if (grain < _op_num)
      _assigned_op[grain] = grain;
    else
    {
      mx = 0.0;
      unsigned int mx_ind = 1e6;
      for (unsigned int i = 0; i < _op_num; ++i) // Find index of max
        if (mx < min_op_dist[i])
        {
          mx = min_op_dist[i];
          mx_ind = i;
        }
      _assigned_op[grain] = mx_ind;
    }
  }

  // Update centroid_data[9] with assigned order parameter value
  for (unsigned int k = 0; k < _data.size(); ++k)
  {
    std::vector<Real> & centroid_data = _data[k];
    unsigned int index1 = centroid_data[6];
    centroid_data[9] = _assigned_op[index1];
  }
#endif
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

unsigned int
EBSDReader::indexFromPoint(const Point & p) const
{
  // Don't assume an ordering on the input data, use the (x, y,
  // z) values of this centroid to determine the index.
  unsigned int x_index, y_index, z_index, global_index;

  x_index = (unsigned int)(p(0) / _dx);
  y_index = (unsigned int)(p(1) / _dy);

  if (_mesh_dimension == 3)
  {
    z_index = (unsigned int)(p(2) / _dz);
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
