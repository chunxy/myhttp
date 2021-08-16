# CC = gcc
# FLAGS = -g -Wall
# INCLUDEPATH = -L http/
# 
# COMMON = net.o conn.o parser.o
# REACTOR = eventloop.o threadpool.o
# 
# server: main.o $(COMMON) $(REACTOR)
# 	$(CC) $(FLAGS) $(INCLUDEPATH) main.c $(OBJS) -o server
# 
# main.o: main.c
# 	$(CC) $(FLAGS) -c $<
# 
# $(COMMON):
# 
# $(REACTOR): %.c %.h event.h
# 	$(CC) $(FLAGS) -c $<
