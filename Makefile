SRCDIR      := src
INCDIR      := src
BUILDDIR    := build
BINDIR      := bin
LIBDIR      := lib

AR=ar
CC=gcc
CCFLAGS=-std=c11 -W -Wextra -pedantic

OPTFLAGS = -O3 -mtune=native
DBGFLAGS = -DNDEBUG
SPARSEFLAG =

CCBUILD = $(OPTFLAGS) $(DBGFLAGS) $(SPARSEFLAG) -I$(INCDIR)

PROGS=degseq gcut gk gkbip gnd gnm gnp tomita naude

.PHONY: clean cleanall

all: $(BUILDDIR) $(BINDIR) $(LIBDIR) $(patsubst %, $(BINDIR)/%, $(PROGS))

o1: OPTFLAGS = -O1 -mtune=native
o1: all

o2: OPTFLAGS = -O2 -mtune=native
o2: all

o3: OPTFLAGS = -O3 -mtune=native
o3: all

sparse: SPARSEFLAG = -DSPARSESET
sparse: all

release: DBGFLAGS = -DNDEBUG
release: all

debug: DBGFLAGS = -DDEBUG -gdwarf -g3
debug: all

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(BINDIR): $(BUILDDIR)
	mkdir -p $(BINDIR)

$(LIBDIR): $(BUILDDIR)
	mkdir -p $(LIBDIR)

$(LIBDIR)/libdiscrete.a: $(BUILDDIR)/arena.o $(BUILDDIR)/bitmanip.o $(BUILDDIR)/dimacs.o $(BUILDDIR)/fatal.o $(BUILDDIR)/platform.o
	$(AR) rvs $@ $(filter %.o,$^)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CCBUILD) $(CCFLAGS) -c $(filter %.c,$^) -o $@



$(BUILDDIR)/degseq.o: $(SRCDIR)/graph-gen/degseq.c
	$(CC) $(CCBUILD) $(CCFLAGS) -c $(filter %.c,$^) -o $@

$(BINDIR)/degseq: $(BUILDDIR)/degseq.o $(LIBDIR)/libdiscrete.a
	$(CC) $(filter %.o,$^) -o $@ $(LIBDIR)/libdiscrete.a -lm


$(BUILDDIR)/gcut.o: $(SRCDIR)/graph-gen/gcut.c
	$(CC) $(CCBUILD) $(CCFLAGS) -c $(filter %.c,$^) -o $@

$(BINDIR)/gcut: $(BUILDDIR)/gcut.o $(LIBDIR)/libdiscrete.a
	$(CC) $(filter %.o,$^) -o $@ $(LIBDIR)/libdiscrete.a -lm


$(BUILDDIR)/gk.o: $(SRCDIR)/graph-gen/gk.c
	$(CC) $(CCBUILD) $(CCFLAGS) -c $(filter %.c,$^) -o $@

$(BINDIR)/gk: $(BUILDDIR)/gk.o $(LIBDIR)/libdiscrete.a
	$(CC) $(filter %.o,$^) -o $@ $(LIBDIR)/libdiscrete.a -lm


$(BUILDDIR)/gkbip.o: $(SRCDIR)/graph-gen/gkbip.c
	$(CC) $(CCBUILD) $(CCFLAGS) -c $(filter %.c,$^) -o $@

$(BINDIR)/gkbip: $(BUILDDIR)/gkbip.o $(LIBDIR)/libdiscrete.a
	$(CC) $(filter %.o,$^) -o $@ $(LIBDIR)/libdiscrete.a -lm


$(BUILDDIR)/gnd.o: $(SRCDIR)/graph-gen/gnd.c
	$(CC) $(CCBUILD) $(CCFLAGS) -c $(filter %.c,$^) -o $@

$(BINDIR)/gnd: $(BUILDDIR)/gnd.o $(LIBDIR)/libdiscrete.a
	$(CC) $(filter %.o,$^) -o $@ $(LIBDIR)/libdiscrete.a -lm


$(BUILDDIR)/gnm.o: $(SRCDIR)/graph-gen/gnm.c
	$(CC) $(CCBUILD) $(CCFLAGS) -c $(filter %.c,$^) -o $@

$(BINDIR)/gnm: $(BUILDDIR)/gnm.o $(LIBDIR)/libdiscrete.a
	$(CC) $(filter %.o,$^) -o $@ $(LIBDIR)/libdiscrete.a -lm


$(BUILDDIR)/gnp.o: $(SRCDIR)/graph-gen/gnp.c
	$(CC) $(CCBUILD) $(CCFLAGS) -c $(filter %.c,$^) -o $@

$(BINDIR)/gnp: $(BUILDDIR)/gnp.o $(LIBDIR)/libdiscrete.a
	$(CC) $(filter %.o,$^) -o $@ $(LIBDIR)/libdiscrete.a -lm


$(BUILDDIR)/benchmark.o: $(SRCDIR)/clique-enum/benchmark.c
	$(CC) $(CCBUILD) $(CCFLAGS) -c $(filter %.c,$^) -o $@

$(BUILDDIR)/clique_enum.o: $(SRCDIR)/clique-enum/clique_enum.c
	$(CC) $(CCBUILD) $(CCFLAGS) -c $(filter %.c,$^) -o $@


$(BUILDDIR)/clique_tomita.o: $(SRCDIR)/clique-enum/clique_tomita.c
	$(CC) $(CCBUILD) $(CCFLAGS) -c $(filter %.c,$^) -o $@

$(BUILDDIR)/tomita.o: $(SRCDIR)/clique-enum/tomita.c $(BUILDDIR)/clique_tomita.o
	$(CC) $(CCBUILD) $(CCFLAGS) -c $(filter %.c,$^) -o $@

$(BINDIR)/tomita: $(BUILDDIR)/tomita.o $(BUILDDIR)/clique_tomita.o $(BUILDDIR)/clique_enum.o $(BUILDDIR)/benchmark.o $(LIBDIR)/libdiscrete.a
	$(CC) $(filter %.o,$^) -o $@ $(LIBDIR)/libdiscrete.a -lm


$(BUILDDIR)/clique_naude.o: $(SRCDIR)/clique-enum/clique_naude.c
	$(CC) $(CCBUILD) $(CCFLAGS) -c $(filter %.c,$^) -o $@

$(BUILDDIR)/naude.o: $(SRCDIR)/clique-enum/naude.c $(BUILDDIR)/clique_naude.o
	$(CC) $(CCBUILD) $(CCFLAGS) -c $(filter %.c,$^) -o $@

$(BINDIR)/naude: $(BUILDDIR)/naude.o $(BUILDDIR)/clique_naude.o $(BUILDDIR)/clique_enum.o $(BUILDDIR)/benchmark.o $(LIBDIR)/libdiscrete.a
	$(CC) $(filter %.o,$^) -o $@ $(LIBDIR)/libdiscrete.a -lm

clean:
	rm -rf $(BUILDDIR)

cleanall: clean
	rm -rf $(LIBDIR)
	rm -rf $(BINDIR)
