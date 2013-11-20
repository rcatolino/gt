#Variables utilisées :
CC=gcc
LD=gcc
CCFLAGS=-Wall -O0 -g -std=gnu99
EDLFLAGS=-g -O0 -std=gnu99
EXE=gt
TEST=test
DAEMON=gtd

OBJ=gt.o
DOBJ=gtd.o serial.o datalist.o
TOBJ=datalist.o serial.o test.o

LIBS=
DLIBS=
all : $(EXE) $(DAEMON)

$(TEST): $(TOBJ)
	@echo building $<
	$(LD) -o $(TEST) $(EDLFLAGS) $(TOBJ)
	@echo done

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

install :
	rm -rf /usr/local/share/gtd/
	mkdir /usr/local/share/gtd/
	cp ./gtd /usr/local/bin/
	cp ./gt /usr/local/bin/
	@echo "You need to manually add an alias in your .zshrc (or equivalent) to query your cd inputs with gt"
