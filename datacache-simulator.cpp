/******************************************************************************
*                                                                             *
*		Name: Scott Kamin                                                     *
*		Class: CDA3101                                                        *
*		Assignment: Implementing a data cache simulator                       *
*		Complie: "g++47 -std=c++11 datacache.cpp"                             *
*                                                                             *
******************************************************************************/

#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <math.h>
#include <bitset>
#include <vector>

using namespace std;

// struct representing a line
struct line{
    bool dirtyBit;
    bool validBit;
    int tag;
    int LRUcount;
};

// 2 functions called to print the header
void printCacheConfigHead(const int ns, const int ss, const int ls);
void printMainHead();
// function to find the spot where the least recently used is
int findLRU(const vector<vector<line>> cache, const int index, const int ns);

int main()
{

	/* Get # of sets, set size, and line size from the config file           */
   ifstream inConfig;
   inConfig.open("trace.config");
   string inWords;

   unsigned short numSets = 0;
   unsigned short setSize = 0;
   unsigned short lineSize = 0;

   getline(inConfig,inWords,':');
   inConfig >> numSets;
   inConfig >> ws;
   getline(inConfig,inWords,':');
   inConfig >> setSize;
   inConfig >> ws;
   getline(inConfig,inWords,':');
   inConfig >> lineSize;

   printCacheConfigHead(numSets, setSize, lineSize);

   unsigned short SoffSet = log2(lineSize);
   unsigned short Sindex = log2(numSets);
   unsigned short Stag = 32 - (SoffSet + Sindex);
   /*------------------------------------------------------------------------*/

   /*------------------------------------------------------------------------*/
   /* Initialize the vector of vectors representing the cache                */
   vector<vector<line> > cache;
   cache.resize(numSets);
   for(int w = 0; w < cache.size(); w++){
        cache.at(w).resize(setSize);
        for(int z = 0; z < setSize; z++){
            cache.at(w).at(z).validBit = false;
            cache.at(w).at(z).tag = 0;
            cache.at(w).at(z).dirtyBit = false;
            cache.at(w).at(z).LRUcount = 0;
        }
   }
   /*------------------------------------------------------------------------*/

   printMainHead();

   /*------------------------------------------------------------------------*/
   /* While loop that takes each line and calculates parts and outputs it    */

    // variables for summary printout
    int Hits = 0;
    int Misses = 0;

    // variables used in while loop
    char endCheck = 'a';
    int refNum = 1;
    int lineNum = 0;

    while(endCheck != EOF){

    // variables that extract from standard input (or .dat file)
	char accType;       // R or W for read or write
	int refSize = 0;    // bytes trying to be accessed
	int hexAdd;         // hexadecimal address

	scanf(" %c:%d:%x", &accType, &refSize, &hexAdd);
	// keep track of line of input
    lineNum++;

	/* Calculate offset, index, and tag numbers                              */
	bitset<32> biAdd(hexAdd);
    bitset<32> offset;
    int tempMarker = 0;
	// get bit string for offset
    for(int q=0; q < SoffSet; ++q){
        tempMarker++;
        if(biAdd[q])
           offset.set(q);
    }
	// get bit string for index
    bitset<32> index;
    for(int y = 0; y < Sindex; y++){
        if(biAdd[tempMarker])
            index.set(y);
            tempMarker++;
    }
	// convert bit string for tag
	bitset<32> tag;
    for(int e = 0; e < Stag; e++){
        if(biAdd[tempMarker])
            tag.set(e);
            tempMarker++;
    }
	// convert bits to ints
    int tagDec = tag.to_ulong();
	int offsetDec = offset.to_ulong();
	int indexDec = index.to_ulong();
	/*-----------------------------------------------------------------------*/

    /*-----------------------------------------------------------------------*/
    /* Check alignment and check read or write size is 1,2,4 or 8            */
    bool validLine = false;
    bool missAline = false;
    if(refSize == 1 || refSize == 2 || refSize == 4 || refSize == 8){
        validLine = true;
        if(refSize > lineSize)
            validLine = false;
    }
	// now checks if linesize is a multiple of reference size
    if(validLine == true){
        if(offsetDec != 0){
            bool alineCheck = false;
            int temp = lineSize;
            while(temp >= 0 && alineCheck == false){
                temp = temp - refSize;
                if(temp == offsetDec)
                    alineCheck = true;
            }
            if(alineCheck == true)
                missAline = true;
        }
    else
        missAline = true;
    }
    /*-----------------------------------------------------------------------*/

	/*-----------------------------------------------------------------------*/
	/* checks whether it is a read or write and checks cache for hit or miss
	and updates cache accordingly if the alignment is valid                  */
	if(validLine == true && missAline == true){
        string RorW;	        // string to print for read or write
        string HorM = "abc";	// string to print for hit or miss
        int memRefMore = 0;

        /* If block for if the access is a read                              */
        if(accType == 'R'){
            RorW = "  read";
            bool tracker = false;
			// for loop checks if data is already in the cache
            for(int i=0; i<setSize && tracker == false; i++){
                if(cache.at(indexDec).at(i).tag == tagDec &&
                   cache.at(indexDec).at(i).validBit == true){
                        HorM = "   hit ";
                        cache.at(indexDec).at(i).LRUcount += 1;
                        tracker = true;
                        Hits++;
                }
            }
			// if not in cache it is a miss
            if(tracker == false){
                HorM = "  miss ";
                Misses++;
                bool emptySpot = false;
				// for loop to see if there is an empty block to insert data
                for(int v=0; v<setSize && emptySpot == false; v++){
                    if(cache.at(indexDec).at(v).validBit == false){
                        memRefMore = 1;	// takes 1 memory reference
                        cache.at(indexDec).at(v).validBit = true;
                        cache.at(indexDec).at(v).tag = tagDec;
                        cache.at(indexDec).at(v).LRUcount = 0;
                        emptySpot = true;
                    }
                }
				// if there is no empty spot the LRU slot is replaced
				// calls a helper function
                if(emptySpot == false){
					// calls function to find place LRU
                    int replaceSpot = findLRU(cache,indexDec,setSize);
                    if(cache.at(indexDec).at(replaceSpot).dirtyBit == true)
                        memRefMore = 2;	// takes 2 memory references
                    else
                        memRefMore = 1;	// takes 1 memory reference
                    cache.at(indexDec).at(replaceSpot).validBit = true;
                    cache.at(indexDec).at(replaceSpot).tag = tagDec;
					// increase LRU counter to keep track of LRU
                    cache.at(indexDec).at(replaceSpot).LRUcount += 1;
                    cache.at(indexDec).at(replaceSpot).dirtyBit = false;
                }
            }
        }
		// else block for a write access
        else{
            RorW = " write";
            bool track = false;
			// for loop checks if data is already in cache
            for(int i=0; i<setSize; i++){
                if(cache.at(indexDec).at(i).tag == tagDec &&
                    cache.at(indexDec).at(i).validBit == true){
                        HorM = "   hit ";
                        Hits++;
                        cache.at(indexDec).at(i).LRUcount += 1;
                        cache.at(indexDec).at(i).dirtyBit = true;
                        track = true;
                        memRefMore = 0;
                }
            }
			// if data is not already there then it is a miss
            if(track == false){
                HorM = "  miss ";
                Misses++;
                bool spotEmpty = false;
				// for loop to check if there is empty block in index
                for(int v=0; v<setSize && spotEmpty == false; v++){
                    if(cache.at(indexDec).at(v).validBit == false){
                        memRefMore = 1;
                        cache.at(indexDec).at(v).validBit = true;
                        cache.at(indexDec).at(v).tag = tagDec;
                        cache.at(indexDec).at(v).dirtyBit = true;
                        cache.at(indexDec).at(v).LRUcount = 0;
                        spotEmpty = true;
                    }
                }
				// if block for if slot needs to be replaced
                if(spotEmpty == false){
                    int replaceSpot = findLRU(cache,indexDec,setSize);
                    if(cache.at(indexDec).at(replaceSpot).dirtyBit == true)
                        memRefMore = 2;
                    else
                        memRefMore = 1;
                    cache.at(indexDec).at(replaceSpot).validBit = true;
                    cache.at(indexDec).at(replaceSpot).tag = tagDec;
                    cache.at(indexDec).at(replaceSpot).LRUcount += 1;
                }
            }
        }
        // end of else block (write access)
        /*********************************************************************/

        /* Print out line with ref number, read or write, address, tag, index,
           offset, hit or miss, and memory references                        */
        cout << setw(4) << right << refNum << ' ' <<  RorW << ' ';
        cout << setw(8) << right << std::hex << hexAdd << ' ';
        cout << setw(7) << right << std::hex << tagDec;
        cout << ' ' << setw(5) << right << std::dec <<  indexDec << ' ';
        cout << setw(6) << right << offsetDec << ' ' << HorM;
        cout << setw(7) << right << memRefMore << endl;
        // increase valid reference number
        refNum++;
        /*********************************************************************/
	}// end of if the reference is valid
	else if(validLine == false){
        // print to stderr if the size is illegal
        cerr << "line "<<lineNum<< " has illegal size " <<refSize<< endl;
	}
	else{
	    // print to stderr if the reference is misaligned
        cerr << "line " << lineNum << " has misaligned reference at address ";
        cerr <<std::hex<< hexAdd << " for size " <<std::dec<< refSize << endl;
	}
	// extract any white space and check for EOF
	cin >> ws;
	endCheck = cin.peek();
   }// end of while loop
   /*------------------------------------------------------------------------*/

    /* Calculate the total accesses, hit ratio, and miss ratio               */
    int Accesses = Hits + Misses;
    double HitsD = static_cast<double>(Hits);
    double MissesD = static_cast<double>(Misses);
    double hitRatio = HitsD / Accesses;
    double missRatio = MissesD / Accesses;
    /*************************************************************************/


    /* Print out summary statistics with the # hits, # misses, # of total
    accesses, hit ratio (to 6 decimal places), miss ratio (6 decimal places) */
    cout << endl;
    cout << endl;
    cout << "Simulation Summary Statistics" << endl;
    cout << "-----------------------------" << endl;
    cout << "Total hits       : " << Hits << endl;
    cout << "Total misses     : " << Misses << endl;
    cout << "Total accesses   : " << Accesses << endl;
    cout << "Hit ratio        : " << setprecision(6) << hitRatio << endl;
    cout << "Miss ratio       : " << setprecision(6) << missRatio << endl;
    cout << endl;

return 0;
}

