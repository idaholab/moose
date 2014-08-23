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
          if ((_nx == 0) || (_ny == 0))
            mooseError("Error reading header, one of X_Dim or Y_Dim was zero.");

          // Must also have nonzero stepsizes
          if ((_dx == 0.) || (_dy == 0.))
            mooseError("Error reading header, one of X_Step or Y_Step was zero.");

          // Compute the total size.  If this is 2D data, don't multiply by zero.
          unsigned total_size = _nz==0 ? _nx*_ny : _nx*_ny*_nz;

          // Resize the _data array
          _data.resize(total_size);

          // Resize each entry of the _data array to 10, which is the number of values
          // we expect for each centroid.
          for (unsigned i=0; i<_data.size(); ++i)
            _data[i].resize(10);
        }

        // Make sure we have processed the header and allocated space to start saving values.
        mooseAssert(_data.size() > 0, "Error, _data vector not properly resized!");

        // Temporary variables to read in on each line
        Real phi1, phi, phi2, x, y, z;
        unsigned int grain, phase, sym;

        std::istringstream iss(line);
        iss >> phi1 >> phi >> phi2 >> x >> y >> z >> grain >> phase >> sym;

        unsigned global_index = indexFromPoint(Point(x, y, z));

        std::vector<Real> & centroid_data = _data[global_index];

        centroid_data[0] = phi1;
        centroid_data[1] = phi;
        centroid_data[2] = phi2;
        centroid_data[3] = x;
        centroid_data[4] = y;
        centroid_data[5] = z;
        centroid_data[6] = grain;
        centroid_data[7] = phase;
        centroid_data[8] = sym;
        centroid_data[9] = 0;

        _phi1_ic.push_back(phi1);
        _PHI_ic.push_back(phi);
        _phi2_ic.push_back(phi2);
        _x_ic.push_back(x);
        _y_ic.push_back(y);
        _z_ic.push_back(z);
        _grn_ic.push_back(grain);
        _phase_ic.push_back(phase);
        _sym_ic.push_back(sym);
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

  // Calculate average initial values of phi1, PHI, phi2, phase, and sym for each grain imported into the simulation.
  // Also Calculate centerpoint of each EBSD grain.
  {
    // Resize the variables
    _avg_data.resize(_grain_num);
    for (unsigned i=0; i<_avg_data.size(); ++i)
    {
      _avg_data[i].resize(8);
    }

    _avg_phi1.resize(_grain_num);
    _avg_PHI.resize(_grain_num);
    _avg_phi2.resize(_grain_num);
    _avg_x.resize(_grain_num);
    _avg_y.resize(_grain_num);
    _avg_z.resize(_grain_num);
    _avg_phase.resize(_grain_num);
    _avg_sym.resize(_grain_num);
    _centerpoints.resize(_grain_num);

    // Iterate through data points to get average variable values for each grain
    unsigned int num_pts;
    for (unsigned int i = 0; i < _grain_num; ++i)
    {
      num_pts = 0;
      for (unsigned int j = 0; j < _grn_ic.size(); ++j)
      {
        if (_grn_ic[j] == i)
        {
          _avg_phi1[i] += _phi1_ic[j];
          _avg_PHI[i] += _PHI_ic[j];
          _avg_phi2[i] += _phi2_ic[j];
          _avg_x[i] += _x_ic[j];
          _avg_y[i] += _y_ic[j];
          _avg_z[i] += _z_ic[j];
          _avg_phase[i] = _phase_ic[j];
          _avg_sym[i] = _sym_ic[j];
          num_pts += 1;
        }
      }
      _avg_phi1[i] = _avg_phi1[i] / num_pts;
      _avg_PHI[i] = _avg_PHI[i] / num_pts;
      _avg_phi2[i] = _avg_phi2[i] / num_pts;
      _avg_x[i] = _avg_x[i] / num_pts;
      _avg_y[i] = _avg_y[i] / num_pts;
      _avg_z[i] = _avg_z[i] / num_pts;
      _avg_phase[i] = _avg_phase[i];
      _avg_sym[i] = _avg_sym[i];

      // Save centerpoints of each grain
      _centerpoints[i](0) = _avg_x[i];
      _centerpoints[i](1) = _avg_y[i];
      _centerpoints[i](2) = _avg_z[i];

      // Save averaged values to array
      unsigned int index = i;
      std::vector<Real> & averaged_data = _avg_data[index];
      averaged_data[0] = _avg_phi1[i];
      averaged_data[1] = _avg_PHI[i];
      averaged_data[2] = _avg_phi2[i];
      averaged_data[3] = _avg_x[i];
      averaged_data[4] = _avg_y[i];
      averaged_data[5] = _avg_z[i];
      averaged_data[6] = _avg_phase[i];
      averaged_data[7] = _avg_sym[i];
    }

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
  }
  // Update centroid_data[9] with assigned order parameter value
  for (unsigned int k = 0; k < _data.size(); ++k)
  {
    std::vector<Real> & centroid_data = _data[k];
    unsigned int index1 = centroid_data[6];
    centroid_data[9] = _assigned_op[index1];
  }
}

