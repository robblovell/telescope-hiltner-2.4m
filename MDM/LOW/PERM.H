class Permanent_int {
  public:
	 int value;
	 int key;
	 Permanent_int () {
		// find a unique key
		// store the number in the database.
		value = 0;
	 }
	 Permanent_int (int key) {
		 // open the database file and look for the keyed value.
		 // if not found, create a new one, else restore the old
		 // value.
		 value = 0;
	 }
};