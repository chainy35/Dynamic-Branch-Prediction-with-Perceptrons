// my_predictor.h
// This file contains a sample my_predictor class.
// It is a simple 32,768-entry gshare with a history length of 15.
// Note that this predictor doesn't use the whole 8 kilobytes available
// for the CBP-2 contest; it is just an example.

#include <math.h>

class my_update : public branch_update {
public:
	unsigned int index;
	int percep_output;
};

class my_predictor : public branch_predictor {
public:
#define HISTORY_LENGTH	30
#define TABLE_BITS	15
#define INDEX_LENGTH 1<<TABLE_BITS
#define THETA floor(1.93 * HISTORY_LENGTH + 14)
#define MAX_WT		127  
#define MIN_WT		-128 
#define PERCEP_TABLE_SZ 268

	my_update u;
	branch_info bi;
	unsigned long long int history; // the branch history rigister (BHR)
    char tab_percep[PERCEP_TABLE_SZ][HISTORY_LENGTH+1]; // the perceptron table
	
	my_predictor (void) { 
	    // initialize the branch history register and perceptron table to 0
	    history = 0;
		for(int i=0 ; i< (PERCEP_TABLE_SZ); i++) {
			for(int j=0; j < (HISTORY_LENGTH+1); j++) {
				tab_percep[i][j] = 0;
			}
		}
		
	}

	branch_update *predict (branch_info & b) {
		bi = b;
		if (b.br_flags & BR_CONDITIONAL) {
			u.index = b.address % (PERCEP_TABLE_SZ);
			
			// calculate the perceptron output using bits of branch history register and 
			// the corresponding weights of the perceptron table
			u.percep_output = tab_percep[u.index][0];
			for (int i=1; i < (HISTORY_LENGTH +1); i++ ) {
				if ( (history >> i ) & 0x1) {
				    u.percep_output += tab_percep[u.index][i];				
				} else {
					u.percep_output -= tab_percep[u.index][i];		
				}
			}
			
			if ( u.percep_output >= 0) {
				u.direction_prediction (true);
				
			} else {
				u.direction_prediction (false);
				
			}

		} else {
			u.direction_prediction (true); // unconditional branch
		}
		u.target_prediction (0);
		return &u;
	}

	void update (branch_update *u, bool taken, unsigned int target) {
		if (bi.br_flags & BR_CONDITIONAL) {
			// if the prediction is wrong or the weight value is smaller than the threshold THETA, 
			// then update the weight  to train the perceptron
			if( ((my_update*)u)->direction_prediction() != taken || abs(((my_update*)u)->percep_output) < THETA) { 
			    unsigned int idx = ((my_update*)u)->index;    
				if (taken) {
			        tab_percep[idx][0] ++;
				} else {
				    tab_percep[idx][0] --;	
				}
				for (int i = 1; i < HISTORY_LENGTH + 1; i++) {
					if (taken == ((history >> i ) & 0x1)) {
				        if (tab_percep[idx][i]  < MAX_WT ) tab_percep[idx][i] ++;
			        } else {
				        if (tab_percep[idx][i] > MIN_WT) tab_percep[idx][i]  --;
			        }
					
				}
			}

			history <<= 1;
			history |= taken;
			history &= (1<<HISTORY_LENGTH)-1;
		}
	}
};
