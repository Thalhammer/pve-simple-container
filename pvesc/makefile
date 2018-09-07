SRC = $(shell find . -name '*.cpp') $(shell find . -name '*.c')
EXCLUDE_SRC = 
FSRC = $(filter-out $(EXCLUDE_SRC), $(SRC))
OBJ = $(FSRC:=.o)

DEP_DIR = .deps

FLAGS = -fPIC -Wall -Wno-unknown-pragmas
CXXFLAGS = -std=c++14
CFLAGS =
LINKFLAGS = ../container/libcontainer.a ../deploy/libdeploy.a ../common/libcommon.a -lcurl -lstdc++fs

OUTFILE = pvesc

.PHONY: clean debug release

release: FLAGS += -O2
release: $(OUTFILE)

debug: FLAGS += -g -O0
debug: $(OUTFILE)

$(OUTFILE): $(OBJ) ../container/libcontainer.a ../deploy/libdeploy.a ../common/libcommon.a
	@echo Generating binary
	@$(CXX) -o $@ $^ $(LINKFLAGS)
	@echo Build done

%.cpp.o: %.cpp version.h
	@echo Building $<
	@$(CXX) -c $(FLAGS) $(CXXFLAGS) $< -o $@
	@mkdir -p `dirname $(DEP_DIR)/$@.d`
	@$(CXX) -c $(FLAGS) $(CXXFLAGS) -MT '$@' -MM $< > $(DEP_DIR)/$@.d

%.c.o: %.c version.h
	@echo Building $<
	@$(CC) -c $(FLAGS) $(CFLAGS) $< -o $@
	@mkdir -p `dirname $(DEP_DIR)/$@.d`
	@$(CC) -c $(FLAGS) $(CFLAGS) -MT '$@' -MM $< > $(DEP_DIR)/$@.d

clean:
	@echo Removing binary
	@rm -f $(OUTFILE)
	@echo Removing objects
	@rm -f $(OBJ)
	@rm -f version.h
	@echo Removing dependency files
	@rm -rf $(DEP_DIR)
	make -C ../container/ clean
	make -C ../deploy/ clean
	make -C ../common/ clean

../container/libcontainer.a: $(shell find ../container/ -name '*.cpp') $(shell find ../container/ -name '*.c') $(shell find ../container/ -name '*.h')
	+make -C ../container/

../deploy/libdeploy.a: $(shell find ../deploy/ -name '*.cpp') $(shell find ../deploy/ -name '*.c') $(shell find ../deploy/ -name '*.h')
	+make -C ../deploy/

../common/libcommon.a: $(shell find ../common/ -name '*.cpp') $(shell find ../common/ -name '*.c') $(shell find ../common/ -name '*.h')
	+make -C ../common/

version.h: version.h.in FORCE
	@cp version.h.in version.h.new
	@sed -i 's/$$COMMITID/$(shell git rev-parse HEAD)/g' version.h.new
	@sed -i 's/$$BRANCH/$(shell git rev-parse --abbrev-ref HEAD)/g' version.h.new
	@if [ ! -e version.h ] ; then \
		cp version.h.new version.h; \
	else \
		if ! diff version.h version.h.new >/dev/null ; then\
			cp version.h.new version.h;\
		fi \
	fi
	@rm version.h.new

FORCE: ;

-include $(OBJ:%=$(DEP_DIR)/%.d)
