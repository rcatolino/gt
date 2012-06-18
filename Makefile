#Variables utilisées :
CC=gcc 
LD=gcc 
CCFLAGS=-Wall 
EDLFLAGS=
EXE=gt
DAEMON=gtd

OBJ=gt.o
DOBJ=gtd.o	

LIBS=
DLIBS=
all : $(EXE) $(DAEMON)

$(DAEMON): $(DOBJ) 
	@echo building $<
	$(LD) -o $(DAEMON) $(EDLFLAGS) $(DOBJ) $(DLIBS)
	@echo done

$(EXE): $(OBJ) 
	@echo building $<
	$(LD) -o $(EXE) $(EDLFLAGS) $(OBJ) $(LIBS)
	@echo done

%.o : %.c *.h 
	@echo building $< ...
	$(CC) $(CCFLAGS) -c $<
	@echo done
	
clean: 
	@echo -n cleaning repository... 
	@rm -f ./$(EXE)
	@rm -f *.o
	@rm -f .*.swp
	@rm -f *~
	@rm -f *.log
	@rm -f *.pid
	@rm -f *.out
	@echo cleaned.

coffee : clean
	@echo No!
