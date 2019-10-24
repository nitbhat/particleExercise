CHARM_HOME=/scratch/nitin/charm/mpi-linux-x86_64-prod/
CHARMC=${CHARM_HOME}/bin/charmc $(OPTS)

MODE=solution

all: particle

OBJS = main.o $(MODE).o custom_rand_gen.o cell.o

N = 1000
K = 30

cifiles: particleSimulation.ci
	$(CHARMC) particleSimulation.ci
	touch cifiles

main.o: main.C cifiles
	$(CHARMC) -c main.C

cell.o: cell.C cifiles
	$(CHARMC) -c cell.C

$(MODE).o: $(MODE).C cifiles
	$(CHARMC) -c $(MODE).C

custom_rand_gen.o: custom_rand_gen.c
	$(CHARMC) -c custom_rand_gen.c

particle: $(OBJS)
	$(CHARMC) -O3 -language charm++ -o particle $(OBJS) -module CommonLBs

clean:
	rm -f *.decl.h *.def.h conv-host *.o particle charmrun cifiles

cleanp:
	rm -f *.sts *.gz *.projrc *.topo *.out

test: all
	./charmrun ./particle $(N) $(K) ++local +p4 $(TESTOPTS)
