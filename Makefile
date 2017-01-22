
DUT=nRF52832
TESTER=nRF51822

DUTID=682684386
TESTERID=480202020

REVERSE=0

ifeq ($(REVERSE),1)
	DUTTMP :=$(TESTER)
	TESTER :=$(DUT)
	DUT :=$(DUTTMP)

	DUTIDTMP :=$(TESTERID)
	TESTERID :=$(DUTID)
	DUTID :=$(DUTIDTMP)
endif

build:
	make -C device/$(DUT) DUT=1 build OBJECT_DIRECTORY=_build-dut
	make -C device/$(TESTER) build OBJECT_DIRECTORY=_build-tester

clean:
	make -C device/$(DUT) clean OBJECT_DIRECTORY=_build-dut
	make -C device/$(TESTER) clean OBJECT_DIRECTORY=_build-tester

flash:
	make -C device/$(DUT) flash OBJECT_DIRECTORY=_build-dut DEVKITID=$(DUTID)
	make -C device/$(TESTER) flash OBJECT_DIRECTORY=_build-tester DEVKITID=$(TESTERID)


build-dut:
	make -C device/$(DUT) DUT=1 build OBJECT_DIRECTORY=_build-dut

clean-dut:
	make -C device/$(DUT) clean OBJECT_DIRECTORY=_build-dut

flash-dut:
	make -C device/$(DUT) flash OBJECT_DIRECTORY=_build-dut DEVKITID=$(DUTID)


build-tester:
	make -C device/$(TESTER) build OBJECT_DIRECTORY=_build-tester

clean-tester:
	make -C device/$(TESTER) clean OBJECT_DIRECTORY=_build-tester

flash-tester:
	make -C device/$(TESTER) flash OBJECT_DIRECTORY=_build-tester DEVKITID=$(TESTERID)
