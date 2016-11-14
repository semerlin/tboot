# lboot 版本号
MAJOR_VERSION = 1
MINOR_VERSION = 0
REVISION_NUMBER = 0
BUILD_NUMBER = 0303-alpha

LBOOT_VERSION = $(MAJOR_VERSION).$(MINOR_VERSION).$(REVISION_NUMBER).$(BUILD_NUMBER)


# lboot 版本头文件
VERSION_FILE = $(obj)include/version_lboot.h


# 指定shell类型，默认设置shell为bash，否则设置为sh
SHELL := $(shell if [ -x /bin/bash ]; \
				then \
					echo "/bin/bash"; \
				else \
					echo "/bin/sh";\
				fi;)
export SHELL

# make时带有 -s 参数时，不打印输出信息
ifeq (,$(findstring s,$(MAKEFLAGS)))
XECHO = echo
else
XECHO = :
endif


# 源码目录，编译输出文件目录
# 默认编译文件输出目录为out文件夹
# 可通过 make O=output 制定输出文件夹为output
# 当 make O= 为空时，设置为默认输出文件夹out
ifdef O
ifeq ("$(origin O)", "command line")
ifneq ($(O),)
BUILD_DIR := $(O)
else
BUILD_DIR := $(CURDIR)/out
endif
endif
else
BUILD_DIR := $(CURDIR)/out
endif

# 创建编译输出目录文件夹
save_dir := $(BUILD_DIR)
$(shell [ -d ${BUILD_DIR} ] || mkdir -p ${BUILD_DIR})

# 检测编译输出目录是否创建成功
BUILD_DIR := $(shell cd ${BUILD_DIR} && /bin/pwd)
$(if $(BUILD_DIR),,$(error output directory "$(save_dir)" does not exist))

# 编译文件输出目录
OBJTREE	:= $(BUILD_DIR)
#源代码目录
SRCTREE	:= $(CURDIR)
#顶层目录
TOPDIR	:= $(SRCTREE)
#链接库目录
LNDIR	:= $(OBJTREE)
export	OBJTREE SRCTREE TOPDIR LNDIR


# 进行电路板config配置的shell脚本
MKCONFIG := $(SRCTREE)/mkconfig
export MKCONFIG


# obj和src定义在config.mk文件中，在此处预先定义是为了可以使用
# unconfig, clean, clobber, distclean等命令
obj := $(OBJTREE)/
src := $(SRCTREE)/
export obj src

# 使CDPATH变量不产生影响
# 经常操作/etc目录下的若干个子目录的时候，我们就把CDPATH设置为/etc
# 这样每次转到/etc下的子目录时就不必加上/etc前缀
# #pwd
# /home/topsage
# #cd ssh
# -bash: cd: ssh: No such file or directory
# #export CDPATH=/etc
# cd ssh
# 这个时候cd ssh命令就会在基础目录/etc中寻找ssh子目录
unexport CDPATH

#########################################################################

# 工具和示例
SUBDIRS	= tools \
	  examples/standalone 

.PHONY : $(SUBDIRS)

# 判断config.mk文件是否存在，不存在就提示需要配置lboot
ifeq ($(OBJTREE)/include/config.mk,$(wildcard $(OBJTREE)/include/config.mk))

# 在config.mk之前包含autoconf.mk,这样所有的config参数都可以被下层调用
# 这里写一个空all的含义是: autoconf.mk.dep是autoconf.mk的生成规则和依赖项
# 也是一个目标
# 这样默认直接make就是执行autoconf.mk的生成规则
# 写一个空all就可避免此问题出现，此时会跳转到下一个all规则执行，进行
# 整个工程的编译
all:
sinclude $(obj)include/autoconf.mk.dep
sinclude $(obj)include/autoconf.mk

# 导入配置的电路板参数
include $(obj)include/config.mk
export	ARCH CPU BOARD VENDOR SOC


# 导入顶层综合配置
# 导入交叉编译指令等其它参数
include $(TOPDIR)/config.mk

#########################################################################
# start.o需要放在首位
OBJS  = arch/$(ARCH)/cpu/$(CPU)/start.o

OBJS := $(addprefix $(obj),$(OBJS))

LIBS  = lib/libgeneric.a
LIBS += lib/libfdt/libfdt.a
LIBS += arch/$(ARCH)/cpu/$(CPU)/lib$(CPU).a
ifdef SOC
LIBS += arch/$(ARCH)/cpu/$(CPU)/$(SOC)/lib$(SOC).a
endif
LIBS += arch/$(ARCH)/lib/lib$(ARCH).a
LIBS += net/libnet.a
LIBS += disk/libdisk.a
LIBS += drivers/dma/libdma.a
LIBS += drivers/mtd/libmtd.a
LIBS += drivers/mtd/nand/libnand.a
LIBS += drivers/net/libnet.a
LIBS += drivers/pcmcia/libpcmcia.a
LIBS += drivers/rtc/librtc.a
LIBS += drivers/video/libvideo.a
LIBS += common/libcommon.a

