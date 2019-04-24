APP     = test.out

SRCDIR  = ../src/zutil
OBJDIR  = .obj/main

SRCS    := main.cpp $(shell find $(SRCDIR) -name '*.cpp') $(shell find $(SRCDIR) -name '*.c') 
OBJS    := $(patsubst %,$(OBJDIR)/%.o,$(SRCS))

CFLAGS  = -I. -I../inc
LDFLAGS = -llzma -lz -llzo2

all: $(APP)

$(APP) : $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -g -o $@

$(OBJDIR)/%.cpp.o: %.cpp
	@mkdir -p $(OBJDIR)/$(dir $<)
	$(CXX) $(CFLAGS) -g -c $< -o $@

$(OBJDIR)/%.c.o: %.c
	@mkdir -p $(OBJDIR)/$(dir $<)
	$(CC) $(CFLAGS) -g -c $< -o $@

clean:
	# rm -rf test.big.* $(APP) $(OBJDIR) test.gz test.lzo test.xz test.txt
	rm -rf $(APP) .obj

