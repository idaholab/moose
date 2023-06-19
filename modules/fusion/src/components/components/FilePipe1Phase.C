#include "FilePipe1Phase.h"
#include "THMMesh.h"
#include "DelimitedFileReader.h"

registerMooseObject("MooseApp", FilePipe1Phase);

InputParameters
FilePipe1Phase::validParams()
{
  InputParameters params = FlowChannel1Phase::validParams();
  params.addRequiredParam<FileName>("csv_file",
                                    "The name of the CSV file to read. Currently, with "
                                    "the exception of the header row, only numeric "
                                    "values are supported.");
  // Suppress length. Also need to set it to something, because it is required in the parent class
  params.set<std::vector<Real>>("length") = {0.0};
  params.suppressParameter<std::vector<Real>>("length");
  // Suppress these parameters. We can gather these information from files
  params.set<Point>("position") = Point(0., 0., 0.);
  params.suppressParameter<Point>("position");
  params.set<RealVectorValue>("orientation") = {1., 0., 0.};
  params.suppressParameter<RealVectorValue>("orientation");
  params.set<std::vector<unsigned int>>("n_elems") = {1};
  params.suppressParameter<std::vector<unsigned int>>("n_elems");

  params.addClassDescription(
      "This class constructs a flow pipe by reading in mesh from a CSV file. "
      "The flow pipe can be curved or straight.");

  return params;
}

FilePipe1Phase::FilePipe1Phase(const InputParameters & params) : FlowChannel1Phase(params)
{
  /// The MOOSE delimited file reader.
  MooseUtils::DelimitedFileReader csv_reader(getParam<FileName>("csv_file"), &_communicator);
  csv_reader.setIgnoreEmptyLines(false);
  csv_reader.setHeaderFlag(MooseUtils::DelimitedFileReader::HeaderFlag::ON);
  csv_reader.setDelimiter(",");
  csv_reader.read();
  const std::vector<std::string> & names = csv_reader.getNames();
  const std::vector<std::vector<double>> & data = csv_reader.getData();
  if (names.size() != 3 || data.size() != 3)
    mooseError("Support only 3D coordinates");

  _x.reserve(data[0].size());
  std::copy(data[0].begin(), data[0].end(), back_inserter(_x));
  _y.reserve(data[1].size());
  std::copy(data[1].begin(), data[1].end(), back_inserter(_y));
  _z.reserve(data[2].size());
  std::copy(data[2].begin(), data[2].end(), back_inserter(_z));

  if (_x.size() != _y.size() || _x.size() != _z.size())
    mooseError("X coordinate dimensions (",
               _x.size(),
               ", ",
               _y.size(),
               ", ",
               _z.size(),
               ") does not match ");

  if (_x.size() < 2)
    mooseError("At least two nodes are required");

  // Number of nodes
  auto n_nodes = _x.size();

  // Distance
  _length = 0;
  for (unsigned int i = 1; i < n_nodes; i++)
  {
    auto dist = Point(_x[i], _y[i], _z[i]) - Point(_x[i - 1], _y[i - 1], _z[i - 1]);
    _length += dist.norm();
  }

  _lengths[0] = _length;

  _n_elem = n_nodes - 1;
  _n_elems[0] = _n_elem;
}

void
FilePipe1Phase::buildMeshNodes()
{
  for (unsigned int i = 0; i < _node_locations.size(); i++)
  {
    RealVectorValue p(_x[i], _y[i], _z[i]);
    addNode(p);
  }
}
