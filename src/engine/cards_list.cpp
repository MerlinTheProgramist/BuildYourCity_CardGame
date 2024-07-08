#include "engine.h"


// Cost, Name, 
const CardSet GameEngine::masterSet{{
    // cards that will not generate in normal deck
    {0, "Architect",         0, 000_T, {1}, {0}, {}, true, 0},
    // {"Construction crew",  1, {0,0,0}, {0}, {0}, } 
    // normal cards, with no additional effects
    {6, "House",             1, 000_T, {1}, {0}},
    {4, "CarPark",           0, 111_T, {0}, {0}},
    {3, "Stadium",           6, 200_T, {2}, {3}},
    {3, "Train Station",     4, 111_T, {1}, {2}},
    {4, "Tower block",       3, 011_T, {1}, {1}},
    {3, "Convention Center", 7, 010_T, {1}, {5}},
    {2, "Hospital",          6, 001_T, {1}, {4}},
    {2, "Theater",           4, 001_T, {1}, {3}},
    {2, "Modern art",        4, 002_T, {0}, {3}},
    {6, "Buissness centre",  3, 002_T, {1}, {1}},
    {4, "Department store",  3, 011_T, {2}, {0}},
    {3, "Airport",           9, 110_T, {1}, {8}},
    {3, "Technology park",   5, 101_T, {2}, {2}},
    {4, "Cinema",            2, 111_T, {1}, {0}},
    {3, "Town hall",         6, 001_T, {0}, {5}},
    {2, "Monument",          9, 003_T, {1}, {8}},
    {2, "Discount retailer", 6, 120_T, {3}, {1}},
    {2, "Multiplex",         4, 120_T, {2}, {1}},
    {3, "Sport park",        4, 200_T, {0}, {3}},
    {5, "Auditorium",        7, 001_T, {0}, {7}},
    {2, "Skyscaper",         8, 001_T, {1}, {7}},
    {2, "Opera",             8, 001_T, {0}, {8}},
    {2, "Theme park",        8, 300_T, {2}, {5}},
    {4, "Market centre",     3, 100_T, {1}, {1}},
    {4, "Townhouse",         1, 001_T, {0}, {1}},
    {2, "Casino",            6, 010_T, {4}, {1}},
    {3, "Train station",     4, 111_T, {1}, {2}},

    // with special things
    {4, "City park",           1, 001_T, {0}, Gain{0, PerTag{001_T}}, {}, true},
    {4, "Shopping arcade",     1, 001_T, {0}, {0, PerTag{001_T}}, {}, true},
    {2, "Shopping centre",     7, 030_T, {2}, {0, PerTag{010_T}}},
    {2, "University",          5, 001_T, {0}, {4, PerBuild{1,"School"}}},
    {6, "Villa",               4, 000_T, {0}, {3, PerBuild{1,"Villa"}}},
    {4, "Bridge",              4, 100_T, {1, PerBuild{1,"Highway"}}, {2}},  
    {3, "Supermarket",         1, 110_T, {1}, {0}, {{"House"},{"Estate"}, {"Traffic interchange"}}},
    {2, "Park and eat",        11,200_T, {0}, {0, PerTag{100_T},PerEnemyTag{100_T}}},
    {4, "Office",              2, 000_T, {1, WithBuild{1, "Tower block"}}, {2}},
    {4, "Traffic intersection",2, 000_T, {0, PerTag{100_T}}, {0,PerTag{100_T}}, {}, true},
    {3, "Shopping mall",       8, 120_T, {3}, {0,PerTag{010_T}}},
    {4, "Municipal Office",    2, 000_T, {0, PerTag{001_T}}, {1}, {}, true}, // cost -1 for every building with Recreation 
    {6, "School",              1, 000_T, {0}, {2}, {{"House"}, {"Estate"}, {"Villa"}}}, // cost -2 for villa   
    {4, "Industrial estate",   1, 000_T, {1, WithBuild{1, "Research centre"}}, {0}}, // cost -1 for research centre and industrial site
    {3, "Boutique",            2, 011_T, {1, WithBuild{1, "Buissness centre"}}, {0}},
    {4, "Resteurant",          1, 011_T, {0, WithBuild{1, "Buissness centre"}}, {1}},  
    {2, "Industrial site",     2, 100_T, {1}, {0}}, // cost -1 for Highway and Road intersection
    {3, "Highway",             3, 100_T, {0}, {0, PerBuild{2, "Road intersection"}}},
    {3, "Museum",              5, 001_T, {0, PerBuild{1,"School"}}, {4}},
    {1, "Tube",                11,000_T, {0}, {0,PerTag{001_T},PerEnemyTag{001_T}}},
    {1, "Coach station",       1, 011_T, {0, WithBuild{1, "Supermarket"}}, {1}},
    {1, "Research centre",     4, 000_T, {1}, {2, PerGameBuild{2, "University"}}},
}};
