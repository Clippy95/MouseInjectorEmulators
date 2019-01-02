#Mouse Injector Makefile
#Use with TDM-GCC 4.9.2-tdm-3 or MinGW
#For portable MinGW download Orwell Dev-C++
#mingw32-make.exe -f makefile to compile

#Compiler directories
MINGWDIR = C:/MinGW64/bin/
CC = $(MINGWDIR)gcc
WINDRES = $(MINGWDIR)windres

#Source directories
SRCDIR = ./
MANYMOUSEDIR = $(SRCDIR)manymouse/
GAMESDIR = $(SRCDIR)games/
OBJDIR = $(SRCDIR)obj/
EXENAME = "$(SRCDIR)Mouse Injector.exe"

#Compiler flags
CFLAGS = -ansi -O2 -m64 -std=c99 -Wall
WARNINGS = -Wextra -pedantic -Wno-parentheses
RESFLAGS = -F pe-x86-64 --input-format=rc -O coff

#Linker flags
OBJS = $(OBJDIR)main.o $(OBJDIR)memory.o $(OBJDIR)mouse.o $(OBJDIR)manymouse.o $(OBJDIR)windows_wminput.o $(OBJDIR)icon.res
GAMEOBJS = $(OBJDIR)game.o $(OBJDIR)ts2.o $(OBJDIR)ts3.o $(OBJDIR)nf.o $(OBJDIR)mohf.o $(OBJDIR)dhv.o
LIBS = -static-libgcc -lpsapi
LFLAGS = $(OBJS) $(GAMEOBJS) -o $(EXENAME) $(LIBS) -m64 -s

#Main recipes
mouseinjector: $(OBJS) $(GAMEOBJS)
	$(CC) $(LFLAGS)

all: clean mouseinjector

#Individual recipes
$(OBJDIR)main.o: $(SRCDIR)main.c $(SRCDIR)main.h $(SRCDIR)memory.h $(SRCDIR)mouse.h $(GAMESDIR)game.h
	$(CC) -c $(SRCDIR)main.c -o $(OBJDIR)main.o $(CFLAGS) $(WARNINGS)

$(OBJDIR)memory.o: $(SRCDIR)memory.c $(SRCDIR)memory.h
	$(CC) -c $(SRCDIR)memory.c -o $(OBJDIR)memory.o $(CFLAGS) $(WARNINGS)

$(OBJDIR)mouse.o: $(SRCDIR)mouse.c $(SRCDIR)mouse.h $(MANYMOUSEDIR)manymouse.h
	$(CC) -c $(SRCDIR)mouse.c -o $(OBJDIR)mouse.o $(CFLAGS) $(WARNINGS)

$(OBJDIR)manymouse.o: $(MANYMOUSEDIR)manymouse.c $(MANYMOUSEDIR)manymouse.h
	$(CC) -c $(MANYMOUSEDIR)manymouse.c -o $(OBJDIR)manymouse.o $(CFLAGS) $(WARNINGS)

$(OBJDIR)windows_wminput.o: $(MANYMOUSEDIR)windows_wminput.c $(MANYMOUSEDIR)manymouse.h
	$(CC) -c $(MANYMOUSEDIR)windows_wminput.c -o $(OBJDIR)windows_wminput.o $(CFLAGS)

$(OBJDIR)icon.res: $(SRCDIR)icon.rc $(SRCDIR)icon.ico
	$(WINDRES) -i $(SRCDIR)icon.rc -o $(OBJDIR)icon.res $(RESFLAGS)

#Game drivers recipe
$(OBJDIR)%.o: $(GAMESDIR)%.c $(SRCDIR)main.h $(SRCDIR)memory.h $(SRCDIR)mouse.h $(GAMESDIR)game.h
	$(CC) -c $< -o $@ $(CFLAGS) $(WARNINGS)

clean:
	rm -f $(SRCDIR)*.exe $(OBJDIR)*.o $(OBJDIR)*.res