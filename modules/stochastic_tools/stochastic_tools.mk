libmesh_CXXFLAGS += -DENABLE_TF

libmesh_CXXFLAGS += -I/data/miniconda3/pkgs/tensorflow-base-2.4.1-py39h23a8cbf_0/lib/python3.9/site-packages/tensorflow/include

libmesh_LIBS += -ltensorflow_framework -ltensorflow_cc
