// -*-c++-*- Copyright (C) 2010 osgPango Development Team
// $Id$

#include <iostream>
#include <osgPango3/Context>

int main(int argc, char** argv) {
	osgPango3::Context::instance().init(132);

	osgPango3::FontList fl;

	unsigned int numFonts = osgPango3::Context::instance().getFontList(fl);

	std::cout << "Found " << numFonts << " font families." << std::endl;

	for(osgPango3::FontList::iterator i = fl.begin(); i != fl.end(); i++) std::cout
		<< *i << std::endl
	;

	return 0;
}
