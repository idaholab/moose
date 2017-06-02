PYTHON_CONFIG:=python-config
PYTHON_CONFIG_WHICH:=$(shell which $(PYTHON_CONFIG) 2>/dev/null)

ifneq ($(PYTHON_CONFIG_WHICH),)
  app_INCLUDES    += $(shell $(PYTHON_CONFIG) --includes)
  libmesh_LIBS    += $(shell $(PYTHON_CONFIG) --ldflags)
else
$(error $(PYTHON_CONFIG) not found)
endif
