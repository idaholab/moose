# list all the cpp files you need to compile
plugin_SOURCES := src/UMAT.cpp src/elasticity/TempDepIsoElasticity.cpp \
		src/creep/CreepPowerLaw.cpp src/creep/PowerLawVonMisesCreep.cpp \
		src/creep/PowerLawHillCreep.cpp src/flow_surfaces/HillFlowSurface.cpp \
		src/flow_surfaces/VonMisesFlowSurface.cpp \
		src/materialFunctor/NRMaterialFunctor.cpp src/materialFunctor/NRPLCreepHill.cpp \
		src/DataConst.cpp src/Switchboard.cpp \
		../util/src/iomsg.cpp ../util/lib/fortran_msg.f90 \
		../util/src/lowerCase.cpp ../util/src/RosenbrockFunctor.cpp \
		../util/src/PolynomialFunctor.cpp ../util/src/LinearFunctor.cpp \
		../util/src/cubicmodel.cpp ../util/src/funcnorm.cpp ../util/src/fx.cpp \
		../util/src/globalnr.cpp ../util/src/goodstepcondition.cpp \
		../util/src/incrementx.cpp ../util/src/jacfx.cpp ../util/src/lu_decomp.cpp \
		../util/src/lu_solve.cpp ../util/src/matdotvec.cpp ../util/src/printmatrix.cpp \
		../util/src/printvector.cpp ../util/src/quadraticmodel.cpp \
		../util/src/transpmatrix.cpp ../util/src/vecdotvec.cpp \
		../util/src/MatrixMultiplication.cpp

# make collection of include directories for the UEXTERNALDB
plugin_INCLUDEDIRS :=  include include/elasticity include/creep include/flow_surfaces \
		include/materialFunctor ../util/include

plugin_INCLUDES := $(shell find $(plugin_INCLUDEDIRS) -name "*.h")

# build the plugin
-include $(MOOSE_DIR)/framework/plugin.mk
