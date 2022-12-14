OPTC = -O3 -DB_64 -ISAPPOROBDD/include
OPTCPP = -std=c++11 -O3 -DB_64 -ISAPPOROBDD/include -ITdZdd/include

HEADER = AdjacentSpec.hpp BigInteger.hpp Clique.hpp DominatingSet.hpp ForestOrTree.hpp FrontierDegreeSpecified.hpp FrontierManager.hpp IndependentSet.hpp Matching.hpp MaxEval.hpp Option.hpp Path.hpp RainbowSpec.hpp RandomSample.hpp Reconf.hpp SolutionSpace.hpp Utility.hpp VariableConditionSpec.hpp ZBDD_reconf.hpp

ddreconf: main.cpp bddc_reconf.o BDD.o ZBDD_reconf.o $(HEADER)
	g++ $(OPTCPP) main.cpp bddc_reconf.o BDD.o ZBDD_reconf.o -o ddreconf

bddc_reconf.o: bddc_reconf.c bddc_reconf.h
	gcc $(OPTC) bddc_reconf.c -c -o bddc_reconf.o

BDD.o: SAPPOROBDD/src/BDD+/BDD.cc SAPPOROBDD/include/BDD.h
	g++ $(OPTCPP) SAPPOROBDD/src/BDD+/BDD.cc -c -o BDD.o

ZBDD_reconf.o: ZBDD_reconf.cpp ZBDD_reconf.hpp
	g++ $(OPTCPP) ZBDD_reconf.cpp -c -o ZBDD_reconf.o
clean:
	rm -rf *.o ddreconf ddreconf.exe
