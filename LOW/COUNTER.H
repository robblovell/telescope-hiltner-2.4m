class Counter {
  Counter () {
  }
  activate (int board) {
	 /* Inform Pacer that the base address is set to 230+board*20 hex */

	 result = PA_INITIALIZE(0x230+board*20);
	 if(result) printf("Counter::activate:Error on PA_INITIALIZE: %d\n",result);
  }
};