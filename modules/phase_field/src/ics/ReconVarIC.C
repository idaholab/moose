#include "ReconVarIC.h"
#include "MooseMesh.h"

template<>
InputParameters validParams<ReconVarIC>()
{

  InputParameters params = validParams<InitialCondition>();
  params.addRequiredParam<UserObjectName>("ebsd_reader", "The EBSDReader GeneralUserObject");
  params.addRequiredParam<bool>("consider_phase","If true, IC will only only deal with grain values and ignore phase");
  params.addParam<unsigned int>("phase", 0,"EBSD phase number to be assigned to this grain");
  params.addParam<bool>("all_to_one",false,"If true, assign all grain numbers in this phase to the same variable");
  params.addParam<unsigned int>("op_num", 0, "Specifies the number of order paraameters to create, all_to_one = false");
  params.addParam<unsigned int>("op_index", 0,"The index for the current order parameter, if all_to_one = false");
  return params;
}

ReconVarIC::ReconVarIC(const std::string & name,InputParameters parameters) :
    InitialCondition(name, parameters),
    _mesh(_fe_problem.mesh()),
    _nl(_fe_problem.getNonlinearSystem()),
    _ebsd_reader(getUserObject<EBSDReader>("ebsd_reader")),
    _consider_phase(getParam<bool>("consider_phase")),
    _phase(getParam<unsigned int>("phase")),
    _all_to_one(getParam<bool>("all_to_one")),
    _op_num(getParam<unsigned int>("op_num")),
    _op_index(getParam<unsigned int>("op_index"))
{
  if (!_consider_phase && _all_to_one)
    mooseError("In ReconVarIC, if you are not considering phase, you can't assign all grains to one variable");
}

void
ReconVarIC::initialSetup()
{
  //Get the number of grains from the EBSD data, reduce by one if consider phase becuase grain numbering starts at one in phase 1
  _grain_num = _ebsd_reader.getGrainNum();
  if (_consider_phase)
    _grain_num -= 1;

  if (!_all_to_one)
  {
    // Output error message
    if (_op_num > _grain_num)
      mooseError("ERROR in PolycrystalReducedIC: Number of order parameters (op_num) can't be larger than the number of grains (grain_num)");

    //Because the grains start at 1 in phase 1
    unsigned int grn_index_offset = 0;
    if (_consider_phase)
      grn_index_offset = 1;

    //Assign center point values
    _centerpoints.resize(_grain_num);

    for (unsigned int gr = 0; gr < _grain_num; gr++)
    {
      const EBSDReader::EBSDAvgData & d = _ebsd_reader.getAvgData(gr + grn_index_offset);
      _centerpoints[gr] = d.p;
    }

    // Assign grains to each order parameter in a way that maximizes distance
    _assigned_op.resize(_grain_num);

    _assigned_op = PolycrystalICTools::assignPointsToVariables(_centerpoints,_op_num, _mesh, _var);

  }
}


// Note that we are not actually using Point coordinates that get passed in to assign the order parameter.
// By knowing the curent elements index, we can use it's centroid to grab the EBSD grain index
// associated with the point from the EBSDReader user object.
Real
ReconVarIC::value(const Point &)
{
  const Point p1 = _current_elem->centroid();
  const EBSDReader::EBSDPointData & d = _ebsd_reader.getData(p1);
  const unsigned int grn_index = d.grain;
  const unsigned int phase_index = d.phase;

  if (!_consider_phase) //This is the old methodology where everything is an order parameter
  {
    if (_assigned_op[grn_index] == _op_index)
      return 1.0;
    else
      return 0.0;
  }
  else
  {
    if (_all_to_one) //Ever part of this phase will be assigned to the same variable
    {
      if (phase_index == _phase)
        return 1.0;
      else
        return 0.0;
    }
    else //For the given phase, each grain is assigned to the corresponding op.
    {
      if (_assigned_op[grn_index] == _op_index && phase_index == _phase)
        return 1.0;
      else
        return 0.0;
    }
  }
}
