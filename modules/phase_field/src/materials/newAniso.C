#include "newAniso.h"
#include "MathUtils.h"

#include <array>

registerMooseObject("MooseApp", AnisoGBEnergyMaterial);

template <>
InputParameters
validParams<AnisoGBEnergyMaterial>()
{
  InputParameters params = validParams<DerivativeFunctionMaterialBase>();
  params.addClassDescription("Calculates the Grain Boundary energy using Bulatov's method");
  params.addRequiredCoupledVarWithAutoBuild(
      "v", "var_name_base", "op_num", "Array of coupled variables");
  params.addRequiredParam<UserObjectName>("euler_angle_provider",
                                          "Name of the euler angle provider user object");
  params.addRequiredParam<Real>("gb_energy_isotropic", "The average grain boundary energy");
  params.addRequiredParam<UserObjectName>("grain_tracker",
                                          "The GrainTracker UserObject to get values from.");
  return params;
}

AnisoGBEnergyMaterial::AnisoGBEnergyMaterial(const InputParameters & parameters)
  : DerivativeFunctionMaterialBase(parameters),
    _op_num(coupledComponents("v")),
    _gb_energy_isotropic(getParam<Real>("gb_energy_isotropic")),
    _euler(getUserObject<EulerAngleProvider>("euler_angle_provider")),
    //_par_vec(getMaterialProperty<std::vector<Real> >("par_vec")),
    _par_vec(43),
    _geom100(4),
    _geom110(4),
    _geom111(4),
    _axes100(3),
    _dirs100(3),
    _axes110(6),
    _dirs110(6),
    _axes111(4),
    _dirs111(4),
    _rot_X_p90(RotationTensor::XAXIS, 90.0),
    _rot_Y_p90(RotationTensor::YAXIS, 90.0),
    _rot_Z_p90(RotationTensor::ZAXIS, 90.0),
    _rot_Z_n90(RotationTensor::ZAXIS, -90.0),
    _symmetry_variants(24),
    _epsilon(1e-6)
{
  for (unsigned int i = 0; i < _op_num; ++i)
    _orientation_matrix[i] = RotationTensor(_euler.getEulerAngles(i));

  // define 100 axes, normalize
  _axes100[0] = Point(1, 0, 0);
  _axes100[1] = Point(0, 1, 0);
  _axes100[2] = Point(0, 0, 1);

  // define crystal direction
  _dirs100[0] = Point(0, 1, 0);
  _dirs100[1] = Point(0, 0, 1);
  _dirs100[2] = Point(1, 0, 0);

  Real s2 = 1.0 / std::sqrt(2.0);

  // define the 110 axes, normalize
  _axes110[0] = Point(s2, s2, 0);
  _axes110[1] = Point(s2, -s2, 0);
  _axes110[2] = Point(s2, 0, s2);
  _axes110[3] = Point(s2, 0, -s2);
  _axes110[4] = Point(0, s2, s2);
  _axes110[5] = Point(0, s2, -s2);

  // define the crystal direction perpendicular to each rotation axis.
  // The formalism demands that this be an axis of at least two-fold symmetry.
  _dirs110[0] = Point(0, 0, 1);
  _dirs110[1] = Point(0, 0, 1);
  _dirs110[2] = Point(0, 1, 0);
  _dirs110[3] = Point(0, 1, 0);
  _dirs110[4] = Point(1, 0, 0);
  _dirs110[5] = Point(1, 0, 0);

  Real s3 = 1.0 / std::sqrt(3);
  s2 = 1.0 / std::sqrt(2);

  // define the 111 axes,  normalize.
  _axes111[0] = Point(s3, s3, s3);
  _axes111[1] = Point(s3, -s3, -s3);
  _axes111[2] = Point(-s3, s3, -s3);
  _axes111[3] = Point(-s3, -s3, s3);

  // define crystal direction
  _dirs111[0] = Point(s2, -s2, 0);
  _dirs111[1] = Point(s2, s2, 0);
  _dirs111[2] = Point(s2, s2, 0);
  _dirs111[3] = Point(s2, -s2, 0);

  _v.resize(_op_num);
  _grad_v.resize(_op_num);
  _orientation_matrix.resize(_op_num);

  for (unsigned int op = 0; op < _op_num; ++op)
  {
    _v[op] = &coupledValue("v", op);
    _grad_v[op] = &coupledGradient("v", op);
  }
}

Real
AnisoGBEnergyMaterial::computeF()
{
  const RealVectorValue axis100(1, 0, 0);
  RealTensorValue rotation_matrix; // rotates gbdir to [100]
  RealVectorValue gbdir;           // the normal to the grain boundary
  Real GBenTot = 0;
  for (unsigned int i = 0; i < _op_num; ++i)
  {
    for (unsigned int j = i + 1; j < _op_num; ++j)
    {
      if(j != i)
      {
        // this calculates the gradient normal based on the qp value
        const Real grad_norm = (*_grad_v[i])[_qp].norm();

        // find the gb normal from the gradient of the order parameter
        gbdir = (*_grad_v[i])[_qp] / grad_norm;

        rotation_matrix = RotationMatrix::rotVec1ToVec2(gbdir, axis100);

        // Use Bulatov's function where the grains are in the frame where the GB norm is (100)
        GBenTot += gB5DOF(rotation_matrix * _orientation_matrix[i],
                         rotation_matrix * _orientation_matrix[j]);
      }
    }
  }
  return GBenTot;
}

