TEST_exodiff = \
	@./$(APPLICATION_NAME)-$(METHOD) -i $(1) > /dev/null 2>&1 && \
	$(FRAMEWORK_DIR)/contrib/exodiff/exodiff -quiet -F 1e-8 -t 5.5E-6 $(2) gold/$(2) | grep -qi "files are the same" && \
	echo $(APPLICATION_NAME) ... OK || \
	(echo $(APPLICATION_NAME) ... FAILED && exit 1)
