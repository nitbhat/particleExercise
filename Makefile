CHARM_HOME=/Users/nitinbhat/Work/software/charm/netlrts-darwin-x86_64-prod
CHARMC=${CHARM_HOME}/bin/charmc $(OPTS)
MODE=solution

BONUS_QUESTION=0# Set to 1 to compile bonus question part
# make clean all after changing BONUS_QUESTION variable

LIVEVIZ_RUN=0# Set to 1 to turn on visualization
# make clean all after changing LIVEVIZ_RUN variable

CHARMC=${CHARM_HOME}/bin/charmc

#$(info $$BONUS_QUESTION is [${BONUS_QUESTION}])

ifeq ($(LIVEVIZ_RUN), 1)
  CHARMC += -module liveViz -DLIVEVIZ_RUN=1
else
  CHARMC += -DLIVEVIZ_RUN=0
endif

ifeq ($(BONUS_QUESTION), 1)
  CHARMC += -DBONUS_QUESTION=1
else
  CHARMC += -DBONUS_QUESTION=0
endif

CHARMC += $(OPTS)

all: particle

OBJS = obj/main.o obj/$(MODE).o obj/custom_rand_gen.o obj/cell.o

N = 100
K = 4
ITER = 100
LBFREQ = 5
PARTICLEDIST = 1,2,3,10
VELFACT = 5
LOGOUTPUT=yes

obj/cifiles: src/particleSimulation.ci src/particle.h
	$(CHARMC) src/particleSimulation.ci
	mv particleSimulation.def.h src/particleSimulation.def.h
	mv particleSimulation.decl.h src/particleSimulation.decl.h
	touch obj/cifiles

obj/main.o: src/main.cpp obj/cifiles src/main.h src/particle.h src/cell.h
	$(CHARMC) -c src/main.cpp -o obj/main.o

obj/cell.o: src/cell.cpp obj/cifiles src/cell.h src/particle.h
	$(CHARMC) -c src/cell.cpp -o obj/cell.o

obj/$(MODE).o: src/$(MODE).cpp obj/cifiles src/particle.h src/cell.h src/main.h
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
	./charmrun +p4 ./particle $(N) $(K) $(ITER) $(PARTICLEDIST) $(VELFACT) $(LOGOUTPUT) $(LBFREQ) $(TESTOPTS)

testbench: all
	./charmrun +p96 ./particle 10000 35 1000 1,2,30,10 5 no $(LBFREQ) $(TESTOPTS)

testviz: all
	./charmrun +p4 ./particle 10000 10 100000 $(PARTICLEDIST) 100 no $(LBFREQ) ++server ++server-port 1234 $(TESTOPTS)
