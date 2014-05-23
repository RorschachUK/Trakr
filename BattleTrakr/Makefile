# Makefile for TRAKR Toy
# Trakr Project

TRACKR_PATH = C:/Trakr
PROGRAM_NAME = BattleTrakr
PRETTY_NAME = BattleTrakr
OUTPUT_PATH = ./Intermediate
OUTPUT_NAME = $(OUTPUT_PATH)/$(PROGRAM_NAME).elf
INTERNALS_PATH = ../Internals

SOURCES = app.c draw.c vector.c
S_OBJECTS = $(OUTPUT_PATH)/app.o $(OUTPUT_PATH)/draw.o $(OUTPUT_PATH)/vector.o
O_IMAGES = Images/battle.o Images/rorschach.o Images/tank.o

OBJECTS = $(S_OBJECTS) $(O_IMAGES) $(INTERNALS_PATH)/trakr.a

SHELL = sh
CC 	= arm-elf-gcc 
AS 	= arm-elf-as 
BIN = arm-elf-ld 
LD 	= arm-elf-ld 
OC  = arm-elf-objcopy

TOPMEMORY=0xFFE7C000
CFLAGS = -O0 -I../Internals/Include -I../Internals -Wall -gstabs+  
TARGET_FLAG = -mcpu=arm926ejs -mapcs-32 -mlittle-endian -specs=specs.semi  
LDFLAGS = -T ../Internals/WJ_APP_8M.ld -Wl,--defsym -Wl,__stack_base=$(TOPMEMORY) -Wl,-Map -Wl,$(basename $@).map -nostartfiles -static  

all: prebuild $(OUTPUT_NAME) postbuild 

Images/battle.o : Images/battle.bmp
	$(OC) -I binary -O elf32-littlearm -B arm $< $@

Images/rorschach.o : Images/rorschach.bmp
	$(OC) -I binary -O elf32-littlearm -B arm $< $@

Images/tank.o : Images/tank.bmp
	$(OC) -I binary -O elf32-littlearm -B arm $< $@

$(OUTPUT_NAME): $(OBJECTS) 
	@echo "Linking... "
	@echo "Creating file $@..."
	@$(CC) -u _start -o $@ $(INTERNALS_PATH)/trakr_start.a $(OBJECTS) $(TARGET_FLAG) $(LDFLAGS)

$(OUTPUT_PATH)/app.o:app.c Makefile
	@echo "Compiling $<"
	@$(CC) -c -o "$@" "$<" $(TARGET_FLAG) $(CFLAGS)

$(OUTPUT_PATH)/draw.o:draw.c Makefile
	@echo "Compiling $<"
	@$(CC) -c -o "$@" "$<" $(TARGET_FLAG) $(CFLAGS)

$(OUTPUT_PATH)/vector.o:vector.c Makefile
	@echo "Compiling $<"
	@$(CC) -c -o "$@" "$<" $(TARGET_FLAG) $(CFLAGS)

.PHONY: clean prebuild postbuild

clean:
	$(RM) -f $(PROGRAM_NAME).bin
	$(RM) -f "$(PRETTY_NAME).bin"
	$(RM) -f $(OUTPUT_PATH)/app.o
	$(RM) -f $(OUTPUT_PATH)/draw.o
	$(RM) -f $(OUTPUT_PATH)/vector.o
	$(RM) -f ../Images/battle.o
	$(RM) -f ../Images/rorschach.o
	$(RM) -f ../Images/tank.o
	$(RM) -f $(OUTPUT_NAME)
	$(RM) -f $(MKDEPFILE)

postbuild:
	arm-elf-objcopy -O binary $(OUTPUT_NAME) "$(PRETTY_NAME).bin"
	@if [ -d "F:/" ] ; then \
		cp "$(PRETTY_NAME).bin" F:/APPS/ ; \
    fi	
prebuild:
	



# End of Makefile