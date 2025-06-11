.PHONY: all step1 step2 step3 step4 step5 step6 step7 step8 step9 clean


step1:
	$(MAKE) -C step1_CH

step2:
	$(MAKE) -C step2_profiling

step3:
	$(MAKE) -C step3_interactive

step4:
	$(MAKE) -C step4_multiuser_server

step6:
	$(MAKE) -C step6_reactor_server

step7:
	$(MAKE) -C step7_threaded_server

step9:
	$(MAKE) -C step9_proactor_server

step10:
	$(MAKE) -C step10_producer_consumer

clean:
	$(MAKE) -C step1_CH clean
	$(MAKE) -C step2_profiling clean
	$(MAKE) -C step3_interactive clean
	$(MAKE) -C step4_multiuser_server clean
	$(MAKE) -C step6_reactor_server clean
	$(MAKE) -C step7_threaded_server clean
	$(MAKE) -C step9_proactor_server clean
	$(MAKE) -C step10_producer_consumer clean