LIBS := $(addprefix $(obj),$(LIBS))
.PHONY : $(LIBS) $(TIMESTAMP_FILE) $(VERSION_FILE)

LIBBOARD = board/$(BOARDDIR)/lib$(BOARD).a
LIBBOARD := $(addprefix $(obj),$(LIBBOARD))

# OBJS LIBS LIBBOARD都是绝对路径
#######################################################################

# 添加GCC库文件，支持除法运算
# 输出arm-none-linux-gnueabi-gcc libgcc.a库文件的位置
# libgcc.a库文件支持除法运算
PLATFORM_LIBGCC = -L $(shell dirname `$(CC) $(CFLAGS) -print-libgcc-file-name`) -lgcc
PLATFORM_LIBS += $(PLATFORM_LIBGCC)
export PLATFORM_LIBS

# Special flags for CPP when processing the linker script.
# Pass the version down so we can handle backwards compatibility
# on the fly.
LDPPFLAGS += \
	-include $(TOPDIR)/include/u-boot/u-boot.lds.h \
	$(shell $(LD) --version | \
	  sed -ne 's/GNU ld version \([0-9][0-9]*\)\.\([0-9][0-9]*\).*/-DLD_MAJOR=\1 -DLD_MINOR=\2/p')

# __OBJS __LIBS为相对路径
__OBJS := $(subst $(obj),,$(OBJS))
__LIBS := $(subst $(obj),,$(LIBS)) $(subst $(obj),,$(LIBBOARD))

#########################################################################
#########################################################################

# 需要编译出来的文件
ALL += $(obj)u-boot.srec $(obj)u-boot.bin $(obj)System.map 

# 输入make all编译全部输出文件
all:		$(ALL)


# 生成uboot
$(obj)u-boot.srec:	$(obj)u-boot
		$(OBJCOPY) -O srec $< $@

$(obj)u-boot.bin:	$(obj)u-boot
		$(OBJCOPY) ${OBJCFLAGS} -O binary $< $@

# 生成uboot命令
GEN_UBOOT = \
		UNDEF_SYM=`$(OBJDUMP) -x $(LIBBOARD) $(LIBS) | \
		sed  -n -e 's/.*\($(SYM_PREFIX)__u_boot_cmd_.*\)/-u\1/p'|sort|uniq`;\
		cd $(LNDIR) && $(LD) $(LDFLAGS) $$UNDEF_SYM $(__OBJS) \
			--start-group $(__LIBS) --end-group $(PLATFORM_LIBS) \
			-Map u-boot.map -o u-boot

$(obj)u-boot:	depend $(SUBDIRS) $(OBJS) $(LIBBOARD) $(LIBS) $(LDSCRIPT) $(obj)u-boot.lds
		$(GEN_UBOOT)

$(OBJS):	depend
		$(MAKE) -C $(dir $(subst $(obj),,$@))

$(LIBS):	depend $(SUBDIRS)
		$(MAKE) -C $(dir $(subst $(obj),,$@))

$(LIBBOARD):	depend $(LIBS)
		$(MAKE) -C $(dir $(subst $(obj),,$@))

$(SUBDIRS):	depend
		$(MAKE) -C $@ all

$(LDSCRIPT):	depend
		$(MAKE) -C $(dir $@) $(notdir $@)

$(obj)u-boot.lds: $(LDSCRIPT)
		$(CPP) $(CPPFLAGS) $(LDPPFLAGS) -ansi -D__ASSEMBLY__ -P - <$^ >$@


$(VERSION_FILE):
	@echo "/* Automatically generated - do not edit! */" > $@
	@echo '#define LBOOT_VERSION      "$(LBOOT_VERSION)"' >> $@
	@LC_ALL=C date '+#define LBOOT_TIME         "%C%y-%m-%d %T"' >> $@

gdbtools:
		$(MAKE) -C tools/gdb all || exit 1

updater:
		$(MAKE) -C tools/updater all || exit 1

env:
		$(MAKE) -C tools/env all MTD_VERSION=${MTD_VERSION} || exit 1

depend dep:	$(VERSION_FILE) $(obj)include/autoconf.mk
		for dir in $(SUBDIRS) ; do $(MAKE) -C $$dir _depend ; done


SYSTEM_MAP = \
		$(NM) $1 | \
		grep -v '\(compiled\)\|\(\.o$$\)\|\( [aUw] \)\|\(\.\.ng$$\)\|\(LASH[RL]DI\)' | \
		LC_ALL=C sort
$(obj)System.map:	$(obj)u-boot
		@$(call SYSTEM_MAP,$<) > $(obj)System.map

