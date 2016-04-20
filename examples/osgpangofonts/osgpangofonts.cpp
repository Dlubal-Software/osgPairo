// -*-c++-*- Copyright (C) 2010 osgPango Development Team
// $Id$

#include <iostream>
#include <osgPairo/Context>

int main(int argc, char** argv) {
	osgPairo::Context::instance().init(132);

	osgPairo::FontList fl;

	unsigned int numFonts = osgPairo::Context::instance().getFontList(fl);

	std::cout << "Found " << numFonts << " font families." << std::endl;

	for(osgPairo::FontList::iterator i = fl.begin(); i != fl.end(); i++) std::cout
		<< *i << std::endl
	;

	return 0;
}
