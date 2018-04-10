// GameLibrary.cpp : Defines the exported functions for the DLL application.  
// Compile by using: cl /EHsc /DGAMELIBRARY_EXPORTS /LD GameLibrary.cpp  

#include "../stdafx.h"
#include "GameLibrary.h"  

namespace GameLibrary
{
	double Functions::Add(double a, double b)
	{
		return a + b;
	}

	double Functions::Multiply(double a, double b)
	{
		return a * b;
	}

	double Functions::AddMultiply(double a, double b)
	{
		return a + (a * b);
	}
}