/////////
//THIS WILL RETURN A VECTOR WHEN WE ARE DONE
/////////
Real
AnisoGBEnergyMaterial::computeDF(unsigned int j_var)
{
  const RealVectorValue axis100(1, 0, 0);
  RealTensorValue rotation_matrix; // rotates gbdir to [100]
  RealVectorValue gbdir;           // the normal to the grain boundary

  ///////
  //THIS WILL BE A VECTOR WHEN WE'RE DONE
  //////
  Real GBenTot = 0;


  for (unsigned int i = 0; i < _op_num; ++i)
  {
    for (unsigned int j = i + 1; j < _op_num; ++j)
    {
      if(j != i && (j == j_var || i == j_var))
      {
        // this calculates the gradient normal based on the qp value
        const Real grad_norm = (*_grad_v[i])[_qp].norm();

        // find the gb normal from the gradient of the order parameter
        gbdir = (*_grad_v[i])[_qp] / grad_norm;

        rotation_matrix = RotationMatrix::rotVec1ToVec2(gbdir, axis100);

        // Use Bulatov's function where the grains are in the frame where the GB norm is (100)
        GBenTot += gB5DOF(rotation_matrix * _orientation_matrix[i],
                          rotation_matrix * _orientation_matrix[j],
                          "eta");// * _deriv_geom[i];
        GBenTot += gB5DOF(rotation_matrix * _orientation_matrix[i],
                          rotation_matrix * _orientation_matrix[i],
                          "ksii");// * _deriv_geom[i];
        GBenTot += gB5DOF(rotation_matrix * _orientation_matrix[i],
                          rotation_matrix * _orientation_matrix[i],
                          "phi");// * _deriv_geom[i];
      }
    }
  }
  return GBenTot;
}

