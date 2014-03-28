################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../main.c \
../net_client.c \
../net_common.c \
../net_crypto.c \
../net_msg.c \
../net_server.c \
../net_session.c \
../net_thread.c \
../net_work_thread.c 

OBJS += \
./main.o \
./net_client.o \
./net_common.o \
./net_crypto.o \
./net_msg.o \
./net_server.o \
./net_session.o \
./net_thread.o \
./net_work_thread.o 

C_DEPS += \
./main.d \
./net_client.d \
./net_common.d \
./net_crypto.d \
./net_msg.d \
./net_server.d \
./net_session.d \
./net_thread.d \
./net_work_thread.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -D _FILE_OFFSET_BITS=64 -I/usr/lib/gcc/x86_64-linux-gnu/4.6/include -I/usr/local/include -I/usr/lib/gcc/x86_64-linux-gnu/4.6/include-fixed -I/usr/include/x86_64-linux-gnu -I/usr/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