void printMainHead(){
   // print the main header
   cout << "Results for Each Reference" << endl;
   cout << endl;
   cout << setw(4) << left << "Ref" << ' ' << setw(6) << left << "Access";
   cout << ' ' << setw(8) << left << "Address" << ' ' << "  Tag   ";
   cout << setw(5) << "Index" << ' ' << "Offset Result Memrefs" << endl;
   cout << "---- ------ -------- ------- ----- ------ ------ -------" << endl;
};

void printCacheConfigHead(const int numSets, const int setSize,
                           const int lineSize){
	// print cache config from .config file
   cout << "Cache Configuration" << endl;
   cout << endl;
   cout << "   " << numSets << ' ' << setSize;
   cout << "-way set associative entries" << endl;
   cout << "   of line size " << lineSize << " bytes" << endl;
   cout << endl;
   cout << endl;
};

int findLRU(const vector<vector<line>> cache, const int index, const int ss){
	// finds place where the LRU block is using a counter
    int lowNum = cache.at(index).at(0).LRUcount;
    int returnSpot = 0;
    for(int w = 0; w < ss; w++){
        if(cache.at(index).at(w).LRUcount < lowNum){
            lowNum = cache.at(index).at(w).LRUcount;
            returnSpot = w;
        }
    }
    return returnSpot;
};
