CHARM_HOME=/scratch/nitin/charm_3/mpi-linux-x86_64-debug
CHARMC=${CHARM_HOME}/bin/charmc $(OPTS)

all: particle

OBJS = particle.o

N = 1000
K = 30

cifiles: particle.ci
	$(CHARMC) particle.ci
	touch cifiles

particle.o: particle.C cifiles
	$(CHARMC) -c particle.C

particle: $(OBJS)
	$(CHARMC) -O3 -language charm++ -o particle $(OBJS) -module CommonLBs

clean:
	rm -f *.decl.h *.def.h conv-host *.o particle charmrun cifiles

cleanp:
	rm -f *.sts *.gz *.projrc *.topo *.out

test: all
	./charmrun ./particle $(N) $(K) ++local +p4 $(TESTOPTS)
