# Generic MinGW makefile (C source only)
# Modify variables/macros to fit your program

# make / make all | will compile your program.
# make clean      | deletes all compiled object and executable files.
# make depend     | rebuilds the dependancy list
# make run        | compiles (if needed) and runs your program

# Compiler command
CC = g++

# Linker command
LD = g++

# Flags to pass to the compiler - add "-g" to include debug information
CFLAGS = -Wall

# Flags to pass to the linker
LDFLAGS = -L"D:\MinGW\lib" -lstdc++ -lWs2_32

# Command used to delete files
RM = del

# List your object files here
OBJS = main.o socket.o imexplode.o

# List your source files here
SRCS = main.cpp socket.cpp imexplode.cpp

# Define your compile target here.
PROG = ChatServer.exe

# Compile everything.
all: $(PROG)

# Link the program
$(PROG): $(OBJS)
	$(LD) $(OBJS) -o $(PROG) $(LDFLAGS)

# All .o files depend on their corresponding .c file
%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	$(RM) $(PROG)
	$(RM) *.o
	$(RM) depend

depend:
	$(CC) $(CFLAGS) -MM $(SRCS) > depend

run: all
	$(PROG)

include depend
