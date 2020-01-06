CHARM_HOME=/Users/nitinbhat/Work/software/charm/netlrts-darwin-x86_64-prod
CHARMC=${CHARM_HOME}/bin/charmc $(OPTS)

LIVEVIZ_RUN=0

ifeq ($(LIVEVIZ_RUN), 1)
  CHARMC=${CHARM_HOME}/bin/charmc -module liveViz -DLIVEVIZ_RUN=1 $(OPTS)
else
  CHARMC=${CHARM_HOME}/bin/charmc -DLIVEVIZ_RUN=0 $(OPTS)
endif


MODE=solution

all: particle

OBJS = obj/main.o obj/$(MODE).o obj/custom_rand_gen.o obj/cell.o

N = 100
K = 4
ITER = 100
LBFREQ = 5

obj/cifiles: src/particleSimulation.ci
	$(CHARMC) src/particleSimulation.ci
	mv particleSimulation.def.h src/particleSimulation.def.h
	mv particleSimulation.decl.h src/particleSimulation.decl.h
	touch obj/cifiles

obj/main.o: src/main.cpp obj/cifiles src/main.h
	$(CHARMC) -c src/main.cpp -o obj/main.o

obj/cell.o: src/cell.cpp obj/cifiles src/cell.h
	$(CHARMC) -c src/cell.cpp -o obj/cell.o

obj/$(MODE).o: src/$(MODE).cpp obj/cifiles
	$(CHARMC) -c src/$(MODE).cpp -o obj/$(MODE).o

obj/custom_rand_gen.o: src/custom_rand_gen.c src/custom_rand_gen.h
	$(CHARMC) -c src/custom_rand_gen.c -o obj/custom_rand_gen.o

particle: $(OBJS)
	$(CHARMC) -O3 -language charm++ -o particle $(OBJS) -module CommonLBs

clean:
	rm -f src/*.decl.h src/*.def.h conv-host *.o obj/*.o particle charmrun obj/cifiles

outclean:
	rm -rf ./output

cleanp:
	rm -f *.sts *.gz *.projrc *.topo *.out

test: all
	./charmrun ./particle $(N) $(K) $(ITER) $(LBFREQ) ++local +p4 $(TESTOPTS)

testviz: all
	./charmrun ./particle $(N) $(K) $(ITER) $(LBFREQ) ++local +p4 ++server ++server-port 1234 $(TESTOPTS)
