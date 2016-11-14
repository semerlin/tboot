# 生成当前编译目录，更新obj和src参数
ifeq ($(CURDIR),$(SRCTREE))
curdir :=
else
curdir := $(subst $(SRCTREE)/,,$(CURDIR))
endif

obj := $(if $(curdir),$(OBJTREE)/$(curdir)/,$(OBJTREE)/)
src := $(if $(curdir),$(SRCTREE)/$(curdir)/,$(SRCTREE)/)

$(shell mkdir -p $(obj))

# 清除和各个平台相关的编译标志
# 具体定义在各个平台相关文件夹的config.mk中定义
PLATFORM_RELFLAGS =
PLATFORM_CPPFLAGS =
PLATFORM_LDFLAGS =

#########################################################################

HOSTCC		= gcc
HOSTCFLAGS	= -Wall -Wstrict-prototypes -O2 -fomit-frame-pointer
HOSTSTRIP	= strip

#########################################################################
#
# 检测传入的编译标志是否有效，有效就返回此编译标志
# 无效返回空
#
# 向/dev/null写入的内容都会永远丢失
# 标准输出和错误输出都重定向到/dev/null文件中
cc-option = $(shell if $(CC) $(CFLAGS) $(1) -S -o /dev/null -xc /dev/null \
		> /dev/null 2>&1; then echo "$(1)"; else echo "$(2)"; fi ;)

#
# 交叉编译环境的编译，链接等变量
#
# gnu汇编工具
AS	= $(CROSS_COMPILE)as

# gnu链接器
LD	= $(CROSS_COMPILE)ld

# C编译器，链接时使用C库
CC	= $(CROSS_COMPILE)gcc

# 编译预处理
# -E代表预处理后即停止，不进行编译，预处理后的代码送往标准输出
CPP	= $(CC) -E

# 创建，修改静态库
AR	= $(CROSS_COMPILE)ar

# 列出目标文件的符号表
NM	= $(CROSS_COMPILE)nm
LDR	= $(CROSS_COMPILE)ldr

# 去掉符号表,之后nm命令就看不见符号表
STRIP	= $(CROSS_COMPILE)strip

# 目标文件从二进制格式翻译或复制到另一种 
OBJCOPY = $(CROSS_COMPILE)objcopy

# 显示目标文件的各种信息
OBJDUMP = $(CROSS_COMPILE)objdump

# 更新库的符号表索引
RANLIB	= $(CROSS_COMPILE)RANLIB

#########################################################################

# 载入电路板配置文件
sinclude $(OBJTREE)/include/autoconf.mk


# 体系架构配置文件
ifdef	ARCH
sinclude $(TOPDIR)/arch/$(ARCH)/lib/config.mk	# include architecture dependend rules
endif

# 不同指令集CPU配置文件
ifdef	CPU
sinclude $(TOPDIR)/arch/$(ARCH)/cpu/$(CPU)/config.mk		# include  CPU	specific rules
endif

# 具体芯片型号配置文件
ifdef	SOC
sinclude $(TOPDIR)/arch/$(ARCH)/cpu/$(CPU)/$(SOC)/config.mk	# include  SoC	specific rules
endif

# 具体电路板配置文件
ifdef	VENDOR
BOARDDIR = $(VENDOR)/$(BOARD)
else
BOARDDIR = $(BOARD)
endif
ifdef	BOARD
sinclude $(TOPDIR)/board/$(BOARDDIR)/config.mk	# include board specific rules
endif

#########################################################################
# v选项屏蔽多余的冗长的处理进程显示
ifneq (,$(findstring s,$(MAKEFLAGS)))
ARFLAGS = cr
else
ARFLAGS = crv
endif


RELFLAGS= $(PLATFORM_RELFLAGS)
DBGFLAGS= -g # -DDEBUG
OPTFLAGS= -Os #-fomit-frame-pointer

# 链接描述文件
ifndef LDSCRIPT
LDSCRIPT := $(TOPDIR)/board/$(BOARDDIR)/u-boot.lds
endif
# objcopy命令参数，表示在拷贝时用0xff填充段与段之间的空隙
OBJCFLAGS += --gap-fill=0xff

gccincdir := $(shell $(CC) -print-file-name=include)

# 预处理参数
# #ifdef __KERNEL__
# //1
# #else
# //2
# #endif
# -D__KERNEL__就会执行1部分，和在代码中定义
# #define __KERNEL__效果一样
CPPFLAGS := $(DBGFLAGS) $(OPTFLAGS) $(RELFLAGS)		\
	-D__KERNEL__

# 定义TEXT_BASE的值
# #define TEXT_BASE $(TEXT_BASE)
ifneq ($(TEXT_BASE),)
CPPFLAGS += -DTEXT_BASE=$(TEXT_BASE)
endif

# 包含头文件位置
CPPFLAGS += -I$(OBJTREE)/include2 -I$(OBJTREE)/include
CPPFLAGS += -I$(TOPDIR)/include

# -fno-buildtin: 不接受两个下划线开头的内建函数
# -ffreestanding: 按独立环境编译
# -nostdinc: 不要在标准系统目录中寻找头文件，只搜索‘-I’
#  选项制定的目录（以及当前目录，如果合适）
#  -isystem
CPPFLAGS += -fno-builtin -ffreestanding -nostdinc	\
	-isystem $(gccincdir) -pipe $(PLATFORM_CPPFLAGS)

ifdef BUILD_TAG
CFLAGS := $(CPPFLAGS) -Wall -Wstrict-prototypes \
	-DBUILD_TAG='"$(BUILD_TAG)"'
else
CFLAGS := $(CPPFLAGS) -Wall -Wstrict-prototypes
endif

CFLAGS += $(call cc-option,-fno-stack-protector)


# $(CPPFLAGS) sets -g, which causes gcc to pass a suitable -g<format>
# option to the assembler.
AFLAGS_DEBUG :=

# 定义__ASSEMBLY__变量
AFLAGS := $(AFLAGS_DEBUG) -D__ASSEMBLY__ $(CPPFLAGS)

# 
LDFLAGS += -Bstatic -T $(obj)u-boot.lds $(PLATFORM_LDFLAGS)
ifneq ($(TEXT_BASE),)
LDFLAGS += -Ttext $(TEXT_BASE)
endif


#########################################################################

export	HOSTCC HOSTCFLAGS \
	AS LD CC CPP AR NM STRIP OBJCOPY OBJDUMP MAKE
export	TEXT_BASE PLATFORM_CPPFLAGS PLATFORM_RELFLAGS CPPFLAGS CFLAGS AFLAGS

#########################################################################

# Allow boards to use custom optimize flags on a per dir/file basis
BCURDIR := $(notdir $(CURDIR))
$(obj)%.s:	%.S
	$(CPP) $(AFLAGS) $(AFLAGS_$(@F)) $(AFLAGS_$(BCURDIR)) -o $@ $<
$(obj)%.o:	%.S
	$(CC)  $(AFLAGS) $(AFLAGS_$(@F)) $(AFLAGS_$(BCURDIR)) -o $@ $< -c
$(obj)%.o:	%.c
	$(CC)  $(CFLAGS) $(CFLAGS_$(@F)) $(CFLAGS_$(BCURDIR)) -o $@ $< -c
$(obj)%.i:	%.c
	$(CPP) $(CFLAGS) $(CFLAGS_$(@F)) $(CFLAGS_$(BCURDIR)) -o $@ $< -c
$(obj)%.s:	%.c
	$(CC)  $(CFLAGS) $(CFLAGS_$(@F)) $(CFLAGS_$(BCURDIR)) -o $@ $< -c -S

#########################################################################
