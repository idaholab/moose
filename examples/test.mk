.PHONY: test
TEST_exodiff = \
	@./$(APPLICATION_NAME)-$(METHOD) -i $(1) &> /dev/null ; \
	exodiff -F 1e-11 -use_old_floor -t 5.5E-6 $(2) gold/$(2) | grep -qP \(different\|ERROR\|command\ not\ found\) && \
	echo $(APPLICATION_NAME) ... FAILED || \
	echo $(APPLICATION_NAME) ... OK 
