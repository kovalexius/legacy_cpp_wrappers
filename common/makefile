PROJECT = libCommon.a
OBJS = Common.o ComPort.o Lockable.o Logger.o MLString.o PasswordArray.o PasswordKeeper.o Registry.o segvcatch.o Thread.o Timer.o Variant.o
INCLUDES = -I.

build: COMPILER = g++
build: LINKER1 = ar
build: LINKER2 = ranlib
build: $(OBJS)
	$(LINKER1) cr $(PROJECT) $(OBJS)
	$(LINKER2) $(PROJECT)

Common.o: Common.h Common.cpp 
	$(COMPILER) -c $(INCLUDES) Common.cpp 
ComPort.o: Common.h ComPort.h ComPort.cpp
	$(COMPILER) -c $(INCLUDES) ComPort.cpp 
Lockable.o: Error.h Lockable.h Lockable.cpp
	$(COMPILER) -c $(INCLUDES) Lockable.cpp 
Logger.o: Logger.h Logger.cpp
	$(COMPILER) -c $(INCLUDES) Logger.cpp 
MLString.o: MLString.h MLString.cpp
	$(COMPILER) -c $(INCLUDES) MLString.cpp 
PasswordArray.o: PasswordArray.h PasswordArray.cpp
	$(COMPILER) -c $(INCLUDES) PasswordArray.cpp 
PasswordKeeper.o: auto_array.h PasswordKeeper.h PasswordKeeper.cpp
	$(COMPILER) -c $(INCLUDES) PasswordKeeper.cpp 
Registry.o: tstring.h Registry.h Registry.cpp
	$(COMPILER) -c $(INCLUDES) Registry.cpp 
segvcatch.o: i386-signal.h x86_64-signal.h segvcatch.h segvcatch.cpp
	$(COMPILER) -c $(INCLUDES) segvcatch.cpp
Thread.o: Thread.h Thread.cpp
	$(COMPILER) -c $(INCLUDES) Thread.cpp 
Timer.o: Lockable.h Error.h Timer.h Timer.cpp
	$(COMPILER) -c $(INCLUDES) Timer.cpp 
Variant.o: Variant.h Variant.cpp
	$(COMPILER) -c $(INCLUDES) Variant.cpp 

clean:
	rm *.o $(PROJECT)

