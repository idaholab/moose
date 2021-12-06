libmesh_CXXFLAGS += -DENABLE_PT

libmesh_CXXFLAGS += -I/Users/germp/mambaforge/envs/moose-pt-conda39/lib/python3.9/site-packages/torch/include/torch/csrc/api/include
libmesh_CXXFLAGS += -I/Users/germp/mambaforge/envs/moose-pt-conda39/lib/python3.9/site-packages/torch/include

libmesh_LDFLAGS += -Wl,-rpath,/Users/germp/mambaforge/envs/moose-pt-conda39/lib/python3.9/site-packages/torch/lib
libmesh_LDFLAGS += -L/Users/germp/mambaforge/envs/moose-pt-conda39/lib/python3.9/site-packages/torch/lib -ltorch
