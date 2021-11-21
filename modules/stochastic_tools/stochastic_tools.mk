libmesh_CXXFLAGS += -DENABLE_TF

libmesh_CXXFLAGS += -I$(CONDA_PREFIX_1)/pkgs/tensorflow-base-2.4.1-py39h23a8cbf_0/lib/python3.9/site-packages/tensorflow/include
# libmesh_CXXFLAGS += -I$(HOME)/miniconda3/pkgs/libprotobuf-3.15.8-h780b84a_0/include
# libmesh_CXXFLAGS += -Wl,-rpath-link,$(HOME)/miniconda3/pkgs/tensorflow-base-2.4.1-py39h23a8cbf_0/lib/python3.9/site-packages/tensorflow
# libmesh_CXXFLAGS += -Wl,-rpath-link,$(HOME)/miniconda3/pkgs/libprotobuf-3.15.8-h780b84a_0/lib
# ibmesh_CXXFLAGS += -Wl,-rpath-link,$(HOME)/miniconda3/pkgs/libprotobuf-3.15.8-h780b84a_0/lib
ibmesh_CXXFLAGS += -D_GLIBCXX_USE_CXX11_ABI=0


# libmesh_LDFLAGS += -L$$(HOME)/miniconda3/pkgs/tensorflow-base-2.4.1-py39h23a8cbf_0/lib/python3.9/site-packages/tensorflow
# libmesh_LDFLAGS += -L$(HOME)/miniconda3/pkgs/libprotobuf-3.15.8-h780b84a_0/lib
libmesh_LIBS += -ltensorflow_framework -ltensorflow_cc
# miniconda3/pkgs/tensorflow-base-2.4.1-py39h23a8cbf_0/lib/python3.9/site-packages/tensorflow
# miniconda3/pkgs/tensorflow-base-2.4.1-py39h23a8cbf_0/lib/python3.9/site-packages/tensorflow/include
