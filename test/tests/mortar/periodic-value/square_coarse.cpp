#include <gmsh.h>
#include <set>
#include <iostream>

namespace factory = gmsh::model::geo;

int
main(int argc, char ** argv)
{
  gmsh::initialize();

  gmsh::option::setNumber("General.Terminal", 1);

  gmsh::model::add("square_coarse");

  double length = 1;
  double left_lc = .5;
  double right_lc = .4;

  auto bottom_left = factory::addPoint(0, 0, 0, left_lc);
  auto bottom_right = factory::addPoint(length, 0, 0, right_lc);
  auto top_right = factory::addPoint(length, length, 0, right_lc);
  auto top_left = factory::addPoint(0, length, 0, left_lc);

  // Generate the volume

  auto bottom = factory::addLine(bottom_left, bottom_right);
  auto right = factory::addLine(bottom_right, top_right);
  auto top = factory::addLine(top_right, top_left);
  auto left = factory::addLine(top_left, bottom_left);

  auto curve_loop = factory::addCurveLoop({bottom, right, top, left});
  auto surface = factory::addPlaneSurface({curve_loop});

  // Add all the boundary ids and names

  size_t boundary_id_counter = 100;

  auto bottom_boundary_id = boundary_id_counter++;
  gmsh::model::addPhysicalGroup(1, {bottom}, bottom_boundary_id);
  gmsh::model::setPhysicalName(1, bottom_boundary_id, "bottom");

  auto right_boundary_id = boundary_id_counter++;
  gmsh::model::addPhysicalGroup(1, {right}, right_boundary_id);
  gmsh::model::setPhysicalName(1, right_boundary_id, "right");

  auto top_boundary_id = boundary_id_counter++;
  gmsh::model::addPhysicalGroup(1, {top}, top_boundary_id);
  gmsh::model::setPhysicalName(1, top_boundary_id, "top");

  auto left_boundary_id = boundary_id_counter++;
  gmsh::model::addPhysicalGroup(1, {left}, left_boundary_id);
  gmsh::model::setPhysicalName(1, left_boundary_id, "left");

  // Add the subdomain ids and names

  size_t subdomain_id_counter = 10;

  auto volume_subdomain_id = subdomain_id_counter++;
  gmsh::model::addPhysicalGroup(2, {surface}, volume_subdomain_id);
  gmsh::model::setPhysicalName(2, volume_subdomain_id, "domain");

  factory::synchronize();

  gmsh::model::mesh::generate(2);
  gmsh::model::mesh::setOrder(2);

  gmsh::write("square_coarse.msh");

  gmsh::finalize();
  return 0;
}
