#  @author:   gaccob
#  @date:     2012-12-6
#  @function: lsi makefile

include lsi.make

.PHONY: xml src sample 

all: xml src sample 

xml:
	$(MAKE) -C $(LSI_XML)

src:
	$(MAKE) -C $(LSI_SRC)

sample:
	$(MAKE) -C $(LSI_SAMPLE)

clean:
	$(MAKE) -C $(LSI_XML) clean
	$(MAKE) -C $(LSI_SRC) clean
	$(MAKE) -C $(LSI_SAMPLE) clean
	@(echo "make lsi clean complete")

