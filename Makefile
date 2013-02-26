#  @author:   gaccob
#  @date:     2012-12-6
#  @function: lsi makefile

include lsi.make

.PHONY: src sample 

all: src sample 

src:
	$(MAKE) -C $(LSI_SRC)

sample:
	$(MAKE) -C $(LSI_SAMPLE)

clean:
	$(MAKE) -C $(LSI_SRC) clean
	$(MAKE) -C $(LSI_SAMPLE) clean
	@(echo "make lsi clean complete")

