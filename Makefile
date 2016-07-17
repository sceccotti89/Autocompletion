
# compiler
CC = g++
# macro
MACRO = -D VERBOSE -D _REENTRANT
# compiler options
CFLAGS = -std=c++11 -O3 -pedantic -Wall -Wextra -Wformat=2 -finline-functions -ggdb -fopenmp $(MACRO) -pthread -Wl,--no-as-needed #-I /home/stefano/boost_1_59_0
# linker options
LFLAGS = -fopenmp -lm -pthread -Wl,--no-as-needed

# libraries directory
LIBDIR = ./lib
# linking options
LIBS = -L $(LIBDIR)
# boost library
BOOST = -L /home/stefano/boost_1_59_0/stage/lib -static -lboost_thread -lboost_system

# ============================ custom libraries =============================== #

# library name
LIBNAME1 = libsearch.a
# files location
PATH1 = ./"Search Tree"/
# library objects
objects1 = searchTree.o

# library name
LIBNAME2 = libRMQ.a
# files location
PATH2 = ./RMQ/
# library objects
objects2 = lca.o naive_rmq.o opt_rmq.o pm_rmq.o sparse_rmq.o

# ============================================================================= #

exe = autocompletion

.PHONY: clean backup resume_backup lib1 lib2 all

# ============================ creates libraries ============================== #

lib1:
	$(CC) $(CFLAGS) -c $(addprefix $(PATH1), $(objects1:.o=.cpp))
	-rm  -f $(LIBDIR)/$(LIBNAME1)
	ar -r $(LIBNAME1) $(objects1)
	cp $(LIBNAME1) $(LIBDIR)

lib2:
	$(CC) $(CFLAGS) -c $(addprefix $(PATH2), $(objects2:.o=.cpp))
	-rm  -f $(LIBDIR)/$(LIBNAME2)
	ar -r $(LIBNAME2) $(objects2)
	cp $(LIBNAME2) $(LIBDIR)

# ============================================================================= #

# compiles the project
compile:
	$(CC) $(CFLAGS) -c start.cpp
	$(CC) $(LFLAGS) -o $(exe) start.o $(LIBS) -lsearch #$(BOOST)

# makes the backup of all the files
backup:
	cp *.cpp *.h *.hpp Makefile ./backup
	date > ./backup/last_modified.txt
	chmod 700 backup

# recovers data backup
resume_backup:
	cp -f ./backup/*.cpp ./backup/*h ./backup/*hpp .

# cleans the environment
clean:
	rm -rf *.o *.gch *.a *~ ./core ./lib/*.a

# entry point
all: 
	@echo "clean the environment"
	$(MAKE) clean
	#$(MAKE) -C ./"Search Tree"/ lib1
	@echo "creates the libraries..."
	$(MAKE) lib1
	@echo "library 1 created"
	#$(MAKE) lib2
	#$(MAKE) lib3
	#$(MAKE) $(exe)
	#-rm -f ./.mtrace.log
	#MALLOC_TRACE=./.mtrace.log ./$(exe)
	#mtrace ./$(exe) ./.mtrace.log
	#valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all ./$(exe)
	#valgrind --track-origins=yes ./$(exe)
	#perf record ./$(exe)
	#./$(exe)
	@echo "compile the project..."
	$(MAKE) compile
	@echo "project compiled"
	#valgrind --max-stackframe=2097216 --track-origins=yes ./$(exe) ./Datasets/Dictionary_Test.txt
	#valgrind --max-stackframe=2097216 --track-origins=yes ./$(exe) ./Datasets/Dictionary_Wikipedia.txt