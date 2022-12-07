# thermochimica
THERMOCHIMICA_DIR ?= $(MOOSE_DIR)/modules/chemical_reactions/contrib/thermochimica
THERMOCHIMICA_SRC = $(THERMOCHIMICA_DIR)/src
ifneq ($(wildcard $(THERMOCHIMICA_SRC)/Thermochimica.f90),)
  ADDITIONAL_CPPFLAGS += -DTHERMOCHIMICA_ENABLED
	include $(APPLICATION_DIR)/contrib/thermochimica.mk
endif
