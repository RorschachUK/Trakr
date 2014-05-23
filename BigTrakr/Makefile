# Makefile for TRAKR Toy
# Trakr Project

TRACKR_PATH = C:/Trackr
PROGRAM_NAME = BigTrakr
PRETTY_NAME = BigTrakr
OUTPUT_PATH = ./Intermediate
OUTPUT_NAME = $(OUTPUT_PATH)/$(PROGRAM_NAME).elf
INTERNALS_PATH = ../Internals

SOURCES = app.c
S_OBJECTS = $(OUTPUT_PATH)/app.o
O_IMAGES = Images/bigtrakr.o Images/rorschach.o Images/up.o Images/down.o Images/marker.o
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

Images/bigtrakr.o : Images/bigtrakr.bmp
	$(OC) -I binary -O elf32-littlearm -B arm $< $@

Images/rorschach.o : Images/rorschach.bmp
	$(OC) -I binary -O elf32-littlearm -B arm $< $@

Images/up.o : Images/up.bmp
	$(OC) -I binary -O elf32-littlearm -B arm $< $@

Images/down.o : Images/down.bmp
	$(OC) -I binary -O elf32-littlearm -B arm $< $@

Images/marker.o : Images/marker.bmp
	$(OC) -I binary -O elf32-littlearm -B arm $< $@

$(OUTPUT_NAME): $(OBJECTS) 
	@echo "Linking... "
	@echo "Creating file $@..."
	@$(CC) -u _start -o $@ $(INTERNALS_PATH)/trakr_start.a $(OBJECTS) $(TARGET_FLAG) $(LDFLAGS)

$(OUTPUT_PATH)/app.o:app.c Makefile
	@echo "Compiling $<"
	@$(CC) -c -o "$@" "$<" $(TARGET_FLAG) $(CFLAGS)

.PHONY: clean prebuild postbuild

clean:
	$(RM) -f $(PROGRAM_NAME).bin
	$(RM) -f "$(PRETTY_NAME).bin"
	$(RM) -f $(OUTPUT_PATH)/app.o
	$(RM) -f ../Images/bigtrakr.o
	$(RM) -f ../Images/rorschach.o
	$(RM) -f ../Images/up.o
	$(RM) -f ../Images/down.o
	$(RM) -f ../Images/marker.o
	$(RM) -f $(OUTPUT_NAME)
	$(RM) -f $(MKDEPFILE)

postbuild:
	arm-elf-objcopy -O binary $(OUTPUT_NAME) "$(PRETTY_NAME).bin"
	@if [ -d "E:/" ] ; then \
		cp "$(PRETTY_NAME).bin" c:/trakr/APPS/ ; \
    fi	
prebuild:
	



# End of Makefile