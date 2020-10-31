 // 3D Map/Maze explorer
// The following program converts a 2D player map into a 3D Rendering and allows the user to explore the maze in first person 

// To modify the maze ( inserting or removing walls), scroll down to the player map on the code editor and add X to add walls,
//or remove X to remove walls.

#include <iostream>
#include <utility>
#include <algorithm>
#include <chrono>
#include <vector>
#include <Windows.h>
#include <stdio.h>

int SHeight = 40;			// Number of rows on screen 
int SWidth = 120;			// Number of screen columns 

int NavigatorH = 16; int NavigatorW = 16;  // Dimensions of Map (2D array), Map coordinates are translated 
                                          // into physical rendering of 3D space displayed on the console screen

float Xplayer_1 = 6.75f;	// XPlayer and YPlayer represent the starting positions of Player 1  on the Map
float Yplayer_1 = 9.3f;     /* Note: Pls follow player numbering system specified, more players will be added in
                             updated version*/

float Max_Lim = 16.0f;			// Maximum rendering distance
float Player_speed = 5.0f;			// Walking Speed

float PView_1 = 3.1415f / 4.0f;	// Field of View divided into four segments 
float Player_Angle1 = 0.0f;	// Starting reference Field of View angle of player 1

int main()
{

	// Synthesize Console Screen Buffer with specified dimensions
	wchar_t* screen = new wchar_t[SWidth * SHeight];  
	HANDLE new_Console = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	DWORD bwpntr = 0;
	SetConsoleActiveScreenBuffer(new_Console);
	

	// Player Map of space and position ( Add X's in place of the dots to insert walls and vice versa 
	//Do not replace the Star character (*) with anything, Do not change the paramaters of the map (16*16) 
	std::wstring playermap;
	playermap += L"XXXXXXXXXXXXXXXX";
	playermap += L"X..............X";
	playermap += L"X..............X";
	playermap += L"...............X";
	playermap += L".......XX......X";
	playermap += L"XXXXXXXXX......X";
	playermap += L"X........*.....X";
	playermap += L"XXX............X";
	playermap += L"XX.............X";
	playermap += L"X......XXXX..XXX";
	playermap += L"X......X.......X";
	playermap += L"X......X.......X";
	playermap += L"X..............X";
	playermap += L"X......XXXXXXXXX";
	playermap += L"X...............";
	playermap += L"XXXXXXXXXXXXXXXX";

	// Variables created to find and use computer current FPS for a more sonistent and smoother player frame transition
	auto T1 = std::chrono::system_clock::now();
	auto T2 = std::chrono::system_clock::now();

	while (1)
	{
	    // Calculates elapsed time for FPS calculation, (required for every iteration in game loop as FPS varies) 
		T2 = std::chrono::system_clock::now();
		std::chrono::duration<float> time_elapsed = T2 - T1;
		T1 = T2;
		float eTime = time_elapsed.count();


		// A Key used for anti-clockwise rotations
		if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
			Player_Angle1-= (Player_speed * 0.8f) * eTime;  //Speed of player angle update can be modified by modifying 
		                                                    // float constant magnitude (currently set to 0.8 ) 

		//D Key used for Clockwise rotation
		if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
			Player_Angle1+= (Player_speed * 0.8f) * eTime;
		
		// For player moving forward (W KEY) 
		if (GetAsyncKeyState((unsigned short)'W') & 0x8000)
		{
			Xplayer_1+= sinf(Player_Angle1) * Player_speed * eTime;
			Yplayer_1 += cosf(Player_Angle1) * Player_speed * eTime;

			// In case player collides with a defined Wall (X) 
			if (playermap.c_str()[(int)Xplayer_1* NavigatorW + (int)Yplayer_1] == 'X')
			{
				Xplayer_1-= sinf(Player_Angle1) * Player_speed * eTime;
				Yplayer_1 -= cosf(Player_Angle1) * Player_speed * eTime;
			}
		}

		// For Movement backwards (S) 

		if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
		{
			Xplayer_1-= sinf(Player_Angle1) * Player_speed * eTime;
			Yplayer_1 -= cosf(Player_Angle1) * Player_speed * eTime;

			// To avoid reverse Wall collisions
			if (playermap.c_str()[(int)Xplayer_1* NavigatorW + (int)Yplayer_1] == 'X')
			{
				Xplayer_1+= sinf(Player_Angle1) * Player_speed * eTime;
				Yplayer_1 += cosf(Player_Angle1) * Player_speed * eTime;
			}
		}


		/* Loop below is designed to segment the players field of view into 120 rays (a preselected number) , for each of the rays projected, 
		 a distance calculation is made to quantify the distance between the player and the walls/obstacles in the 
		 players field of view. The distance calculation data from the rays projected are then collected and translated 
		 into diplay information onto the Console screen, each ray will be translated into pixel information in one screen column */

		for (int x = 0; x < SWidth; x++)
		{
			// No. of rays is dependent on Screen width desired by user ( 1 Ray= 1 Column of Screen information ) 
			float Rangle = (Player_Angle1- PView_1 / 2.0f) + ((float)x / (float)SWidth) * PView_1;
			
			/* Incrementally finds wall distances from player ( by step size specified )calculated distances can be made more accurate with 
			finer increments, this will affect game performace negatively*/

			float step_increments = 0.1f;   // Incremental distance calculation 										
			float total_Wdistance = 0.0f; // Cumulative distance                                    

			bool wall_found = false;		// Returns true when booundary or wall is encountered by projected ray 
 
			
			float unit_vectorY = cosf(Rangle);  // unit vectors ( in the  X and Y direction) for Players 
			float unit_vectorX = sinf(Rangle);  //  ray angle relative to starting angle of player field of view


			bool boundary_detected= false;		// Used for corner detection and boundary defintion (see next loop below)

			

			while (!wall_found && total_Wdistance < Max_Lim) // while ray does not hit a wall and the cumulative distance of each ray 
				                                           // does not exceed the total map dimensions (to avoid ray leaving bounds) 
			{
				total_Wdistance += step_increments; 
				int X_mapvalue = (int)(Xplayer_1 + unit_vectorX * total_Wdistance); // translates each player coordinate into 1D Char array position ( for map) 
				int Y_mapvalue = (int)(Yplayer_1 + unit_vectorY * total_Wdistance);

				
				// Asserts that ray remains in bounds ( stops if bounds are exceeded) 
				if (X_mapvalue < 0 || X_mapvalue >= NavigatorW || Y_mapvalue < 0 || Y_mapvalue >= NavigatorH) 
				{
					wall_found = true;		
					total_Wdistance = Max_Lim; // once the cumulative distance exceeds map dimensions, set that distance to the maximum distance limit
				}                             
				else  // Ray is within the bounds we must test with each increment ( determine whether Wall or normal space) 
				{

					// Ray from player encounters wall 
					if (playermap.c_str()[X_mapvalue * NavigatorW + Y_mapvalue] == 'X')
					{
					
						wall_found = true; 

						
						/* In order to define each wall tile, we design a basic corner detection 
						 algorithm to define the tile boundaries 
						 
						 Information projected from each of the four corners of each tile will create very acute angles  
						 with the incident ray projected from thte player, we create a vector of the distances and angles of
						 those vectors and sort it to find the rough locations of the tile corners
		
						 */



						std::vector<std::pair<float, float>> d_vec;  // This vector stores both the distance and angle information
						                                       
						/* The loop below calculates the dot_product product and magnitude of the vectors projecting 
						 from each corner of the tiles */

				
						for (int xcorner = 0; xcorner < 2; xcorner++)    
							for (int ycorner = 0; ycorner < 2; ycorner++)
							{
								
								float y_vector = (float)Y_mapvalue + ycorner - Yplayer_1;    // Y and X vectors found from each corner 
								float x_vector = (float)X_mapvalue + xcorner - Xplayer_1; 
								float d = sqrt(x_vector * x_vector + y_vector * y_vector);  // Calculates magnitude from each corner on map 
								float dot_product = (unit_vectorX * x_vector / d) + (unit_vectorY * y_vector / d);  // Dot product for angle
								d_vec.push_back(std::make_pair(d, dot_product));  // Adds to the vector a total of four corners
							}
			            // Sorts resultant vector from largest Distance and angle to smallest 
						sort(d_vec.begin(), d_vec.end(), [](const std::pair<float, float>& left, const std::pair<float, float>& right) {return left.first < right.first; });  // Magnitudes and dot_product products of rays will be sorted 

						/* Finds the first three closest corners which are tested with the specified threshold, if the fall below 
						this threshold they are classified as boundary cells and are shaded as such on the screen */

						float lim_ang = 0.01;   // Angle is set to 0.01 randians, any cell that creates an angle more acute than this one is classified as a corner 
						if (acos(d_vec.at(0).second) < lim_ang) boundary_detected= true;
						if (acos(d_vec.at(1).second) < lim_ang) boundary_detected= true;
						if (acos(d_vec.at(2).second) < lim_ang) boundary_detected= true;
					}
				}
			}

			/* Translation from Ray information into 2D space on Console Screen here  */

			// Three clasifications exist here, the Screen cell is either a Ceiling, wall or Floor tile. 
			// Each Cell is shaded according to classfication and distance information provided by the ray 

			int ceiling_limit = (float)(SHeight / 2.0) - SHeight / ((float)total_Wdistance);   // 
			int floortile = SHeight - ceiling_limit;

			// Shader walls based on distance, Brighter shades for closer distances 
			short tilechar = ' ';   // Char used to represent that CELL on the Screen 
			if (total_Wdistance <= Max_Lim / 4.0f)			tilechar = 0x2588;	 
			else if (total_Wdistance < Max_Lim / 3.0f)		tilechar = 0x2593;
			else if (total_Wdistance < Max_Lim / 2.0f)		tilechar = 0x2592;
			else if (total_Wdistance < Max_Lim)				tilechar = 0x2591;
			else											tilechar = ' ';		

			if (boundary_detected)		tilechar = ' ';  // Whenever boolean for boundary triggered, the cells in thre respective column are filled with this char (black space) 
			                                             
			for (int y = 0; y < SHeight; y++) // RAY information to Screen Row illustration 
			{
				  // If the distance isbelow the calculated ceiling threshold// reference (0,0) top left corner 
				if (y <= ceiling_limit)
					screen[y * SWidth + x] = ' ';  // fill ceiling tiles as blank spaces on screen 
				else if (y > ceiling_limit && y <= floortile)
					screen[y * SWidth + x] = tilechar;  // WALL cells on screen 
				else 
				{  // must be floor cells on screen
					
					// The floor cells are shaded differently according to player distance
					float ft = 1.0f - (((float)y - SHeight / 2.0f) / ((float)SHeight / 2.0f));
					if (ft < 0.25)		tilechar = 'X';
					else if (ft < 0.5)	tilechar = 'x';
					else if (ft < 0.75)	tilechar = '.';
					else if (ft < 0.9)	tilechar = '-';
					else				tilechar = ' ';
					screen[y * SWidth + x] = tilechar;
				}
			}
		}

		// Print player coordinates on map
		swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f ", Xplayer_1, Yplayer_1);

		/* Redraw player map on screen*/
		for (int dn = 0; dn < NavigatorW; dn++)
			for (int en = 0; en < NavigatorW; en++)
			{
				screen[(en + 1) * SWidth + dn] = playermap[en * NavigatorW + dn];
			}
		screen[((int)Xplayer_1+ 1) * SWidth + (int)Yplayer_1] = 'P';

		//Screen output 
		screen[SWidth * SHeight - 1] = '\0';
		WriteConsoleOutputCharacter(new_Console, screen, SWidth * SHeight, { 0,0 }, &bwpntr);
		
	}

	return 0;
}