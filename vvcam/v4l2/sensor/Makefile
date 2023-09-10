all:
	@cd ov2775;  make || exit $$?;
	@cd os08a20; make || exit $$?;
	@cd ar1335;  make || exit $$?;
	@cd camera-proxy-driver;  make || exit $$?;

clean:
	@cd ov2775;   make clean;
	@cd os08a20;  make clean;
	@cd ar1335;   make clean;
	@cd camera-proxy-driver;   make clean;

modules_install:
	@cd ov2775;   make modules_install;
	@cd os08a20;  make modules_install;
	@cd ar1335;   make modules_install;
	@cd camera-proxy-driver;   make modules_install;

