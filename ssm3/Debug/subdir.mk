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
../net_work_thread.c \
../ssm.c \
../ssm_bitmap.c \
../ssm_container.c \
../ssm_discovery.c \
../ssm_erasure_code.c \
../ssm_io.c \
../ssm_net.c \
../ssm_util.c 

OBJS += \
./main.o \
./net_client.o \
./net_common.o \
./net_crypto.o \
./net_msg.o \
./net_server.o \
./net_session.o \
./net_thread.o \
./net_work_thread.o \
./ssm.o \
./ssm_bitmap.o \
./ssm_container.o \
./ssm_discovery.o \
./ssm_erasure_code.o \
./ssm_io.o \
./ssm_net.o \
./ssm_util.o 

C_DEPS += \
./main.d \
./net_client.d \
./net_common.d \
./net_crypto.d \
./net_msg.d \
./net_server.d \
./net_session.d \
./net_thread.d \
./net_work_thread.d \
./ssm.d \
./ssm_bitmap.d \
./ssm_container.d \
./ssm_discovery.d \
./ssm_erasure_code.d \
./ssm_io.d \
./ssm_net.d \
./ssm_util.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -D_FILE_OFFSET_BITS=64 -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


