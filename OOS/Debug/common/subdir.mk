################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../common/AutoLock.cpp \
../common/Lock.cpp \
../common/Semaphore.cpp \
../common/Thread.cpp 

OBJS += \
./common/AutoLock.o \
./common/Lock.o \
./common/Semaphore.o \
./common/Thread.o 

CPP_DEPS += \
./common/AutoLock.d \
./common/Lock.d \
./common/Semaphore.d \
./common/Thread.d 


# Each subdirectory must supply rules for building sources it contributes
common/%.o: ../common/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


