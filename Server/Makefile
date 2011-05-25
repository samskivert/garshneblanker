NAME = Garshneblanker

CC = SC
LD = SLink

RM      = Delete
RMFLAGS = FORCE QUIET

CFLAGS  =
LDFLAGS = NODEBUG STRIPDEBUG NOICONS QUIET

SRCS = \
	cxhand.c \
	handlers.c \
	libraries.c \
	main.c \
	module.c \
	parse.c \
	prefs.c \
	resource.c \
	var.c

OBJS = $(SRCS:.c=.o)

all: $(NAME)

clean:
	$(RM) $(RMFLAGS) $(OBJS) $(NAME)

$(NAME): $(OBJS)
	$(LD) $(LDFLAGS) FROM $(OBJS) TO $(NAME)

revision:
	BumpRev 38 $(NAME)
	$(RM) $(RMFLAGS) $(NAME)_rev.i

.c.o:
	$(CC) $*.c
