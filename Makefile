CHARM_HOME=/scratch/nitin/charm/mpi-linux-x86_64-prod/
CHARMC=${CHARM_HOME}/bin/charmc $(OPTS)

MODE=solution

all: particle

OBJS = main.o $(MODE).o custom_rand_gen.o cell.o

N = 100
K = 4

cifiles: particleSimulation.ci
	$(CHARMC) particleSimulation.ci
	touch cifiles

main.o: main.cpp cifiles main.h
	$(CHARMC) -c main.cpp

cell.o: cell.cpp cifiles cell.h
	$(CHARMC) -c cell.cpp

$(MODE).o: $(MODE).cpp cifiles
	$(CHARMC) -c $(MODE).cpp

custom_rand_gen.o: custom_rand_gen.c custom_rand_gen.h
	$(CHARMC) -c custom_rand_gen.c

particle: $(OBJS)
	$(CHARMC) -O3 -language charm++ -o particle $(OBJS) -module CommonLBs

clean:
	rm -f *.decl.h *.def.h conv-host *.o particle charmrun cifiles

outclean:
	rm -rf ./output

cleanp:
	rm -f *.sts *.gz *.projrc *.topo *.out

test: all
	./charmrun ./particle $(N) $(K) ++local +p4 $(TESTOPTS)
