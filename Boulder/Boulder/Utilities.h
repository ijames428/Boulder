#ifndef UTILITIES_H
#define UTILITIES_H

using namespace std;
#include <SFML/Graphics.hpp>
#include <sstream>

class Utilities {
public:
	Utilities() { }

	static std::vector<float> StringBoxDataToUsableVectorBoxData(string str_box_data) {
		float boxScale = 40.0f;

		std::vector<float> vect_box;

		if (str_box_data != "") {
			std::stringstream ss_box(str_box_data);
			float hurt_box_character;

			while (ss_box >> hurt_box_character)
			{
				vect_box.push_back(hurt_box_character);

				if (ss_box.peek() == ',')
					ss_box.ignore();
			}
		}
		else {
			vect_box.push_back(1.0f);
			vect_box.push_back(1.0f);
			vect_box.push_back(1.0f);
			vect_box.push_back(1.0f);
		}

		float box_relative_x = ((vect_box[0] + (vect_box[2] / 2.0f)) / boxScale);
		float box_relative_y = ((vect_box[1] + (vect_box[3] / 2.0f) - (vect_box[1] + (vect_box[3] / 2.0f))) / boxScale);// -(radius * 30.0f);
		float box_width = (vect_box[2]) / boxScale;
		float box_height = (vect_box[3]) / boxScale;

		return std::vector<float> { box_relative_x, box_relative_y, box_width, box_height };
	}
};

#endif