all:
	cd haarp && $(MAKE)
	cd haarp/plugins && $(MAKE)

clean:
	cd haarp && $(MAKE) clean
	cd haarp/plugins && $(MAKE) clean

install:
	cd haarp && $(MAKE) install
