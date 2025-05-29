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
    
    // Use below if Haversine formula is needed instead of Euclidean distance
    /*
    gdouble sat1_lat, sat1_lon, sat2_lat, sat2_lon;

    sat1_lat = sat1->ssplat;
    sat1_lon = sat1->ssplon;
    sat2_lat = sat2->ssplat;
    sat2_lon = sat2->ssplon;

    return haversine_dist_calc_driver(sat1_lat, sat1_lon, sat2_lat, sat2_lon);
    */
}

// Driver function for calculating distance between two satellites
// Euclidean distance - need verification
// If not correct, will need to use Haversine formula
gdouble dist_calc_driver (gdouble sat1_posx, gdouble sat1_posy, gdouble sat1_posz,
                          gdouble sat2_posx, gdouble sat2_posy, gdouble sat2_posz)
{
    gdouble dist;

    dist = sqrt( pow((sat1_posx - sat2_posx), 2)
                + pow((sat1_posy - sat2_posy), 2)
                + pow((sat1_posz - sat2_posz), 2) );
    
    return dist;
}

// Haversine forumla
// Use if Euclidean distance is not correct
gdouble haversine_dist_calc_driver(gdouble sat1_lat, gdouble sat1_lon,
                                   gdouble sat2_lat, gdouble sat2_lon)
{
    gdouble dist;

    // distance betwen lat and lon
    gdouble dLat = (sat2_lat - sat1_lat) * PI / 180.0;
    gdouble dLon = (sat2_lon - sat1_lon) * PI / 180.0;

    // convert to rad
    sat1_lat = (sat1_lat) * PI / 180.0;
    sat2_lat = (sat2_lat) * PI / 180.0;

    // apply the formula
    gdouble a = pow (sin(dLat / 2), 2) +
                pow (sin(dLon / 2), 2) +
                cos(sat1_lat) * cos(sat2_lat);   
    gdouble c = 2 * asin(sqrt(a));
    dist = EARTH_RADIUS * c;

    return dist;
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


gdouble get_perpendicular_distance_from_centre_of_earth(gdouble side_a, gdouble side_b, gdouble side_c)
{

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