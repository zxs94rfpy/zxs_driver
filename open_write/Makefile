ifeq ($(KERNELRELEASE),)
#
KERN_DIR := /home/csgec/6818GEC/kernel
# 获取当前的路径名
PWD := $(shell pwd)

CROSS_COMPILE := /home/csgec/6818GEC/prebuilts/gcc/linux-x86/arm/arm-eabi-4.8/bin/arm-eabi-

module:
	make -C $(KERN_DIR) ARCH=arm M=$(PWD) CROSS_COMPILE=$(CROSS_COMPILE)  modules

clean:
	rm -rf *.o
	rm -rf *.ko
	rm -rf modules.order  .tmp_versions Module.symvers
	rm -rf .*.cmd  *.mod.c
	
else
	
obj-m +=  w_r.o

endif