SOURCES = test/runtests.cpp \
          test/fakeString.cpp \
          test/fakePrint.cpp \
          test/fakeStatus.cpp \
          test/mockArduino.cpp \
          src/extendedPrinter.cpp \
          src/lineStepCalculator.cpp \
          src/plotterController.cpp \
          src/point.cpp \
          src/polarPlotter.cpp \
          src/step.cpp \
          src/stepBank.cpp

OBJECTS := $(addsuffix .o, $(addprefix .build/, $(basename $(SOURCES))))
DEPFILES := $(subst .o,.dep, $(subst .build/,.deps/, $(OBJECTS)))
TESTCPPFLAGS = -D__IN_TEST__ -Isrc -Itest
CPPDEPFLAGS = -MMD -MP -MF .deps/$(basename $<).dep
RUNTEST := $(if $(COMSPEC), runtest.exe, runtest)

all: runtests

.build/%.o: %.cpp
	mkdir -p .deps/$(dir $<)
	mkdir -p .build/$(dir $<)
	$(COMPILE.cpp) $(TESTCPPFLAGS) $(CPPDEPFLAGS) -o $@ $<

runtests: $(OBJECTS)
	$(CC) -g $(OBJECTS) -lstdc++ -o $@

clean:
	@rm -rf .deps/ .build/ $(RUNTEST)

-include $(DEPFILES)
