#ifndef LOCATION_H
#define LOCATION_H

#include <stdio.h>

struct Location {
	int first_line;
	int first_column;
	int last_line;
	int last_column;

	Location();
	Location(int fl, int fc, int ll, int lc) : first_line(fl), first_column(fc), last_line(ll), last_column(lc) {}
};

#define printerr(format, ...) do { fprintf(stderr, "%d.%d-%d.%d: " format, loc.first_line, loc.first_column, loc.last_line, loc.last_column, ##__VA_ARGS__); return; } while(0)

#define printsyntaxerr(loc, format, ...) do { fprintf(stderr, "%d.%d-%d.%d: " format, loc.first_line, loc.first_column, loc.last_line, loc.last_column, ##__VA_ARGS__); } while(0)

#define printheisenerr(format, ...) do { fprintf(stderr, format, ##__VA_ARGS__); return; } while(0)

#endif // LOCATION_H
