#include "evolve.h"

extern double eTable[26][26];		// English contact table
extern double cTable[26][26];		// Cipher contact table
extern vector < keyFitType > population; // The population of possible keys

// Returns the fitness found using the currently selected fitness function
double fitness( string key ) {
    if(FITNESS_FUNC == 0)
        return eFitness(key);
    else
        return bFitness(key);
} // end fitness

// Fitness function using Euclidean distance
// SMALLER IS BETTER FOR THIS FITNESS
double eFitness( string key ) {
	double fit = 0.0; // Fitness
	for( int i = 0; i < 26; i++ ) {
		for( int j = 0; j < 26; j++ ) {
            if(eTable[i][j] != 0) // Only calculate fitness for english frequencies that exist
    			fit += pow(( eTable[i][j] - cTable[(int)(key[i] - 97)][(int)(key[j] - 97)] ), 2);
            // Punish if the key has a frequency that english does not
            //else if(cTable[(int)(key[i] - 97)][(int)(key[j] - 97)] != 0)
            //    fit *= ePunishment;
		}
	}
	return fit;
} // end eFitness


// Fitness function using Bhattacharyya distance
// BIGGER IS BETTER FOR THIS FITNESS (oh yeah)
double bFitness( string key ) {
	double fit = 0.0; // Fitness
	for( int i = 0; i < 26; i++ ) {
		for( int j = 0; j < 26; j++ ) {
            if(eTable[i][j] != 0)
    			fit += sqrt(eTable[i][j] * cTable[(int)(key[i] - 97)][(int)(key[j] - 97)]);
            // Punish if the key has a frequency that english does not
            //else if(cTable[(int)(key[i] - 97)][(int)(key[j] - 97)] != 0)
            //    fit *= bPunishment;
		}
	}
	return fit;
} // end bFitness


// Mutate chromosome in-place using permutation-based mutation operator
void mutate( int chromosome ) {
	if(MUTATION == 0) { // Single Swap mutation
		swap(population[chromosome].key[randMod(26)], population[chromosome].key[randMod(26)]);
	} else if(MUTATION == 1) { // Swap twice
		swap(population[chromosome].key[randMod(26)], population[chromosome].key[randMod(26)]);
		swap(population[chromosome].key[randMod(26)], population[chromosome].key[randMod(26)]);
    } else if(MUTATION == 2) { // Three-point shift 
        int a, b, c;
        a = randMod(26 - 2);
        do { b = randMod(26 - 1); } while(b <= a);
        do { c = randMod(26); } while(c <= b);
        char tempB = population[chromosome].key[b];
		swap(population[chromosome].key[a], population[chromosome].key[b]);
		swap(population[chromosome].key[c], tempB );
    }
} // end mutate


// Using a tournament selection to select parents out of the population, culls the weak (bwuahahha)
// This is where we calculate the fitnesses
// NOTE: Assumes a tourney size of three. This may (and probably should) change
int select( int &parentA, int &parentB ) {
    int indicies[TSIZE];
    int range[POPSIZE];
    vector <indFitType> members;
    indFitType m;
    int temp = 0;

    // Randomly select a unique individual (spent more time on this than i should've...)
    for( int i = 0; i < POPSIZE; i++ ) range[i] = i;
    for( int i = 0; i < TSIZE; i++ ) {
        do { temp = randMod(POPSIZE); } while(range[temp] < 0);
        indicies[i] = range[temp];
        range[temp] = -1;
    }

    // Setup the tournament, determining fitness of each contestant
    for( int i = 0; i < TSIZE; i++ ) {
        m.index = indicies[i];
        m.fit = population[indicies[i]].fit;
        members.push_back(m);
    }

    // FIGHT! FIGHT FOR YOUR SURVIVAL!
    sort(members.begin(), members.end(), compSelect);

    // Winners circle
    parentA = members[0].index;
    parentB = members[1].index;

    // Losing individual will be overwritten by child...what a horrible fate
    return members[TSIZE - 1].index;
} // end select

// Comparison function for sorting that changes based on fitness function used
bool compSelect( indFitType a, indFitType b) {
    if(FITNESS_FUNC == 0)
        return (a.fit < b.fit);
    else
        return (a.fit > b.fit);
} // end compSelect


// PMX Crossover. Code derived from Robert Heckendorn's implementation found in pmx.cpp
void pmx( int parentA, int parentB, int child ) {
    const double prob = 3.0 / 26.0; // Tweak this tweak the world
    bool pick[26];
    int locA[26];
    int locB[26];
    string a = population[parentA].key;
    string b = population[parentB].key;
    string c(26, 'a'); // Temporary child string

    // Copy a random subnet (a "swath") of parent B
    for( int i = 0; i < 26; i++ ) {
        if(choose(prob)) { // Select what to copy from parent B
            pick[i] = true;
            c[i] = b[i];
        } else { // Mark everything else as bad (-1)
            pick[i] = false;
            c[i] = -1;
        }

        // Location lookup table for A and B
        locA[(int)(a[i] - 97)] = locB[(int)(b[i] - 97)] = i; 
    }

    // Remove duplicates
    for( int i = 0; i < 26; i++ ) {
        if( pick[i] && !pick[locB[(int)(a[i] - 97)]] ) {
            int loc = i;
            do { loc = locA[(int)(b[loc] - 97)]; } while( c[loc] != -1 );
            c[loc] = a[i]; 
        }
    }

    // Copy remaining genes from parent A
    for( int i = 0; i < 26; i++ ) {
        if( c[i] == -1 ) c[i] = a[i];
    }

    // Put child into population
    population[child].key = c;
} // end pmx