Real
AnisoGBEnergyMaterial::gB5DOF(RealTensorValue P, RealTensorValue S, std::string whichComponent)
{
  // P and S are the rotation Matrices for the two grains
  distancesToSet(P, S, _geom100, _axes100, _dirs100, whichComponent);
  distancesToSet(P, S, _geom110, _axes110, _dirs110, whichComponent);
  distancesToSet(P, S, _geom111, _axes111, _dirs111, whichComponent);

  return weightedMeanEnergy(whichComponent);
}
/////////////////////
//  FOR NOW, THIS IS ONLY VALID FOR NONDERIVATIVE TAKING
//  FOR NOW, THIS IS ONLY VALID FOR NONDERIVATIVE TAKING
//  FOR NOW, THIS IS ONLY VALID FOR NONDERIVATIVE TAKING
//  FOR NOW, THIS IS ONLY VALID FOR NONDERIVATIVE TAKING
//  FOR NOW, THIS IS ONLY VALID FOR NONDERIVATIVE TAKING
/////////////////////
void
AnisoGBEnergyMaterial::distancesToSet(const RealTensorValue & P,
                                      RealTensorValue & S,
                                      std::vector<std::vector<Real>> & geom,
                                      const std::vector<RealVectorValue> & axes,
                                      const std::vector<RealVectorValue> & dirs,
                                      std::string whichComponent)
{
  unsigned int naxes = axes.size();

  // force the distance to be strictly less than one, allowing for roundoff
  const Real dismax = 1.0 - _epsilon;

  // determine the number of axes for a given rotation axis
  Real period = libMesh::pi * naxes / 6;

  /**
   * Create 24 symmetry equivalent variants of S
   * This is the coset appropriate for the rotation convention where S*P
   * is the misorientation represented in the grain frame.  If you're
   * getting odd results, e.g. misorientations that you know are CSL are
   * coming out entirely wrong, you may be using the opposite convention;
   * try replacing P and S with P' and S'.
   */
  _symmetry_variants[0] = S;

  RealTensorValue Temp = S;
  for (unsigned int i = 1; i < 4; ++i) // Rotate three times around X by +90 degrees
  {
    Temp = Temp * _rot_X_p90;
    _symmetry_variants[i] = Temp;
  }
  for (unsigned int i = 4; i < 16; ++i) // Rotate three times around Y by +90 degrees
  {
    Temp = _symmetry_variants[i - 4];
    Temp = Temp * _rot_Y_p90;
    _symmetry_variants[i] = Temp;
  }
  for (unsigned int i = 16; i < 20; ++i) // Rotate around Z by +90 degrees
  {
    Temp = _symmetry_variants[i - 16];
    Temp = Temp * _rot_Z_p90;
    _symmetry_variants[i] = Temp;
  }
  for (unsigned int i = 20; i < 24; ++i) // Rotate around Z by -90 degrees
  {
    Temp = _symmetry_variants[i - 20];
    Temp = Temp * _rot_Z_n90;
    _symmetry_variants[i] = Temp;
  }

  // Preallocate all parameter lists at their maximum possible sizes
  // Redundant representations will be removed at the end
  geom[0].resize(24 * naxes); // distances
  geom[1].resize(24 * naxes); // ksis
  geom[2].resize(24 * naxes); // etas
  geom[3].resize(24 * naxes); // phis

  // Number of hits found so far
  unsigned int thisindex = 0;

  // Step through all combinations of symmetrically-equivalent axes and coset elements
  // _symmetry_variants
  for (unsigned int i = 0; i < naxes; ++i)
  {
    // Completing the orthonormal coordinate set.
    // theta1 and theta2 are defined in the plane spanned by (dir,dir2)
    RealVectorValue dir2 = axes[i].cross(dirs[i]);

    RealTensorValue R;

    // for each symmetry-related variant of the second grain
    for (unsigned int j = 0; j < 24; ++j)
    {
      R = _symmetry_variants[j].transpose() * P;

      // This rotates any vector in cube P into a vector in cube S
      Quaternion q = {0.0};

      // Calculation from here on out is much easier with quaternions.
      mat2Quat(R, q);

      const Real lq = std::sqrt(q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
      RealVectorValue axi(q[1] / lq, q[2] / lq, q[3] / lq);

      // Rotation angle
      Real psi = 2.0 * std::acos(q[0]);
      Real dot_p = axi * axes[i];

      // Compute rotational distance from boundary P/S to the rotation set i.
      // This formula produces 2*sin(delta/2), where delta is the angle of closest approach.
      const Real dis = 2 * std::sqrt(std::abs(1 - dot_p * dot_p)) * std::sin(psi / 2.0);

      if (dis < dismax)
      {
        // angle of rotation about ax that most closely approximates R
        Real theta = 2 * std::atan(dot_p * std::tan(psi / 2.0));

        // compute the normal of the best-fitting GB in grain 1
        RealVectorValue n1 = P.row(0);
        RealVectorValue n2 = _symmetry_variants[j].row(0);

        RealTensorValue RA;
        Quaternion q2 = {std::cos(theta / 2.0),
                         std::sin(theta / 2.0) * axes[i](0),
                         std::sin(theta / 2.0) * axes[i](1),
                         std::sin(theta / 2.0) * axes[i](2)};

        // Rotation matrix that most closely approximates R
        quat2Mat(q2, RA);

        // from this point on we're dealing with the idealized rotation RA, not the original R
        RealVectorValue m1 = n1 + RA.transpose() * n2;

        const Real l = m1.norm();
        if (l >= _epsilon) // discard values for very large distances
        {

          // halfway between the two normal vectors from the two grains
          m1 /= l;

          // same representation in the other grain
          RealVectorValue m2 = RA * m1;

          // compute the inclination angle for the common rotation axis and avoid NaN's if m1 and
          // axes[i] are exactly parallel
          const Real m1_dot_axes = std::abs(m1 * axes[i]);
          Real phi = m1_dot_axes > 1.0 ? 0.0 : std::acos(m1_dot_axes);

          Real theta1;
          Real theta2;

          // partition the total rotation angle "theta"
          if (std::abs(axes[i] * m1) > (1.0 - _epsilon)) // check if best-fitting GB is pure twist
          {
            theta1 = -theta / 2.0; // eta is meaningless for a twist boundary
            theta2 = theta / 2.0;
          }
          else
          {
            // Project m1 and m2 into the plane normal to ax and determine
            // the rotation angles of them relative to dir
            theta1 = std::atan2(dir2 * m1, dirs[i] * m1);
            theta2 = std::atan2(dir2 * m2, dirs[i] * m2);
          }

          // Reduce both angles to interval (-period/2,period/2)
          // semi-open with a small numerical error
          theta2 = theta2 - MathUtils::round(theta2 / period) * period;
          theta1 = theta1 - MathUtils::round(theta1 / period) * period;

          // implement the semi-open interval in order to avoid an annoying
          // numerical problem where certain representations are double-counted
          if (std::abs(theta2 + period / 2) < _epsilon)
            theta2 = theta2 + period;
          if (std::abs(theta1 + period / 2) < _epsilon)
            theta1 = theta1 + period;

          /* Since this is only being run on fcc and fluorite elements, which are
          centrosymmetric, and all dir vectors are 2-fold axes, then
          the operations of swapping theta1 and theta2, and of
          multilying both by -1, are symmetries for the energy
          function. This lets us fold everything into a small right
          triangle in (ksi,eta) space: */
          Real ksi = std::abs(theta2 - theta1);
          Real eta = std::abs(theta2 + theta1);

          // round everything to 1e-6, so that negligible numerical differences are dropped
          geom[0][thisindex] = _epsilon * MathUtils::round(dis / _epsilon);
          geom[1][thisindex] = _epsilon * MathUtils::round(ksi / _epsilon);
          geom[2][thisindex] = _epsilon * MathUtils::round(eta / _epsilon);
          geom[3][thisindex] = _epsilon * MathUtils::round(phi / _epsilon);
          thisindex = thisindex + 1;
        }
        else
        {
          // discard large geom[0]
          geom[0].erase(geom[0].begin() + thisindex);
          geom[1].erase(geom[1].begin() + thisindex);
          geom[2].erase(geom[2].begin() + thisindex);
          geom[3].erase(geom[3].begin() + thisindex);
        }
      }
      else
      {
        // dump excess preallocated slots
        geom[0].erase(geom[0].begin() + thisindex);
        geom[1].erase(geom[1].begin() + thisindex);
        geom[2].erase(geom[2].begin() + thisindex);
        geom[3].erase(geom[3].begin() + thisindex);
      }
    }
  }
  sortGeom(geom);
}

void
AnisoGBEnergyMaterial::sortGeom(std::vector<std::vector<Real>> & geom)
{
  unsigned int geom_size = geom[0].size();
  // bail out early because the unsigned math in the loops will fail in this case
  if (geom_size == 0)
    return;

  // sort values by distance, ksi, eta, and phi
  bool change = true;
  while (change)
  {
    change = false;
    for (unsigned int i = 0; i < (geom_size - 1); ++i)
    {
      if (geom[0][i + 1] < geom[0][i])
      {
        Real temp = geom[0][i];
        geom[0][i] = geom[0][i + 1];
        geom[0][i + 1] = temp;
        temp = geom[1][i];
        geom[1][i] = geom[1][i + 1];
        geom[1][i + 1] = temp;
        temp = geom[2][i];
        geom[2][i] = geom[2][i + 1];
        geom[2][i + 1] = temp;
        temp = geom[3][i];
        geom[3][i] = geom[3][i + 1];
        geom[3][i + 1] = temp;
        change = true;
      }
      else if (geom[0][i + 1] == geom[0][i])
      {
        if (geom[1][i + 1] < geom[1][i])
        {
          Real temp = geom[0][i];
          geom[0][i] = geom[0][i + 1];
          geom[0][i + 1] = temp;
          temp = geom[1][i];
          geom[1][i] = geom[1][i + 1];
          geom[1][i + 1] = temp;
          temp = geom[2][i];
          geom[2][i] = geom[2][i + 1];
          geom[2][i + 1] = temp;
          temp = geom[3][i];
          geom[3][i] = geom[3][i + 1];
          geom[3][i + 1] = temp;
          change = true;
        }
        else if (geom[1][i + 1] == geom[1][i])
        {
          if (geom[2][i + 1] < geom[2][i])
          {
            Real temp = geom[0][i];
            geom[0][i] = geom[0][i + 1];
            geom[0][i + 1] = temp;
            temp = geom[1][i];
            geom[1][i] = geom[1][i + 1];
            geom[1][i + 1] = temp;
            temp = geom[2][i];
            geom[2][i] = geom[2][i + 1];
            geom[2][i + 1] = temp;
            temp = geom[3][i];
            geom[3][i] = geom[3][i + 1];
            geom[3][i + 1] = temp;
            change = true;
          }
          else if (geom[2][i + 1] == geom[2][i])
          {
            if (geom[3][i + 1] < geom[3][i])
            {
              Real temp = geom[0][i];
              geom[0][i] = geom[0][i + 1];
              geom[0][i + 1] = temp;
              temp = geom[1][i];
              geom[1][i] = geom[1][i + 1];
              geom[1][i + 1] = temp;
              temp = geom[2][i];
              geom[2][i] = geom[2][i + 1];
              geom[2][i + 1] = temp;
              temp = geom[3][i];
              geom[3][i] = geom[3][i + 1];
              geom[3][i + 1] = temp;
              change = true;
            }
          }
        }
      }
    }
  }

  // remove duplicate values. Real-counting in the same representation of one boundary messes up the
  // weighing functions in weightedMeanEnergy()
  for (unsigned int i = 0; i < (geom_size - 1); ++i)
    for (unsigned int j = i + 1; j < geom_size; ++j)
      if (geom[0][i] == geom[0][j] && geom[1][i] == geom[1][j] && geom[2][i] == geom[2][j] &&
          geom[3][i] == geom[3][j])
      {
        geom[0].erase(geom[0].begin() + j);
        geom[1].erase(geom[1].begin() + j);
        geom[2].erase(geom[2].begin() + j);
        geom[3].erase(geom[3].begin() + j);
        --j;
        geom_size = geom[0].size();
      }

  // remove excess zeroes
  for (unsigned int i = 0; i < geom_size; ++i)
    if (geom[0][i] == 0 && geom[1][i] == 0 && geom[2][i] == 0 && geom[3][i] == 0)
    {
      geom[0].erase(geom[0].begin() + i);
      geom[1].erase(geom[1].begin() + i);
      geom[2].erase(geom[2].begin() + i);
      geom[3].erase(geom[3].begin() + i);
      --i;
      geom_size = geom[0].size();
    }
}

Real
AnisoGBEnergyMaterial::weightedMeanEnergy(std::string whichComponent)
{
  // Pull out the parameters relevant to the weighting of the 100, 110, and 111 sets
  const Real & eRGB = _par_vec[0];      // The only dimensioned parameter. The energy scale,
                                        // set by the energy of a random boundary.
  const Real & d0100 = _par_vec[1];     // Maximum distance for the 100 set. Also the distance
                                        // scale for the RSW weighting function.
  const Real & d0110 = _par_vec[2];     // same for the 110 set
  const Real & d0111 = _par_vec[3];     // same for the 111 set
  const Real & weight100 = _par_vec[4]; // weight for the 100 set, relative to the random boundary
  const Real & weight110 = _par_vec[5]; // same for 110
  const Real & weight111 = _par_vec[6]; // same for 111

  // Cutoff of weighting function at small d for numerical purposes
  Real offset = _epsilon;

  const unsigned int geom100_size = _geom100[0].size();
  const unsigned int geom110_size = _geom110[0].size();
  const unsigned int geom111_size = _geom111[0].size();

  mooseAssert(geom100_size == _geom100[1].size(), "geom100 size inconsistency");
  mooseAssert(geom110_size == _geom110[1].size(), "geom110 size inconsistency");
  mooseAssert(geom111_size == _geom111[1].size(), "geom111 size inconsistency");

  // Calculate energy lists in units of eRGB
  _e100.resize(geom100_size);
  set100(whichComponent);
  _e110.resize(geom110_size);
  set110(whichComponent);
  _e111.resize(geom111_size);
  set111(whichComponent);

  _s100.resize(geom100_size);
  _s110.resize(geom110_size);
  _s111.resize(geom111_size);

  // Calculate the weights, in a manner designed to give an RSW-like function of distance
  // Note it calculates a weight for every representation of the boundary in each set
  for (unsigned int i = 0; i < geom100_size; ++i)
  {
    if (_geom100[0][i] > d0100) // weight saturates at zero above d0
      _s100[i] = 1.0;
    else if (_geom100[0][i] <
             (offset * d0100)) // Avoid nan's, replace with something small but finite
      _s100[i] = offset * libMesh::pi / 2.0;
    else
      _s100[i] = std::sin((libMesh::pi / 2.0) * _geom100[0][i] / d0100);
  }

  // same for 110
  for (unsigned int i = 0; i < geom110_size; ++i)
  {
    if (_geom110[0][i] > d0110)
      _s110[i] = 1.0;
    else if (_geom110[0][i] < (offset * d0110))
      _s110[i] = offset * libMesh::pi / 2.0;
    else
      _s110[i] = std::sin((libMesh::pi / 2.0) * _geom110[0][i] / d0110);
  }

  // same for 111
  for (unsigned int i = 0; i < geom111_size; ++i)
  {
    if (_geom111[0][i] > d0111)
      _s111[i] = 1.0;
    else if (_geom111[0][i] < (offset * d0111))
      _s111[i] = offset * libMesh::pi / 2.0;
    else
      _s111[i] = std::sin((libMesh::pi / 2.0) * _geom111[0][i] / d0111);
  }

  Real w100;
  Real w110;
  Real w111;

  Real sum100 = 0.0;
  Real sum110 = 0.0;
  Real sum111 = 0.0;

  Real sumw100 = 0.0;
  Real sumw110 = 0.0;
  Real sumw111 = 0.0;

  // calculate weights and sum up results
  for (unsigned int i = 0; i < geom100_size; ++i)
  {
    ///////
    //WE NEED TO MAKE _deriv_geom LATER
    ///////
    w100 = (1.0 / (_s100[i] * (1.0 - 0.5 * std::log(_s100[i]))) - 1.0) * weight100;
    sum100 += (_e100[i] * w100);
    sumw100 += w100;
  }
  for (unsigned int i = 0; i < geom110_size; ++i)
  {
    w110 = (1 / (_s110[i] * (1.0 - 0.5 * std::log(_s110[i]))) - 1.0) * weight110;
    sum110 += (_e110[i] * w110);
    sumw110 += w110;
  }
  for (unsigned int i = 0; i < geom111_size; ++i)
  {
    w111 = (1.0 / (_s111[i] * (1.0 - 0.5 * std::log(_s111[i]))) - 1) * weight111;
    sum111 += (_e111[i] * w111);
    sumw111 += w111;
  }

  // calculate energy
  if(whichComponent == "None")
    return eRGB * (sum100 + sum110 + sum111 + 1) / (sumw100 + sumw110 + sumw111 + 1);
  else
    return eRGB * (sum100 + sum110 + sum111) / (sumw100 + sumw110 + sumw111 + 1);
}

void
AnisoGBEnergyMaterial::set100(std::string whichComponent)
{
  const Real & pwr1 = _par_vec[7]; // 100 tilt/twist mix power law: twist
  const Real & pwr2 = _par_vec[8]; // 100 tilt/twist mix power law: tilt

  const unsigned int geom100_size = _geom100[1].size();
  _entwist.resize(geom100_size);
  _entilt.resize(geom100_size);

  twist100(_geom100[1], whichComponent);
  aTGB100(_geom100[2], _geom100[1], whichComponent);

  if(whichComponent == "phi")
    for (unsigned int i = 0; i < geom100_size; ++i)
      _e100[i] = -2 * _entwist[i] * pwr1 * std::pow(1 - (2.0 * _geom100[3][i] / libMesh::pi), pwr1 - 1) / libMesh::pi +
                 2 * _entilt[i] * pwr2 * std::pow((2.0 * _geom100[3][i] / libMesh::pi), pwr2 - 1) / libMesh::pi;
  else
    for (unsigned int i = 0; i < geom100_size; ++i)
      _e100[i] = _entwist[i] * std::pow(1.0 - (2.0 * _geom100[3][i] / libMesh::pi), pwr1) +
                 _entilt[i] * std::pow((2.0 * _geom100[3][i] / libMesh::pi), pwr2);
}

void
AnisoGBEnergyMaterial::set110(std::string whichComponent)
{
  const Real & pwr1 = _par_vec[19]; // 110 tilt/twist mix power law: twist
  const Real & pwr2 = _par_vec[20]; // 110 tilt/twist mix power law: tilt

  const unsigned int geom110_size = _geom110[1].size();
  _entwist.resize(geom110_size);
  _entilt.resize(geom110_size);

  twist110(_geom110[1], whichComponent);
  aTGB110(_geom110[2], _geom110[1], whichComponent);

  if(whichComponent == "phi")
    for (unsigned int i = 0; i < geom110_size; ++i)
      _e110[i] = -2 * _entwist[i] * pwr1 * std::pow(1 - (2.0 * _geom100[3][i] / libMesh::pi), pwr1 - 1) / libMesh::pi +
                 2 * _entilt[i] * pwr2 * std::pow(2.0 * _geom100[3][i] / libMesh::pi, pwr2 - 1) / libMesh::pi;
  else
    for (unsigned int i = 0; i < geom110_size; ++i)
      _e110[i] = _entwist[i] * std::pow((1.0 - (2.0 * _geom110[3][i] / libMesh::pi)), pwr1) +
                 _entilt[i] * std::pow((2.0 * _geom110[3][i] / libMesh::pi), pwr2);
}

void
AnisoGBEnergyMaterial::set111(std::string whichComponent)
{
  const Real & a = _par_vec[34]; // linear part of 111 tilt/twist interpolation
  Real b = a - 1.0;              // ensures correct value at x = 1

  const unsigned int geom111_size = _geom111[1].size();
  _entwist.resize(geom111_size);
  _entilt.resize(geom111_size);

  twist111(_geom111[1], whichComponent);
  aTGB111(_geom111[2], _geom111[1], whichComponent);

  // This one fit well enough with a simple one-parameter parabola that the more
  // complicated power laws in the other sets weren't needed
  if(whichComponent == "phi")
  for (unsigned int i = 0; i < geom111_size; ++i)
    _e111[i] = _entwist[i] + (_entilt[i] - _entwist[i]) *
                             (a * 2.0 / libMesh::pi -
                              b * 4 * std::pow((2.0 * _geom111[3][i] / libMesh::pi), 2.0) / libMesh::pi);
  else
    for (unsigned int i = 0; i < geom111_size; ++i)
      _e111[i] = _entwist[i] + (_entilt[i] - _entwist[i]) *
                                   (a * 2.0 * _geom111[3][i] / libMesh::pi -
                                    b * std::pow((2.0 * _geom111[3][i] / libMesh::pi), 2.0));
}

void
AnisoGBEnergyMaterial::twist100(std::vector<Real> ksi, std::string whichComponent)
{
  const Real Emax = _par_vec[9];         // 100 twist maximum energy
  const Real a = _par_vec[10];           // 100 twist RSW shape factor.
  const Real period = libMesh::pi / 2.0; // twist period

  const unsigned int ksi_size = ksi.size();
  for (unsigned int i = 0; i < ksi_size; ++i)
  {
    if(whichComponent == "ksi")
    {
      Real Dxlogx = 2 * std::cos(2 * ksi[i]) * std::log(std::sin(2 * ksi[i])) +
                    2 * std::cos(2 * ksi[i]);
      if (std::isnan(Dxlogx))
        Dxlogx = 2 * std::cos(2 * ksi[i]);
      _entwist[i] = Emax * (std::cos(2 * ksi[i]) - a * Dxlogx);
    }
    else
    {
      ksi[i] = std::fmod(std::abs(ksi[i]), period); // rotation symmetry
      if (ksi[i] > period / 2)
        ksi[i] = period - ksi[i];

        // implement an RSW function of ksi
        Real xlogx = std::sin(2 * ksi[i]) * std::log(std::sin(2 * ksi[i]));
        if (std::isnan(xlogx)) // force the limit to zero as x->0
        xlogx = 0.0;

        _entwist[i] = Emax * (std::sin(2 * ksi[i]) - a * xlogx);
    }
  }
}

void
AnisoGBEnergyMaterial::twist110(std::vector<Real> ksi, std::string whichComponent)
{
  const Real th1 = _par_vec[21]; // 110 twist peak position

  const Real en1 = _par_vec[22]; // 110 twist energy peak value
  const Real en2 = _par_vec[23]; // Sigma3 energy (110 twist, so not a coherent twin)
  const Real en3 = _par_vec[24]; // energy at the symmetry point

  const Real a01 = 0.5;
  const Real a12 = 0.5;
  const Real a23 = 0.5;

  const Real th2 = std::acos(1.0 / 3.0); // Sigma3
  const Real th3 = libMesh::pi / 2.0; // 110 90-degree boundary is semi-special, although not a CSL
  const Real period = libMesh::pi;    // the twist period

  const unsigned int ksi_size = ksi.size();
  for (unsigned int i = 0; i < ksi_size; ++i)
  {
    ksi[i] = std::fmod(std::abs(ksi[i]), period); // rotation symmetry
    if (ksi[i] > period / 2.0)
      ksi[i] = period - ksi[i];
    if (ksi[i] <= th1)
      _entwist[i] = en1 * rSW(ksi[i], 0, th1, a01, whichComponent);
    else if (ksi[i] > th1 && ksi[i] <= th2)
      _entwist[i] = en2 + (en1 - en2) * rSW(ksi[i], th2, th1, a12, whichComponent);
    else if (ksi[i] > th2)
    {
      _entwist[i] = en3 + (en2 - en3) * rSW(ksi[i], th3, th2, a23, whichComponent);
    }
  }
}

void
AnisoGBEnergyMaterial::twist111(std::vector<Real> ksi, std::string whichComponent)
{
  const Real thd = _par_vec[36]; // 111 twist peak position

  const Real enm = _par_vec[37]; // 111 twist energy at the peak
  const Real en2 = _par_vec[27]; // Coherent Sigma3 twin shows up in two distinct places in the code

  const Real a1 = _par_vec[35]; // 111 twist RSW shape parameter
  const Real a2 = a1;

  const unsigned int ksi_size = ksi.size();
  for (unsigned int i = 0; i < ksi_size; ++i)
  {
    if (ksi[i] > libMesh::pi / 3.0)
      ksi[i] = 2.0 * libMesh::pi / 3.0 - ksi[i];
    if (ksi[i] <= thd)
      _entwist[i] = enm * rSW(ksi[i], 0, thd, a1, whichComponent);
    else
      _entwist[i] = en2 + (enm - en2) * rSW(ksi[i], libMesh::pi / 3.0, thd, a2, whichComponent);
  }
}

void
AnisoGBEnergyMaterial::aTGB100(const std::vector<Real> & eta, const std::vector<Real> & ksi, std::string whichComponent)
{
  const Real pwr = _par_vec[11]; // 100 aTGB interpolation power law
  const Real period = libMesh::pi / 2.0;

  if(whichComponent == "ksi")
  {
    _Den1.resize(ksi.size());
    _Den2.resize(ksi.size());
  }
  _en1.resize(ksi.size());
  _en2.resize(ksi.size());
  _ksi_back.resize(ksi.size());

  sTGB100(ksi, _en1, _Den1, whichComponent); // value at eta = 0

  const unsigned int ksi_size = ksi.size();
  for (unsigned int i = 0; i < ksi_size; ++i)
    _ksi_back[i] = period - ksi[i];
  sTGB100(_ksi_back, _en2, _Den2, whichComponent); // value at eta = pi/2

  // eta dependence is a power law that goes from the higher to the lower,
  // whichever direction that is
  for (unsigned int i = 0; i < ksi_size; ++i)
    if (_en1[i] >= _en2[i])
      if(whichComponent == "eta")
        _entilt[i] = _en1[i] - (_en1[i] - _en2[i]) * pwr * std::pow((eta[i] / period), pwr - 1);
      else if(whichComponent == "ksi")
        _entilt[i] = _Den1[i] - (_Den1[i] - _Den2[i]) * std::pow((eta[i] / period), pwr);
      else
        _entilt[i] = _en1[i] - (_en1[i] - _en2[i]) * std::pow((eta[i] / period), pwr);
    else
      if(whichComponent =="eta")
        _entilt[i] = _en2[i] - (_en2[i] - _en1[i]) * pwr * std::pow((1.0 - (eta[i] / period)), pwr - 1);
      else if(whichComponent == "ksi")
        _entilt[i] = _Den2[i] - (_Den2[i] - _Den1[i]) * std::pow((1.0 - (eta[i] / period)), pwr);
      else
        _entilt[i] = _en2[i] - (_en2[i] - _en1[i]) * std::pow((1.0 - (eta[i] / period)), pwr);
}

void
AnisoGBEnergyMaterial::aTGB110(const std::vector<Real> & eta, const std::vector<Real> & ksi, std::string whichComponent)
{
  // 110 aTGB interpolation RSW shape factor
  const Real a = _par_vec[25];
  const Real period = libMesh::pi;

  const unsigned int ksi_size = ksi.size();

  if(whichComponent == "ksi")
  {
    _Den1.resize(ksi.size());
    _Den2.resize(ksi.size());
  }
  _en1.resize(ksi.size());
  _en2.resize(ksi.size());
  _ksi_back.resize(ksi.size());

  sTGB110(ksi, _en1, _Den1, whichComponent);
  for (unsigned int i = 0; i < ksi_size; ++i)
    _ksi_back[i] = period - ksi[i];
  sTGB110(_ksi_back, _en2, _Den2, whichComponent);

  // Power-law interpolation did not work well in this case. Did an RSW function instead
  for (unsigned int i = 0; i < ksi_size; ++i)
  {
    if (_en1[i] >= _en2[i])
      if(whichComponent == "eta")
        _entilt[i] = _en2[i] + (_en1[i] - _en2[i]) * rSW(eta[i], libMesh::pi, 0.0, a, whichComponent);
      else if(whichComponent == "ksi")
        _entilt[i] = _Den2[i] + (_Den1[i] - _Den2[i]) * rSW(eta[i], libMesh::pi, 0.0, a);
      else
        _entilt[i] = _en2[i] + (_en1[i] - _en2[i]) * rSW(eta[i], libMesh::pi, 0.0, a);
    else
      if(whichComponent == "eta")
        _entilt[i] = _en1[i] + (_en2[i] - _en1[i]) * rSW(eta[i], 0.0, libMesh::pi, a, whichComponent);
      else if(whichComponent == "ksi")
        _entilt[i] = _Den1[i] + (_Den2[i] - _Den1[i]) * rSW(eta[i], 0.0, libMesh::pi, a);
      else
        _entilt[i] = _en1[i] + (_en2[i] - _en1[i]) * rSW(eta[i], 0.0, libMesh::pi, a);
  }
}

void
AnisoGBEnergyMaterial::aTGB111(const std::vector<Real> & eta, const std::vector<Real> & ksi, std::string whichComponent)
{
  /**
   * Below the following value of ksi, we ignore the eta dependence.  This is
   * because there's little evidence that it actually varies.  Above this
   * value, we interpolate on an RSW function that follows the Sigma3 line,
   * which is also a line of symmetry for the function.
   */
  const Real & ksim = _par_vec[38];  // 111 aTGB ksi break
  const Real & enmax = _par_vec[39]; // energy at the peak (ksi == ksim)
  const Real & enmin = _par_vec[40]; // energy at the minimum (Sigma3, eta == 0)
  const Real & encnt = _par_vec[41]; // energy at the symmetry point (Sigma3, eta == pi/3)

  Real a1 = 0.5;
  Real a2 = 0.5;

  /**
   * eta scaling parameter for 111 aTGB RSW function on Sigma3 line
   * This RSW function is unusual in that the change in shape of the
   * function is much better captured by changing the angular scale rather
   * than changing the dimensionless shape factor.
   */
  const Real & etascale = _par_vec[42];
  Real chi;
  const unsigned int ksi_size = ksi.size();
  for (unsigned int i = 0; i < ksi_size; ++i)
  {
    const Real ksii = ksi[i] > libMesh::pi / 3.0 ? 2.0 * libMesh::pi / 3.0 - ksi[i] : ksi[i];
    const Real etai = eta[i] > libMesh::pi / 3.0 ? 2.0 * libMesh::pi / 3.0 - eta[i] : eta[i];

    if (ksii <= ksim)
      if(whichComponent == "ksi")
        _entilt[i] = enmax * rSW(ksii, 0, ksim, a1, whichComponent);
      else
        _entilt[i] = enmax * rSW(ksii, 0, ksim, a1);
    else
    {
      // chi is the shape of the function along the Sigma3 line.
      if(whichComponent == "eta")
        chi = enmin + (encnt - enmin) * rSW(etai, 0, libMesh::pi / (2.0 * etascale), 0.5, whichComponent);
      else
        chi = enmin + (encnt - enmin) * rSW(etai, 0, libMesh::pi / (2.0 * etascale), 0.5);
      if(whichComponent == "ksi")
        _entilt[i] = chi + (enmax - chi) * rSW(ksii, libMesh::pi / 3.0, ksim, a2, whichComponent);
      else
        _entilt[i] = chi + (enmax - chi) * rSW(ksii, libMesh::pi / 3.0, ksim, a2);
    }
  }
}

void
AnisoGBEnergyMaterial::sTGB100(const std::vector<Real> & ksi, std::vector<Real> & en, std::vector<Real> & Den, std::string whichComponent)
{
  const Real & en2 = _par_vec[12]; // peak before first Sigma5
  const Real & en3 = _par_vec[13]; // first Sigma5
  const Real & en4 = _par_vec[14]; // peak between Sigma5's
  const Real & en5 = _par_vec[15]; // second Sigma5
  const Real & en6 = _par_vec[16]; // Sigma17

  const Real & th2 = _par_vec[17];                 // position of peak before first Sigma5
  const Real & th4 = _par_vec[18];                 // position of peak between Sigma5's
  Real th6 = 2 * std::acos(5.0 / std::sqrt(34.0)); // Sigma17 rotation angle

  Real a12 = 0.5; // RSW shape factor. In previous versions, these were allowed to
  Real a23 = a12; // vary, however there were too few vicinal boundaries in the
  Real a34 = a12; // ensemble to constrain them. We found that forcing the great
  Real a45 = a12; // majority of them to be 0.5 helped to constrain the fit and
  Real a56 = a12; // produced reasonable results. This holds true for most of the
  Real a67 = a12; // RSW shape factors throughout this code.

  Real en1 = 0.0; // Sigma1 at left end
  Real en7 = 0.0; // Sigma1 at right end

  Real th1 = 0.0;               // Sigma1 at left end
  Real th3 = std::acos(0.8);    // first Sigma5
  Real th5 = std::acos(0.6);    // second Sigma5
  Real th7 = libMesh::pi / 2.0; // Sigma1 at right end

  // piecewise RSW function
  const unsigned int ksi_size = ksi.size();
  for (unsigned int i = 0; i < ksi_size; ++i)
  {
    if (ksi[i] <= th2)
      en[i] = en1 + (en2 - en1) * rSW(ksi[i], th1, th2, a12);
      if (whichComponent == "ksi")
        Den[i] = (en2 - en1) * rSW(ksi[i], th1, th2, a12, whichComponent);
    else if (ksi[i] > th2 && ksi[i] <= th3)
      en[i] = en3 + (en2 - en3) * rSW(ksi[i], th3, th2, a23);
      if (whichComponent == "ksi")
        Den[i] = (en2 - en3) * rSW(ksi[i], th3, th2, a23, whichComponent);
    else if (ksi[i] > th3 && ksi[i] <= th4)
      en[i] = en3 + (en4 - en3) * rSW(ksi[i], th3, th4, a34);
      if (whichComponent == "ksi")
        Den[i] = (en4 - en3) * rSW(ksi[i], th3, th4, a34, whichComponent);
    else if (ksi[i] > th4 && ksi[i] <= th5)
      en[i] = en5 + (en4 - en5) * rSW(ksi[i], th5, th4, a45);
      if (whichComponent == "ksi")
        Den[i] = (en4 - en5) * rSW(ksi[i], th5, th4, a45, whichComponent);
    else if (ksi[i] > th5 && ksi[i] <= th6)
      en[i] = en6 + (en5 - en6) * rSW(ksi[i], th6, th5, a56);
      if (whichComponent == "ksi")
        Den[i] = (en5 - en6) * rSW(ksi[i], th6, th5, a56, whichComponent);
    else if (ksi[i] > th6 && ksi[i] <= th7)
      en[i] = en7 + (en6 - en7) * rSW(ksi[i], th7, th6, a67);
      if (whichComponent == "ksi")
        Den[i] = (en6 - en7) * rSW(ksi[i], th7, th6, a67, whichComponent);
  }
}

void
AnisoGBEnergyMaterial::sTGB110(const std::vector<Real> & ksi, std::vector<Real> & en, std::vector<Real> & Den, std::string whichComponent)
{
  const Real & en2 = _par_vec[26]; // peak between Sigma1 and Sigma3
  const Real & en3 = _par_vec[27]; // Coherent Sigma3 twin relative energy; one of the more
                                   // important element-dependent parameters
  const Real & en4 = _par_vec[28]; // energy peak between Sigma3 and Sigma11
  const Real & en5 = _par_vec[29]; // Sigma11 energy
  const Real & en6 = _par_vec[30]; // energy peak between Sigma11 and Sigma1

  const Real & th2 = _par_vec[31]; // peak between Sigma1 and Sigma3
  const Real & th4 = _par_vec[32]; // peak between Sigma3 and Sigma11
  const Real & th6 = _par_vec[33]; // peak between Sigma11 and higher Sigma1

  Real a12 = 0.5;
  Real a23 = a12;
  Real a34 = a12;
  Real a45 = a12;
  Real a56 = a12;
  Real a67 = a12;

  Real en1 = 0.0;
  Real en7 = 0.0;

  Real th1 = 0.0;
  Real th3 = std::acos(1.0 / 3.0);   // Sigma3
  Real th5 = std::acos(-7.0 / 11.0); // Sigma11
  Real th7 = libMesh::pi;

  const unsigned int ksi_size = ksi.size();
  for (unsigned int i = 0; i < ksi_size; ++i)
  {
    Real ksii = libMesh::pi - ksi[i]; // This is a legacy of an earlier ksi-eta mapping.
    if (ksii <= th2)
      en[i] = en1 + (en2 - en1) * rSW(ksii, th1, th2, a12);
      if(whichComponent == "ksi")
        Den[i] = en1 + (en2 - en1) * rSW(ksii, th1, th2, a12, whichComponent);
    else if (ksii > th2 && ksii <= th3)
      en[i] = en3 + (en2 - en3) * rSW(ksii, th3, th2, a23);
      if(whichComponent == "ksi")
        Den[i] = en3 + (en2 - en3) * rSW(ksii, th3, th2, a23, whichComponent);
    else if (ksii > th3 && ksii <= th4)
      en[i] = en3 + (en4 - en3) * rSW(ksii, th3, th4, a34);
      if(whichComponent == "ksi")
        Den[i] = en3 + (en4 - en3) * rSW(ksii, th3, th4, a34, whichComponent);
    else if (ksii > th4 && ksii <= th5)
      en[i] = en5 + (en4 - en5) * rSW(ksii, th5, th4, a45);
      if(whichComponent == "ksi")
        Den[i] = en5 + (en4 - en5) * rSW(ksii, th5, th4, a45, whichComponent);
    else if (ksii > th5 && ksii <= th6)
      en[i] = en5 + (en6 - en5) * rSW(ksii, th5, th6, a56);
      if(whichComponent == "ksi")
        Den[i] = en5 + (en6 - en5) * rSW(ksii, th5, th6, a56, whichComponent);
    else if (ksii > th6 && ksii <= th7)
      en[i] = en7 + (en6 - en7) * rSW(ksii, th7, th6, a67);
      if(whichComponent == "ksi")
        Den[i] = en7 + (en6 - en7) * rSW(ksii, th7, th6, a67, whichComponent);
  }
}

Real
AnisoGBEnergyMaterial::rSW(Real theta, Real thetaMin, Real thetaMax, Real a, std::string whichComponent)
{
  // Interval of angles where defined
  const Real dtheta = thetaMax - thetaMin;

  // Normalized angle
  const Real sintheta = std::sin((theta - thetaMin) * libMesh::pi / (dtheta * 2));
  const Real Dsintheta = libMesh::pi * std::cos((theta - thetaMin) * libMesh::pi / (dtheta * 2)) / (dtheta * 2);
  // Cut off a small sins to avoid 0 * infinity problem.
  // The proper limit is 0.
  if (sintheta >= _epsilon)
    if(whichComponent == "None")
      return sintheta - a * (sintheta * std::log(sintheta));
    else
      return Dsintheta - a * (Dsintheta * std::log(sintheta) + Dsintheta);
  else
    if(whichComponent == "None")
      return sintheta;
    else
      return Dsintheta;
}
