#include "EBSDReader.h"
#include "MooseMesh.h"

template<>
InputParameters validParams<EBSDReader>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addRequiredParam<FileName>("filename", "The name of the file containing the EBSD data");
  params.addRequiredParam<unsigned int>("crys_num", "Specifies the number of order paraameters to create");
  params.addRequiredParam<unsigned int>("grain_num", "Specifies the number of grains in the reconstructed dataset");
  return params;
}

EBSDReader::EBSDReader(const std::string & name, InputParameters params) :
    GeneralUserObject(name, params),
    _mesh(_fe_problem.mesh()),
    _nl(_fe_problem.getNonlinearSystem()),
    _filename(getParam<FileName>("filename")),
    _op_num(getParam<unsigned int>("crys_num")),
    _grain_num(getParam<unsigned int>("grain_num")),
    _mesh_dimension(_mesh.dimension()),
    _nx(0),
    _ny(0),
    _nz(0),
    _dx(0.),
    _dy(0.),
    _dz(0.)
{
  std::ifstream stream_in(_filename.c_str());

  if (!stream_in)
    mooseError("Can't open EBSD file: " << _filename);

  // Labels to look for in the header
  std::vector<std::string> labels;
  labels.push_back("X_step");
  labels.push_back("X_Dim");
  labels.push_back("Y_step");
  labels.push_back("Y_Dim");
  labels.push_back("Z_step");
  labels.push_back("Z_Dim");

  // Dimension variables to store once they are found in the header
  // X_step, X_Dim, Y_step, Y_Dim, Z_step, Z_Dim
  // We use Reals even though the Dim values should all be integers...
  std::vector<Real> label_vals(labels.size());

  // Set to true once we reach the first data (non-comment) line.  We must
  // have parsed the entire header successfully before this happens
  bool header_parsed = false;

  std::string line;
  while (true)
  {
    // Try to read something
    std::getline(stream_in, line);

    if (stream_in)
    {
      // We need to process the comment lines that have:
      // X_step, X_Dim
      // Y_step, Y_Dim
      // Z_step, Z_Dim
      // in them.
      // Note that for 2D data, Z_Dim will be zero, but that's OK because Z_Dim isn't
      // required to do the indexing we're going to do...
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
      {
        // Process lines that don't start with a comment character (data lines)

        // Make sure we have successfully parsed the header
        if (!header_parsed)
        {
          header_parsed = true;

          // Copy stuff out of the label_vars array into class variables
          _dx = label_vals[0];
          _nx = label_vals[1];

          _dy = label_vals[2];
          _ny = label_vals[3];

          _dz = label_vals[4];
          _nz = label_vals[5];

          // Must have at least 2D data
          if (_nx == 0 || _ny == 0 || (_nz == 0 && _mesh_dimension > 2))
            mooseError("Error reading header, spatial dimension of the EBSD data is lower than the dimension of the mesh.");

          // Must also have nonzero stepsizes
          if (_dx == 0.0 || _dy == 0.0 || (_dz == 0.0 && _mesh_dimension > 2))
            mooseError("Error reading header, EBSD data step size is zero.");

          // Compute the total size.  If this is 2D data, don't multiply by zero.
          unsigned total_size = _mesh_dimension < 3 ? _nx*_ny : _nx*_ny*_nz;

          // Resize the _data array
          _data.resize(total_size);
        }

        // Make sure we have processed the header and allocated space to start saving values.
        mooseAssert(_data.size() > 0, "Error, _data vector not properly resized!");

        // Temporary variables to read in on each line
        EBSDPointData d;
        Real x, y, z;

        std::istringstream iss(line);
        iss >> d.phi1 >> d.phi >> d.phi2 >> x >> y >> z >> d.grain >> d.phase >> d.symmetry;
        d.p = Point(x,y,z);

        // The Order parameter is not yet assigned. We initialize it to zero in order not to have undefined values that break the testing.
        d.op = 0;

        unsigned global_index = indexFromPoint(Point(x, y, z));
        _data[global_index] = d;
      }

      // Go to next line of file, skip the rest of the error checking code below
      continue;
    }

    if (stream_in.eof())
      break;

    // If we made it here, then !file AND !file.eof, therefore the
    // stream is "bad".
    mooseError("Stream is bad!");
  }
  stream_in.close();


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
    mooseError("ERROR in PolycrystalReducedIC: Number of order parameters (crys_num) can't be larger than the number of grains (grain_num)");

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
