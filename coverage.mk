
# Build the code for test coverage analysis
# jhrg 11/17/20

AM_CXXFLAGS += --coverage -pg
AM_LDFLAGS += --coverage -pg

# gcov options
# -i (write .gcov files), -f (function summaries), -r (relative paths
# only - i.e., elide system functions), -m (demangle names)

# This doesn't quite work - loops forever. jhrg 11/17/20
COVDIRS = $(SUBDIRS:%=cov-%)

coverage: $(COVDIRS)
	gcov -r -i .libs/*.o

$(COVDIRS):
	$(MAKE) $(MFLAGS) -C $(@:cov-%=%) coverage

.PHONY: $(COVDIRS) coverage
