#Variables utilisées :
CC=gcc
LD=gcc
CCFLAGS=-Wall -g
EDLFLAGS=-g
EXE=gt
DAEMON=gtd

OBJ=gt.o
DOBJ=gtd.o serial.o datalist.o

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

install :
	rm -rf /usr/local/share/gtd
	mkdir /usr/local/share/gtd
	cp ./gtd /usr/local/bin/
	cp ./gt /usr/local/bin/
	echo "You need to manually add an alias in your .zshrc (or equivalent) parse your cd inputs with gt"
