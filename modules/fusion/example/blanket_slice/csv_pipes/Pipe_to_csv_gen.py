#imports
import pandas as pd
import gmsh 
import sys

sys.path.append('../Meshes/')

gmsh.initialize()
gmsh.model.add("Blanket_Simple.geo")

ov = gmsh.model.geo.copy([0, 15])
gmsh.model.geo.translate(ov, w_ch/2, d_ch/2, 0)

gmsh.model.mesh.generate(3)
gmsh.write('Blanket_Slice.msh')