EBSDReader::~EBSDReader()
{
}

Real
EBSDReader::getData(const Point & p, MooseEnum data_type) const
{
  // TODO: If we always keep these in order, we can just return
  // _data[indexFromPoint(p)][data_type]...

  switch (data_type)
  {
    case PHI1:
      // phi1 is entry [0] at each centroid
      return _data[indexFromPoint(p)][0];

    case PHI:
      // phi is entry [1] at each centroid
      return _data[indexFromPoint(p)][1];

    case PHI2:
      // phi2 is entry [2] at each centroid
      return _data[indexFromPoint(p)][2];

    case X:
      // x is entry [3] at each centroid
      return _data[indexFromPoint(p)][3];

    case Y:
      // y is entry [4] at each centroid
      return _data[indexFromPoint(p)][4];

    case Z:
      // z is entry [5] at each centroid
      return _data[indexFromPoint(p)][5];

    case GRAIN:
      // grain is entry [6] at each centroid
      return _data[indexFromPoint(p)][6];

    case PHASE:
      // grain is entry [7] at each centroid
      return _data[indexFromPoint(p)][7];

    case SYMMETRY:
      // symmetry is entry [8] at each centroid
      return _data[indexFromPoint(p)][8];

    case OP:
      // OP is entry [9] at each centroid
      return _data[indexFromPoint(p)][9];
  }
  mooseError("Invalid DataType " << data_type << " requested.");
}

Real
EBSDReader::getAvgData(const unsigned int & var, MooseEnum data_type) const
{
  // TODO: If we always keep these in order, we can just return
  // _avg_data[indexFromIndex(var)][data_type]...

  switch (data_type)
  {
  case AVG_PHI1:
      // avg_phi1 is entry [0] in _avg_data array
      return _avg_data[indexFromIndex(var)][0];

    case AVG_PHI:
      // avg_phi is entry [1] in _avg_data array
      return _avg_data[indexFromIndex(var)][1];

    case AVG_PHI2:
      // avg_phi2 is entry [2] in _avg_data array
      return _avg_data[indexFromIndex(var)][2];

    case AVG_X:
      // avg_x is entry [3] in _avg_data array
      return _avg_data[indexFromIndex(var)][3];

    case AVG_Y:
      // avg_y is entry [4] in _avg_data array
      return _avg_data[indexFromIndex(var)][4];

    case AVG_Z:
      // avg_x is entry [5] in _avg_data array
      return _avg_data[indexFromIndex(var)][5];

    case AVG_PHASE:
      // avg_phase is entry [6] in _avg_data array
      return _avg_data[indexFromIndex(var)][6];

    case AVG_SYMMETRY:
      // avg_symmetry is entry [7] in _avg_data array
      return _avg_data[indexFromIndex(var)][7];
  }

  mooseError("Invalid DataType " << data_type << " requested.");
}

unsigned EBSDReader::indexFromPoint(const Point & p) const
{
  // Don't assume an ordering on the input data, use the (x, y,
  // z) values of this centroid to determine the index.
  unsigned x_index = static_cast<unsigned>(p(0)/_dx);
  unsigned y_index = static_cast<unsigned>(p(1)/_dy);
  unsigned z_index = static_cast<unsigned>(p(2)/_dz);

  // Compute the global index into the _data array.  This stores points
  // in a [z][y][x] ordering.
  unsigned global_index = (z_index*_ny + y_index)*_nx + x_index;

  // Don't access out of range!
  if (global_index >= _data.size())
    mooseError("Error! Index out of range in EBSDReader::indexFromPoint().");

  return global_index;
}

unsigned EBSDReader::indexFromIndex(const unsigned int & var) const
{

  // Transfer the index into the _avg_data array.
  unsigned avg_index = var;

  // Don't access out of range!
  if (avg_index >= _avg_data.size())
    mooseError("Error! Index out of range in EBSDReader::indexFromIndex()");

  return avg_index;
}
