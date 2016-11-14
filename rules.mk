#########################################################################
# 自动生成目录下需要编译文件的依赖项
#########################################################################

_depend:	$(obj).depend

# $(SRCS)代表当前目录下面的源文件
# obj参数使用config.mk修改为当前编译的目录
$(obj).depend:	$(SRCS)
		@rm -f $@
		@for f in $(SRCS); do \
			g=`basename $$f | sed -e 's/\(.*\)\.\w/\1.o/'`; \
			$(CC) -M $(CPPFLAGS) -MQ $(obj)$$g $$f >> $@ ; \
		done

#########################################################################
