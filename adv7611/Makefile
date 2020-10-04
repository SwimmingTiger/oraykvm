
ifeq ($(PARAM_FILE), )
		PARAM_FILE:=../../Makefile.param
		include $(PARAM_FILE)
endif

obj-m := adv7611.o
Adv7611-y += adv7611.o 

EXTRA_CFLAGS+=-I$(PWD)/

default:	
	@$(MAKE) -C $(LINUX_ROOT) M=$(PWD) modules 
	arm-hisiv300-linux-gcc -o test_adv7611  test_adv7611.c
	cp adv7611.ko test_adv7611  /home/hzy/share
clean:
	@make -C $(LINUX_ROOT) M=$(PWD) clean 
	rm test_adv7611

