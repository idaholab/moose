#include "EBSDReader.h"

template<>
InputParameters validParams<EBSDReader>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addRequiredParam<FileName>("filename", "The name of the file containing the EBSD data");
  return params;
}

EBSDReader::EBSDReader(const std::string & name, InputParameters params) :
    GeneralUserObject(name, params),
    _filename(getParam<FileName>("filename")),
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

      // Process lines that don't start with a comment character (data lines)
      if (line.find("#") != 0)
      {
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

          // Resize each entry of the _data array to 9, which is the number of values
          // we expect for each centroid.
          for (unsigned i=0; i<_data.size(); ++i)
            _data[i].resize(9);
        }

        // Make sure we have processed the header and allocated space to start saving values.
        mooseAssert(_data.size() > 0, "Error, _data vector not properly resized!");

        // Temporary variables to read in on each line
        Real phi1, phi, phi2, x, y, z;
        unsigned int grain, phase, sym;

        std::istringstream iss(line);
        iss >> phi1 >> phi >> phi2 >> x >> y >> z >> grain >> phase >> sym;

        unsigned global_index = this->index_from_point( Point(x,y,z) );

        // Moose::out << "Computed global index " << global_index << std::endl;

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
}

EBSDReader::~EBSDReader()
{
}

Real
EBSDReader::get_data(const Point & p, MooseEnum data_type) const
{
  // TODO: If we always keep these in order, we can just return
  // _data[index_from_point(p)][data_type]...

  switch (data_type)
  {
    case PHI1:
      // phi1 is entry [0] at each centroid
      return _data[index_from_point(p)][0];

    case PHI:
      // phi is entry [1] at each centroid
      return _data[index_from_point(p)][1];

    case PHI2:
      // phi2 is entry [2] at each centroid
      return _data[index_from_point(p)][2];

    case X:
      // x is entry [3] at each centroid
      return _data[index_from_point(p)][3];

    case Y:
      // y is entry [4] at each centroid
      return _data[index_from_point(p)][4];

    case Z:
      // z is entry [5] at each centroid
      return _data[index_from_point(p)][5];

    case GRAIN:
      // grain is entry [6] at each centroid
      return _data[index_from_point(p)][6];

    case PHASE:
      // grain is entry [7] at each centroid
      return _data[index_from_point(p)][7];

    case SYMMETRY_CLASS:
      // symmetry is entry [8] at each centroid
      return _data[index_from_point(p)][8];
  }

  mooseError("Invalid DataType " << data_type << " requested.");
}

unsigned EBSDReader::index_from_point(const Point& p) const
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
    mooseError("Error! Index out of range in EBSDReader::index_from_point().");

  return global_index;
}
