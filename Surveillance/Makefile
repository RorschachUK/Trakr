# Makefile for TRAKR Toy
# Trakr Project

TRACKR_PATH = C:/Trakr
PROGRAM_NAME = Surveillance
PRETTY_NAME = Surveillance
OUTPUT_PATH = ./Intermediate
OUTPUT_NAME = $(OUTPUT_PATH)/$(PROGRAM_NAME).elf
INTERNALS_PATH = ../Internals

SOURCES = app.c camera.c
S_OBJECTS = $(OUTPUT_PATH)/app.o $(OUTPUT_PATH)/camera.o 
O_IMAGES = Images/camb.o Images/camw.o Images/rorschach.o

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

Images/camb.o : Images/camb.bmp
	$(OC) -I binary -O elf32-littlearm -B arm $< $@

Images/camw.o : Images/camw.bmp
	$(OC) -I binary -O elf32-littlearm -B arm $< $@

Images/rorschach.o : Images/rorschach.bmp
	$(OC) -I binary -O elf32-littlearm -B arm $< $@

$(OUTPUT_NAME): $(OBJECTS) 
	@echo "Linking... "
	@echo "Creating file $@..."
	@$(CC) -u _start -o $@ $(INTERNALS_PATH)/trakr_start.a $(OBJECTS) $(TARGET_FLAG) $(LDFLAGS)

$(OUTPUT_PATH)/app.o:app.c Makefile
	@echo "Compiling $<"
	@$(CC) -c -o "$@" "$<" $(TARGET_FLAG) $(CFLAGS)

$(OUTPUT_PATH)/camera.o:camera.c Makefile
	@echo "Compiling $<"
	@$(CC) -c -o "$@" "$<" $(TARGET_FLAG) $(CFLAGS)

.PHONY: clean prebuild postbuild

clean:
	$(RM) -f $(PROGRAM_NAME).bin
	$(RM) -f "$(PRETTY_NAME).bin"
	$(RM) -f $(OUTPUT_PATH)/app.o
	$(RM) -f $(OUTPUT_PATH)/camera.o
	$(RM) -f ../Images/camb.o
	$(RM) -f ../Images/camw.o
	$(RM) -f ../Images/rorschach.o
	$(RM) -f $(OUTPUT_NAME)
	$(RM) -f $(MKDEPFILE)

postbuild:
	arm-elf-objcopy -O binary $(OUTPUT_NAME) "$(PRETTY_NAME).bin"
	@if [ -d "E:/" ] ; then \
		cp "$(PRETTY_NAME).bin" c:/trakr/APPS/ ; \
    fi	
prebuild:
	



# End of Makefile