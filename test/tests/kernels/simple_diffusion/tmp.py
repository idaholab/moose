import netCDF4
import deepdiff
import collections
from pprint import pprint

file0 = 'simple_diffusion_out.e'
file1 = 'gold/simple_diffusion_out.e'

data0 = netCDF4.Dataset(file0, 'r')
data1 = netCDF4.Dataset(file1, 'r')

Dimension = collections.namedtuple('Dimension', 'name size')
dim0 = {k:Dimension(v.name, v.size) for k,v in data0.dimensions.items()}
dim1 = {k:Dimension(v.name, v.size) for k,v in data1.dimensions.items()}

Variable = collections.namedtuple('Variable', 'name size shape data')
var0 = {k:Variable(v.name, v.size, v.shape, v[:]) for k,v in data0.variables.items()}
var1 = {k:Variable(v.name, v.size, v.shape, v[:]) for k,v in data1.variables.items()}


ddiff = deepdiff.DeepDiff(dim0, dim1, view='tree', ignore_order=True)
pprint(ddiff, indent=2)
pprint(ddiff.get_stats())

ddiff = deepdiff.DeepDiff(var0, var1, view='tree', ignore_order=True)
pprint(ddiff, indent=2)
pprint(ddiff.get_stats())
