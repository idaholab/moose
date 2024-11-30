# We need to separate out declaring INSTALLABLE_DIRS for combined because
# it needs to be used in both modules/Makefile and modules/combined/Makefile
INSTALLABLE_MODULE_DIRS := $(shell find $(MOOSE_DIR)/modules -maxdepth 1 -mindepth 1 -type d -not -name doc -not -name module_loader -not -name heat_conduction -not -name tensor_mechanics)
INSTALLABLE_DIRS        := $(foreach module,$(INSTALLABLE_MODULE_DIRS),../$(shell basename $(module))/test/tests->$(shell basename $(module))) ../../tutorials/tutorial04_meshing/app/test/tests->reactor_tutorial
