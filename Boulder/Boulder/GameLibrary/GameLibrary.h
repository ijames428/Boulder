// GameLibrary.h - Contains declaration of Function class  
#pragma once  

namespace GameLibrary
{
	// This class is exported from the GameLibrary.dll  
	class Functions
	{
	public:
		// Returns a + b  
		static double Add(double a, double b);

		// Returns a * b  
		static double Multiply(double a, double b);

		// Returns a + (a * b)  
		static double AddMultiply(double a, double b);
	};
}