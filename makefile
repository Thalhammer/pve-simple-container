SRC := $(shell find . -name '*.cpp') $(shell find . -name '*.c')
EXCLUDE_SRC := 
FSRC := $(filter-out $(EXCLUDE_SRC), $(SRC))
OBJ := $(FSRC:%=$(BUILDDIR)/obj/%.o)

DEP_DIR = .deps

FLAGS ?= -fPIC -Wall -Wno-unknown-pragmas
CXXFLAGS ?= -std=c++14
CFLAGS ?=
STATICLIBS ?= -lstdc++fs -pthread -lssl -lcrypto -lz
LIBS ?= -ldl -lcurl
LINKFLAGS ?=

OUTNAME = pvesc

ARCH := $(shell getconf LONG_BIT)
GITVERSION := $(shell git describe --tags --long | cut -d- -f1,2 | cut -c2-)
DEBVERSION ?= $(firstword $(subst -, ,$(GITVERSION)))
DEBITERATION ?= $(or $(word 2,$(subst -, ,$(GITVERSION))),0)
DEBFOLDER ?= $(OUTNAME)_$(DEBVERSION)-$(DEBITERATION)

FPM := fpm --verbose -s dir -t deb -v $(DEBVERSION) --iteration $(DEBITERATION) -m 'Dominik Thalhammer <dominik@thalhammer.it>' --vendor 'Dominik Thalhammer <dominik@thalhammer.it>' --url 'https://github.com/Thalhammer/pve-simple-container' -C $(DEBFOLDER)

.PHONY: clean debug release debug_static release_static

all: release debug

release: release_shared release_static
debug: debug_shared debug_static

debug_static: FLAGS += -g -Og -fsanitize=address -fno-omit-frame-pointer
debug_static: LINKFLAGS += -lasan
debug_static:
	$(eval BUILDDIR=debug/static)
	$(eval LINKFLAGS += -Wl,-Bstatic $(STATICLIBS) -Wl,-Bdynamic $(LIBS))
	$(eval FLAGS += -I $(BUILDDIR)/include/)
	$(eval export FLAGS)
	$(eval export LINKFLAGS)
	$(eval export BUILDDIR)
	@+make $(BUILDDIR)/$(OUTNAME) --no-print-directory

release_static: FLAGS += -O2 -flto
release_static:
	$(eval BUILDDIR=release/static)
	$(eval LINKFLAGS += -Wl,-Bstatic $(STATICLIBS) -Wl,-Bdynamic $(LIBS))
	$(eval FLAGS += -I $(BUILDDIR)/include/)
	$(eval export FLAGS)
	$(eval export LINKFLAGS)
	$(eval export BUILDDIR)
	@+make $(BUILDDIR)/$(OUTNAME) --no-print-directory
	@strip --strip-unneeded $(BUILDDIR)/$(OUTNAME)

debug_shared: FLAGS += -g -Og -fPIC -fsanitize=address -fno-omit-frame-pointer
debug_shared: LINKFLAGS += -lasan
debug_shared:
	$(eval BUILDDIR=debug/shared)
	$(eval LINKFLAGS=$(STATICLIBS) $(LIBS))
	$(eval FLAGS += -I $(BUILDDIR)/include/)
	$(eval export FLAGS)
	$(eval export LINKFLAGS)
	$(eval export BUILDDIR)
	@+make $(BUILDDIR)/$(OUTNAME) --no-print-directory

release_shared: FLAGS += -O2 -fPIC -flto
release_shared:
	$(eval BUILDDIR=release/shared)
	$(eval LINKFLAGS += $(STATICLIBS) $(LIBS))
	$(eval FLAGS += -I $(BUILDDIR)/include/)
	$(eval export FLAGS)
	$(eval export LINKFLAGS)
	$(eval export BUILDDIR)
	@+make $(BUILDDIR)/$(OUTNAME) --no-print-directory
	@strip --strip-unneeded $(BUILDDIR)/$(OUTNAME)

$(BUILDDIR)/$(OUTNAME): $(OBJ)
	@echo Generating $@
	@$(CXX) -o $@ $^ $(LINKFLAGS)
	@echo Build done

$(BUILDDIR)/obj/%.cc.o: %.cc  $(BUILDDIR)/include/version.h
	@echo Building $<
	@mkdir -p `dirname $@`
	@$(CXX) -c $(FLAGS) $(CXXFLAGS) $< -o $@
	@mkdir -p `dirname $(DEP_DIR)/$@.d`
	@$(CXX) -c $(FLAGS) $(CXXFLAGS) -MT '$@' -MM $< > $(DEP_DIR)/$@.d

$(BUILDDIR)/obj/%.cpp.o: %.cpp $(BUILDDIR)/include/version.h
	@echo Building $<
	@mkdir -p `dirname $@`
	@$(CXX) -c $(FLAGS) $(CXXFLAGS) $< -o $@
	@mkdir -p `dirname $(DEP_DIR)/$@.d`
	@$(CXX) -c $(FLAGS) $(CXXFLAGS) -MT '$@' -MM $< > $(DEP_DIR)/$@.d

$(BUILDDIR)/obj/%.c.o: %.c $(BUILDDIR)/include/version.h
	@echo Building $<
	@mkdir -p `dirname $@`
	@$(CC) -c $(FLAGS) $(CFLAGS) $< -o $@
	@mkdir -p `dirname $(DEP_DIR)/$@.d`
	@$(CC) -c $(FLAGS) $(CFLAGS) -MT '$@' -MM $< > $(DEP_DIR)/$@.d

clean:
	@echo Removing binary
	@rm -rf release
	@rm -rf debug
	@rm -f version.h
	@echo Removing dependency files
	@rm -rf $(DEP_DIR)

$(BUILDDIR)/include/version.h: ./pvesc/version.h.in | FORCE
	@mkdir -p $(BUILDDIR)/include/
	@cp $^ $@.new
	@sed -i 's/$$COMMITID/$(shell git rev-parse HEAD)/g' $@.new
	@sed -i 's/$$BRANCH/$(shell git rev-parse --abbrev-ref HEAD)/g' $@.new
	@if [ ! -e $@ ] ; then \
		cp $@.new $@; \
	else \
		if ! diff $@ $@.new >/dev/null ; then\
			cp $@.new $@;\
		fi \
	fi
	@rm $@.new

FORCE: ;

-include $(OBJ:%=$(DEP_DIR)/%.d)
