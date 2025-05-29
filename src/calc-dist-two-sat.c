/*
    Utility functions to calculate distance between two satellites
    Positions of each satellite are given by sat->pos
    sat->pos is vector of [x, y, z] obtained from SGP4 SDP4 algorithms
    Find distance between two points in a sphere where locations are known
*/


// have not been included in makefile, include when ready

#ifdef HAVE_CONFIG_H
#include <build-config.h>
#endif


#include <glib.h>
#include <glib/gi18n.h>

#include <math.h>

#include "gtk-sat-data.h"
#include "orbit-tools.h"
#include "calc-dist-two-sat.h"
#include "sat-cfg.h"
#include "sat-log.h"
#include "sgpsdp/sgp4sdp4.h"
#include "time-tools.h"


// Warpper function for dist_calc_driver()
gdouble dist_calc (sat_t *sat1, sat_t *sat2)
{
    gdouble sat1_posx, sat1_posy, sat1_posz;
    gdouble sat2_posx, sat2_posy, sat2_posz;

    sat1_posx = sat1->pos.x;
    sat1_posy = sat1->pos.y;
    sat1_posz = sat1->pos.z;

    sat2_posx = sat2->pos.x;
    sat2_posy = sat2->pos.y;
    sat2_posz = sat2->pos.z;

    return dist_calc_driver(sat1_posx, sat1_posy, sat1_posz,
                            sat2_posx, sat2_posy, sat2_posz);
}

// Driver function for calculating distance between two satellites
gdouble dist_calc_driver (gdouble sat1_posx, gdouble sat1_posy, gdouble sat1_posz,
                          gdouble sat2_posx, gdouble sat2_posy, gdouble sat2_posz)
{
    //placeholder
    return 0.0;
}

// Check if the straight line between two satellites collide with Earth
// We assume radius of Earth + 20km of atmosphere is the zone the laser
// cannot pass through.
// @param dist: distance between two satellites
gboolean check_collision (gdouble dist)
{
    // need implementation
    return False;
}


// Use heron's formula to get Area then divide by side C for height
// Return the height of a triangle given three sides
// side a is the distance from centre of Earth to sat A
// side b is the distance from centre of Earth to sat B
// side c is the straight line distance between sat A and B
gdouble get_height_of_triangle(gdouble side_a, gdouble side_b, gdouble side_c)
{
    gdouble height;

    // s: semiperimeter, a: area of triangle
    gdouble s, a;
    s = 0.5 * (side_a + side_b + side_c);
    a = sqrt(s * (s - side_a) * (s - side_b) * (s - side_c));

    height = (2 * a) / side_c;

    return height;
}