#
# Auto-generate the autoconf.mk file (which is included by all makefiles)
#
# This target actually generates 2 files; autoconf.mk and autoconf.mk.dep.
# the dep file is only include in this top level makefile to determine when
# to regenerate the autoconf.mk file.
$(obj)include/autoconf.mk.dep: $(obj)include/config.h include/common.h
	@$(XECHO) Generating $@ ; \
	set -e ; \
	: Generate the dependancies ; \
	$(CC) -x c -DDO_DEPS_ONLY -M $(HOSTCFLAGS) $(CPPFLAGS) \
		-MQ $(obj)include/autoconf.mk include/common.h > $@

$(obj)include/autoconf.mk: $(obj)include/config.h
	@$(XECHO) Generating $@ ; \
	set -e ; \
	: Extract the config macros ; \
	$(CPP) $(CFLAGS) -DDO_DEPS_ONLY -dM include/common.h | \
		sed -n -f tools/scripts/define2mk.sed > $@.tmp && \
	mv $@.tmp $@

#########################################################################
else	# !config.mk
all $(obj)u-boot.srec $(obj)u-boot.bin \
$(obj)u-boot.img $(obj)u-boot.dis $(obj)u-boot \
$(SUBDIRS) $(VERSION_FILE) gdbtools updater env depend \
dep $(obj)System.map:
	@echo "System not configured - see README" >&2
	@ exit 1
endif	# config.mk


include/license.h: tools/bin2header COPYING
	 cat COPYING | gzip -9 -c | ./tools/bin2header license_gzip > include/license.h
#########################################################################

unconfig:
	@rm -f $(obj)include/config.h $(obj)include/config.mk \
		$(obj)board/*/config.tmp $(obj)board/*/*/config.tmp \
		$(obj)include/autoconf.mk $(obj)include/autoconf.mk.dep

#########################################################################
# Config Systems
#########################################################################

%_config : unconfig
	@$(MKCONFIG) $(subst _config,,$@)


#########################################################################
#########################################################################
#########################################################################

clean:
	@rm -f $(obj)examples/standalone/82559_eeprom			  \
	       $(obj)examples/standalone/eepro100_eeprom		  \
	       $(obj)examples/standalone/hello_world			  \
	       $(obj)examples/standalone/interrupt			  \
	       $(obj)examples/standalone/mem_to_mem_idma2intr		  \
	       $(obj)examples/standalone/sched				  \
	       $(obj)examples/standalone/smc91111_eeprom		  \
	       $(obj)examples/standalone/test_burst			  \
	       $(obj)examples/standalone/timer
	@rm -f $(obj)examples/api/demo{,.bin}
	@rm -f $(obj)tools/bmp_logo	   $(obj)tools/easylogo/easylogo  \
	       $(obj)tools/env/{fw_printenv,fw_setenv}			  \
	       $(obj)tools/envcrc					  \
	       $(obj)tools/gdb/{astest,gdbcont,gdbsend}			  \
	       $(obj)tools/gen_eth_addr    $(obj)tools/img2srec		  \
	       $(obj)tools/mkimage	   $(obj)tools/mpc86x_clk	  \
	       $(obj)tools/ncb		   $(obj)tools/ubsha1
	@rm -f $(obj)board/cray/L1/{bootscript.c,bootscript.image}	  \
	       $(obj)board/netstar/{eeprom,crcek,crcit,*.srec,*.bin}	  \
	       $(obj)board/trab/trab_fkt   $(obj)board/voiceblue/eeprom   \
	       $(obj)board/armltd/{integratorap,integratorcp}/u-boot.lds  \
	       $(obj)u-boot.lds						  \
	@rm -f $(obj)include/bmp_logo.h
	@rm -f $(VERSION_FILE)
	@find $(OBJTREE) -type f \
		\( -name 'core' -o -name '*.bak' -o -name '*~' \
		-o -name '*.o'	-o -name '*.a' -o -name '*.exe'	\) -print \
		| xargs rm -f

clobber:	clean
	@find $(OBJTREE) -type f \( -name .depend \
		-o -name '*.srec' -o -name '*.bin' -o -name u-boot.img \) \
		-print0 \
		| xargs -0 rm -f
	@rm -f $(OBJS) $(obj)*.bak $(obj)*.*~
	@rm -f $(obj)u-boot $(obj)u-boot.map $(obj)u-boot.hex $(ALL)
	@rm -f $(obj)tools/{env/crc32.c,inca-swap-bytes}
	@rm -f $(obj)include/asm/proc $(obj)include/asm/arch $(obj)include/asm
	@[ ! -d $(obj)nand_spl ] || find $(obj)nand_spl -name "*" -type l -print | xargs rm -f
	@[ ! -d $(obj)onenand_ipl ] || find $(obj)onenand_ipl -name "*" -type l -print | xargs rm -f

ifeq ($(OBJTREE),$(SRCTREE))
mrproper \
distclean:	clobber unconfig
else
mrproper \
distclean:	clobber unconfig
	rm -rf $(obj)*
endif

backup:
	F=`basename $(TOPDIR)` ; cd .. ; \
	gtar --force-local -zcvf `date "+$$F-%Y-%m-%d-%T.tar.gz"` $$F

#########################################################################
