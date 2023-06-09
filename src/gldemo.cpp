//
// Array Visualisation for CIS6007 WRIT1
//

#include <iostream>
#include <string>
#include <random>
#include "gl_stuff.h"
#include "Array2D.h"
#include <tbb/tbb.h>


using namespace std;
using namespace tbb;


#pragma region Function Prototypes

void initPattern(Array2D<int>& array); // Initialise grid with "alive" and "dead" cell patterns
void update(void); // Main update function - apply Game of Life rules here

#pragma endregion


//
// Example array
//
Array2D<int>						myArray = Array2D<int>(32, 32);


// Starting point for our program...
int main(int argc, char* argv[])
{
	// Initialise OpenGL window...
	gl_init(argc, argv, update, "CIS6007 WRIT1 - Game of Life Demo", true);

	// Initialise our Game of Life starting pattern...
	initPattern(myArray);

	// Start the main loop - this calls our update function (below) to update our game of life generations.
	// Note: We never return from glutMainLoop until we quit the application
	glutMainLoop();

    return 0;
}


// Initialise grid with "alive" and "dead" cell patterns
void initPattern(Array2D<int>& array) {

	// 1. First clear the array (set all cells to "dead" ie. 0)
	parallel_for(
		0,
		array.w * array.h,
		[&](int i) {

			int x = i % array.w;
			int y = i / array.h;
			array(x, y) = 0;
		}
	);

	// Setup some cells to be "alive" - this is the initial state
	// NOTE: *** REPLACE THIS WITH YOUR OWN CODE TO SETUP THE LIVE CELL PATTERNS
	myArray(2, 2) = 1;
	myArray(2, 3) = 1;
	myArray(2, 4) = 1;
    myArray(3, 3) = 1;

}


// Main update function - apply Game of Life rules here
// Note: This is called by FreeGLUT's main event loop - YOU DON'T NEED TO CALL THIS - IT'S CALLED FOR YOU!!!
void update(void) {

    // Create temporary array to handle cells
    Array2D<int> tempArray(myArray.w, myArray.h);

   
    // Split grid into subsections that can update in parallel
    parallel_for(
        blocked_range2d<int>(0, myArray.h, 0, myArray.w),
        [&](const blocked_range2d<int>& r) {
            // Iterate over current rows
            for (int y = r.rows().begin(); y < r.rows().end(); y++) {
                // Iterate over current columns
                for (int x = r.cols().begin(); x < r.cols().end(); x++) {

                    int neighbors = 0;

                    // Iterate over the grid in parallel
                    // Find alive neighbour cells
                    for (int i = -1; i <= 1; i++) {
                        for (int j = -1; j <= 1; j++) {

                            // Find coords of neighbour
                            int xn = x + i;
                            int yn = y + j;

                            // Handle grid boundary (wrap cells around the edges of the grid)
                            if (xn < 0) xn = myArray.w - 1;
                            if (yn < 0) yn = myArray.h - 1;

                            if (xn >= myArray.w) xn = 0;
                            if (yn >= myArray.h) yn = 0;

                            // Only count alive neighbours
                            if (i != 0 || j != 0) {
                                neighbors += myArray(xn, yn);
                            }
                        }
                    }

                    // Applying game of life rules //

                    // Cell is alive but has less than two/more than three neighbours, so it dies.
                    if (myArray(x, y) == 1 && (neighbors < 2 || neighbors > 3)) {
                        tempArray(x, y) = 0;
                    }

                    // Dead cell has 3 neighbours, so it revives
                    else if (myArray(x, y) == 0 && neighbors == 3) {
                        tempArray(x, y) = 1;
                    }

                    // Otherwise, cell stays the same
                    else {
                        tempArray(x, y) = myArray(x, y);
                    }
                }
            }
        }
    );

    // Replace the main array with the updated array
    myArray = tempArray;

    // Rerender screen
    renderLatestGeneration(myArray);

    // Sleep for 100 ms (DEBUG: for artificially slowing the program in cases of high execution speeds)
    // std::this_thread::sleep_for(std::chrono::milliseconds(100));

}