

all:: CROW


CROW: $(PYTHON_MODULES)

.PHONY : crow_clean

clean :: crow_clean

crow_clean :
	@rm -f $(CROW_DIR)/control_modules/_distribution1D.so \
	  $(CROW_DIR)/control_modules/_crowtools.so \
	  $(CROW_DIR)/control_modules/distribution1D_wrap.cxx \
	  $(CROW_DIR)/control_modules/crowtools_wrap.cxx \
	  $(CROW_DIR)/control_modules/distribution1D.py \
	  $(CROW_DIR)/control_modules/libdistribution1D.* \
	  $(CROW_DIR)/control_modules/crowtools.py \
	  $(CROW_DIR)/control_modules/*.so* \
	  $(CROW_DIR)/control_modules/*.dylib* \
          $(CROW_DIR)/control_modules/*.lo \
          $(CROW_DIR)/control_modules/*.la \
          $(CROW_DIR)/control_modules/*.pyc \
	  $(CROW_DIR)/crow_modules/*.so* \
	  $(CROW_DIR)/crow_modules/*.dylib* \
	  $(CROW_DIR)/crow_modules/*_wrap.cxx \
	  $(CROW_DIR)/crow_modules/*_wrap.cpp \
	  $(CROW_DIR)/crow_modules/*py[23].py \
	  $(CROW_DIR)/crow_modules/*.pyc \
	  $(CROW_LIB) $(CROW_DIR)/libCROW-$(METHOD).*
	@rm -Rf $(CROW_DIR)/install/ $(CROW_DIR)/build/
	@rm -Rf $(CROW_DIR)/control_modules/.libs/ $(CROW_DIR)/.libs/
	@rm -Rf $(CROW_DIR)/src/distributions/.libs/
	@rm -Rf $(CROW_DIR)/src/tools/.libs/
	@rm -Rf $(CROW_DIR)/src/utilities/.libs/
	@rm -Rf $(CROW_DIR)/src/distributions/*.lo \
	  $(CROW_DIR)/src/tools/*.lo \
	  $(CROW_DIR)/src/utilities/*.lo
	@rm -Rf $(CROW_DIR)/src/distributions/*.d \
	  $(CROW_DIR)/src/tools/*.d \
	  $(CROW_DIR)/src/utilities/*.d

clobber::
	@rm -f $(CROW_DIR)/control_modules/_distribution1D.so \
	  $(CROW_DIR)/control_modules/distribution1D_wrap.cxx \
	  $(CROW_DIR)/control_modules/distribution1D.py

cleanall::
	make -C $(CROW_DIR) clean
